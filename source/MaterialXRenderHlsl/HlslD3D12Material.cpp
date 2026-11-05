//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslD3D12Material.h>

#include <MaterialXRender/ShaderRenderer.h>

#define NOMINMAX 1
#include <Windows.h>
#include <d3d12.h>
#include <wrl/client.h>

#include <cstdio>
#include <cstring>
#include <unordered_map>

MATERIALX_NAMESPACE_BEGIN

namespace
{

using Microsoft::WRL::ComPtr;

const std::size_t NOT_FOUND = std::size_t(-1);

D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle(uint64_t ptr)
{
    D3D12_CPU_DESCRIPTOR_HANDLE h;
    h.ptr = static_cast<SIZE_T>(ptr);
    return h;
}

D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle(uint64_t ptr)
{
    D3D12_GPU_DESCRIPTOR_HANDLE h;
    h.ptr = ptr;
    return h;
}

D3D12_SHADER_VISIBILITY toVisibility(HlslShaderVisibility visibility)
{
    switch (visibility)
    {
        case HlslShaderVisibility::Vertex: return D3D12_SHADER_VISIBILITY_VERTEX;
        case HlslShaderVisibility::Pixel:  return D3D12_SHADER_VISIBILITY_PIXEL;
        default:                           return D3D12_SHADER_VISIBILITY_ALL;
    }
}

DXGI_FORMAT toVertexFormat(const HlslVertexElement& e)
{
    static const DXGI_FORMAT floats[] = { DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT,
                                          DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT };
    static const DXGI_FORMAT sints[] = { DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32G32_SINT,
                                         DXGI_FORMAT_R32G32B32_SINT, DXGI_FORMAT_R32G32B32A32_SINT };
    static const DXGI_FORMAT uints[] = { DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32G32_UINT,
                                         DXGI_FORMAT_R32G32B32_UINT, DXGI_FORMAT_R32G32B32A32_UINT };
    if (e.componentCount < 1 || e.componentCount > 4)
        return DXGI_FORMAT_UNKNOWN;
    switch (e.componentType)
    {
        case HlslComponentType::SInt: return sints[e.componentCount - 1];
        case HlslComponentType::UInt: return uints[e.componentCount - 1];
        default:                      return floats[e.componentCount - 1];
    }
}

std::string hresultString(HRESULT hr)
{
    char code[16];
    std::snprintf(code, sizeof(code), "0x%08X", static_cast<unsigned int>(hr));
    return code;
}

// Where the contents of a root constant buffer view come from.
struct CbufferSource
{
    HlslD3D12Material::Stage stage = HlslD3D12Material::Stage::Vertex;
    std::size_t cbufferIndex = 0;
};

} // namespace

struct HlslD3D12Material::Impl
{
    HlslD3D12ContextPtr context;
    HlslProgramPtr program;

    HlslStageReflection vsReflection;
    HlslStageReflection psReflection;
    HlslRootSignatureDesc rootDesc;
    ComPtr<ID3D12RootSignature> rootSignature;

    // CPU copies of each constant buffer, indexed like the stage
    // reflection's getCbuffers().
    std::vector<std::vector<uint8_t>> vsCbuffers;
    std::vector<std::vector<uint8_t>> psCbuffers;

    // Per root parameter: the constant buffer it binds, when it is a
    // constant buffer view.
    std::vector<CbufferSource> cbufferSources;

    std::vector<HlslVertexElement> vertexLayout;
    unsigned int vertexStride = 0;
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;

    std::vector<uint64_t> textures;
    std::vector<uint64_t> samplers;

    std::unordered_map<uint64_t, ComPtr<ID3D12PipelineState>> pipelines;

    const HlslStageReflection& reflection(Stage stage) const
    {
        return stage == Stage::Vertex ? vsReflection : psReflection;
    }

    std::vector<std::vector<uint8_t>>& cbuffers(Stage stage)
    {
        return stage == Stage::Vertex ? vsCbuffers : psCbuffers;
    }

    int findCbufferBySlot(Stage stage, unsigned int slot) const
    {
        const auto& list = reflection(stage).getCbuffers();
        for (std::size_t i = 0; i < list.size(); ++i)
        {
            if (list[i].slot == slot && list[i].space == 0)
                return static_cast<int>(i);
        }
        return -1;
    }

    bool writeRange(Stage stage, std::size_t cbufferIndex, std::size_t offset, const void* data, std::size_t count)
    {
        auto& list = cbuffers(stage);
        if (cbufferIndex >= list.size() || !data || count == 0)
            return false;
        std::vector<uint8_t>& bytes = list[cbufferIndex];
        if (offset + count > bytes.size())
            return false;
        std::memcpy(bytes.data() + offset, data, count);
        return true;
    }

    void createRootSignature();
    ID3D12PipelineState* getPipeline(uint32_t renderTargetFormat, uint32_t depthFormat,
                                     const HlslD3D12RenderState& state);
};

void HlslD3D12Material::Impl::createRootSignature()
{
    const std::vector<HlslRootParameter>& params = rootDesc.getParameters();
    std::vector<D3D12_ROOT_PARAMETER> rootParams(params.size());
    std::vector<D3D12_DESCRIPTOR_RANGE> ranges(params.size());
    for (std::size_t i = 0; i < params.size(); ++i)
    {
        const HlslRootParameter& p = params[i];
        D3D12_ROOT_PARAMETER& rp = rootParams[i];
        rp.ShaderVisibility = toVisibility(p.visibility);
        if (p.type == HlslRootParameterType::ConstantBuffer)
        {
            rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            rp.Descriptor.ShaderRegister = p.registerBase;
            rp.Descriptor.RegisterSpace = p.space;
        }
        else
        {
            D3D12_DESCRIPTOR_RANGE& range = ranges[i];
            range.RangeType = (p.type == HlslRootParameterType::TextureTable)
                            ? D3D12_DESCRIPTOR_RANGE_TYPE_SRV
                            : D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            range.NumDescriptors = p.registerCount;
            range.BaseShaderRegister = p.registerBase;
            range.RegisterSpace = p.space;
            range.OffsetInDescriptorsFromTableStart = 0;
            rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rp.DescriptorTable.NumDescriptorRanges = 1;
            rp.DescriptorTable.pDescriptorRanges = &range;
        }
    }

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters = static_cast<UINT>(rootParams.size());
    desc.pParameters = rootParams.empty() ? nullptr : rootParams.data();
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                 D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                 D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                 D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> errors;
    HRESULT hr = ::D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1,
                                               blob.GetAddressOf(), errors.GetAddressOf());
    if (FAILED(hr))
    {
        std::string message = "HlslD3D12Material: root signature serialization failed.";
        if (errors)
            message += std::string(" ") + static_cast<const char*>(errors->GetBufferPointer());
        throw ExceptionRenderError(message);
    }
    hr = context->getDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
                                                   IID_PPV_ARGS(rootSignature.GetAddressOf()));
    if (FAILED(hr))
        throw ExceptionRenderError("HlslD3D12Material: CreateRootSignature failed (" + hresultString(hr) + ").");
}

ID3D12PipelineState* HlslD3D12Material::Impl::getPipeline(uint32_t renderTargetFormat, uint32_t depthFormat,
                                                          const HlslD3D12RenderState& state)
{
    const uint64_t stateBits = (state.blend ? 1u : 0u) | (state.depthTest ? 2u : 0u) |
                               (state.depthWrite ? 4u : 0u) | (state.depthLessEqual ? 8u : 0u) |
                               (state.cullBackFaces ? 16u : 0u) | (state.wireframe ? 32u : 0u);
    const uint64_t key = static_cast<uint64_t>(renderTargetFormat) |
                         (static_cast<uint64_t>(depthFormat) << 16) | (stateBits << 32);
    auto it = pipelines.find(key);
    if (it != pipelines.end())
        return it->second.Get();

    const std::vector<uint8_t>& vs = program->getVertexBytecode();
    const std::vector<uint8_t>& ps = program->getPixelBytecode();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pd = {};
    pd.pRootSignature = rootSignature.Get();
    pd.VS = { vs.data(), vs.size() };
    pd.PS = { ps.data(), ps.size() };

    D3D12_RENDER_TARGET_BLEND_DESC& blend = pd.BlendState.RenderTarget[0];
    blend.BlendEnable = state.blend ? TRUE : FALSE;
    blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blend.BlendOp = D3D12_BLEND_OP_ADD;
    blend.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
    blend.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blend.LogicOp = D3D12_LOGIC_OP_NOOP;
    blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    pd.SampleMask = UINT_MAX;

    // Front faces are clockwise in render target space, which matches
    // counter-clockwise front faces in the OpenGL convention since render
    // target space flips the vertical axis.
    pd.RasterizerState.FillMode = state.wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
    pd.RasterizerState.CullMode = state.cullBackFaces ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;
    pd.RasterizerState.FrontCounterClockwise = FALSE;
    pd.RasterizerState.DepthClipEnable = TRUE;

    pd.DepthStencilState.DepthEnable = state.depthTest ? TRUE : FALSE;
    pd.DepthStencilState.DepthWriteMask = state.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    pd.DepthStencilState.DepthFunc = state.depthLessEqual ? D3D12_COMPARISON_FUNC_LESS_EQUAL : D3D12_COMPARISON_FUNC_LESS;

    pd.InputLayout.pInputElementDescs = inputElements.empty() ? nullptr : inputElements.data();
    pd.InputLayout.NumElements = static_cast<UINT>(inputElements.size());
    pd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pd.NumRenderTargets = 1;
    pd.RTVFormats[0] = static_cast<DXGI_FORMAT>(renderTargetFormat);
    pd.DSVFormat = static_cast<DXGI_FORMAT>(depthFormat);
    pd.SampleDesc.Count = 1;

    ComPtr<ID3D12PipelineState> pipeline;
    const HRESULT hr = context->getDevice()->CreateGraphicsPipelineState(&pd, IID_PPV_ARGS(pipeline.GetAddressOf()));
    if (FAILED(hr))
    {
        throw ExceptionRenderError("HlslD3D12Material: CreateGraphicsPipelineState failed (" + hresultString(hr) +
                                   "). DXIL bytecode must be signed, which requires dxil.dll next to dxcompiler.dll.");
    }
    pipelines.emplace(key, pipeline);
    return pipeline.Get();
}

HlslD3D12Material::HlslD3D12Material(HlslD3D12ContextPtr context, HlslProgramPtr program) :
    _impl(new Impl())
{
    Impl& d = *_impl;
    d.context = std::move(context);
    d.program = std::move(program);

    if (!d.context || !d.context->getDevice())
        throw ExceptionRenderError("HlslD3D12Material: null device.");
    if (!d.program || !d.program->isValid())
        throw ExceptionRenderError("HlslD3D12Material: program not built.");

    d.vsReflection = d.program->getVertexReflection();
    d.psReflection = d.program->getPixelReflection();
    if (!d.vsReflection.isValid() || !d.psReflection.isValid())
        throw ExceptionRenderError("HlslD3D12Material: shader reflection failed.");

    d.rootDesc = HlslRootSignatureDesc::create(d.vsReflection, d.psReflection);
    if (d.rootDesc.getRootCost() > 64)
        throw ExceptionRenderError("HlslD3D12Material: root signature exceeds 64 DWORDs.");
    d.createRootSignature();

    for (const HlslReflectedCbuffer& cb : d.vsReflection.getCbuffers())
        d.vsCbuffers.emplace_back((cb.size + 15u) & ~15u, uint8_t(0));
    for (const HlslReflectedCbuffer& cb : d.psReflection.getCbuffers())
        d.psCbuffers.emplace_back((cb.size + 15u) & ~15u, uint8_t(0));

    for (const HlslRootParameter& p : d.rootDesc.getParameters())
    {
        CbufferSource source;
        if (p.type == HlslRootParameterType::ConstantBuffer)
        {
            source.stage = (p.visibility == HlslShaderVisibility::Vertex) ? Stage::Vertex : Stage::Pixel;
            const int index = d.reflection(source.stage).findCbuffer(p.name);
            source.cbufferIndex = index < 0 ? 0 : static_cast<std::size_t>(index);
        }
        d.cbufferSources.push_back(source);
    }

    d.vertexStride = buildHlslVertexLayout(d.vsReflection.getInputs(), d.vertexLayout);
    for (const HlslVertexElement& e : d.vertexLayout)
    {
        D3D12_INPUT_ELEMENT_DESC ied = {};
        ied.SemanticName = e.semanticName.c_str();
        ied.SemanticIndex = e.semanticIndex;
        ied.Format = toVertexFormat(e);
        ied.InputSlot = 0;
        ied.AlignedByteOffset = e.offset;
        ied.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        d.inputElements.push_back(ied);
    }
}

HlslD3D12Material::~HlslD3D12Material() = default;

const HlslStageReflection& HlslD3D12Material::getVertexReflection() const
{
    return _impl->vsReflection;
}

const HlslStageReflection& HlslD3D12Material::getPixelReflection() const
{
    return _impl->psReflection;
}

const std::vector<HlslResourceBinding>& HlslD3D12Material::getPixelBindings() const
{
    return _impl->psReflection.getBindings();
}

const HlslRootSignatureDesc& HlslD3D12Material::getRootSignatureDesc() const
{
    return _impl->rootDesc;
}

const std::vector<HlslVertexElement>& HlslD3D12Material::getVertexLayout() const
{
    return _impl->vertexLayout;
}

unsigned int HlslD3D12Material::getVertexStride() const
{
    return _impl->vertexStride;
}

bool HlslD3D12Material::setCbufferDataByName(Stage stage, const std::string& name, const void* data, std::size_t size)
{
    const int index = _impl->reflection(stage).findCbuffer(name);
    if (index < 0)
        return false;
    std::vector<uint8_t>& bytes = _impl->cbuffers(stage)[index];
    if (!data || size == 0)
        return false;
    const std::size_t n = std::min(size, bytes.size());
    std::memcpy(bytes.data(), data, n);
    std::fill(bytes.begin() + n, bytes.end(), uint8_t(0));
    return true;
}

bool HlslD3D12Material::setCbufferDataBySlot(Stage stage, unsigned int slot, const void* data, std::size_t size)
{
    const int index = _impl->findCbufferBySlot(stage, slot);
    if (index < 0)
        return false;
    return setCbufferDataByName(stage, _impl->reflection(stage).getCbuffers()[index].name, data, size);
}

std::size_t HlslD3D12Material::lookupVariableOffset(Stage stage, const std::string& cbufferName,
                                                    const std::string& memberName) const
{
    const HlslStageReflection& refl = _impl->reflection(stage);
    const int index = refl.findCbuffer(cbufferName);
    if (index < 0)
        return NOT_FOUND;
    for (const HlslReflectedVariable& var : refl.getCbuffers()[index].variables)
    {
        if (var.name == memberName)
            return var.offset;
    }
    return NOT_FOUND;
}

bool HlslD3D12Material::setCbufferRange(Stage stage, unsigned int slot, std::size_t offset,
                                        const void* data, std::size_t count)
{
    const int index = _impl->findCbufferBySlot(stage, slot);
    return index >= 0 && _impl->writeRange(stage, index, offset, data, count);
}

bool HlslD3D12Material::setCbufferRange(Stage stage, const std::string& cbufferName, std::size_t offset,
                                        const void* data, std::size_t count)
{
    const int index = _impl->reflection(stage).findCbuffer(cbufferName);
    return index >= 0 && _impl->writeRange(stage, index, offset, data, count);
}

bool HlslD3D12Material::patchVariable(Stage stage, const std::string& memberName, const void* data, std::size_t count)
{
    HlslUniformLocation location;
    if (!_impl->reflection(stage).findVariable(memberName, location))
        return false;
    return _impl->writeRange(stage, location.cbufferIndex, location.offset, data, count);
}

bool HlslD3D12Material::patchArrayMember(Stage stage, const std::string& arrayName, std::size_t index,
                                         const std::string& memberName, const void* data, std::size_t count)
{
    HlslUniformLocation location;
    if (!_impl->reflection(stage).findArrayMember(arrayName, index, memberName, location))
        return false;
    return _impl->writeRange(stage, location.cbufferIndex, location.offset, data, count);
}

void HlslD3D12Material::setTexture(unsigned int slot, uint64_t srvDescriptor)
{
    if (slot >= _impl->textures.size())
        _impl->textures.resize(static_cast<std::size_t>(slot) + 1, 0);
    _impl->textures[slot] = srvDescriptor;
}

void HlslD3D12Material::setSampler(unsigned int slot, uint64_t samplerDescriptor)
{
    if (slot >= _impl->samplers.size())
        _impl->samplers.resize(static_cast<std::size_t>(slot) + 1, 0);
    _impl->samplers[slot] = samplerDescriptor;
}

void HlslD3D12Material::bind(ID3D12GraphicsCommandList* commandList, uint32_t renderTargetFormat,
                             uint32_t depthFormat, const HlslD3D12RenderState& state)
{
    Impl& d = *_impl;
    if (!commandList)
        throw ExceptionRenderError("HlslD3D12Material::bind: null command list.");

    ID3D12Device* device = d.context->getDevice();
    commandList->SetGraphicsRootSignature(d.rootSignature.Get());
    commandList->SetPipelineState(d.getPipeline(renderTargetFormat, depthFormat, state));

    const std::vector<HlslRootParameter>& params = d.rootDesc.getParameters();
    for (std::size_t i = 0; i < params.size(); ++i)
    {
        const HlslRootParameter& p = params[i];
        const UINT rootIndex = static_cast<UINT>(i);
        if (p.type == HlslRootParameterType::ConstantBuffer)
        {
            const CbufferSource& source = d.cbufferSources[i];
            const std::vector<uint8_t>& bytes = d.cbuffers(source.stage)[source.cbufferIndex];
            const HlslD3D12UploadAllocation upload = d.context->allocateUpload(bytes.size(), 256);
            std::memcpy(upload.cpuAddress, bytes.data(), bytes.size());
            commandList->SetGraphicsRootConstantBufferView(rootIndex, upload.gpuAddress);
            continue;
        }

        const bool isTexture = (p.type == HlslRootParameterType::TextureTable);
        const HlslD3D12DescriptorType type = isTexture ? HlslD3D12DescriptorType::Resource
                                                       : HlslD3D12DescriptorType::Sampler;
        uint64_t cpuStart = 0;
        uint64_t gpuStart = 0;
        if (!d.context->allocateFrameDescriptors(type, p.registerCount, cpuStart, gpuStart))
            throw ExceptionRenderError("HlslD3D12Material::bind: shader-visible descriptor heap exhausted.");

        // Only pixel-stage registers in space 0 carry bound resources; the
        // rest of the table is filled with defaults.
        const bool bindable = (p.visibility == HlslShaderVisibility::Pixel && p.space == 0);
        const std::vector<uint64_t>& bound = isTexture ? d.textures : d.samplers;
        const uint64_t fallback = isTexture ? d.context->getNullTextureDescriptor()
                                            : d.context->getDefaultSamplerDescriptor();
        const unsigned int increment = d.context->getDescriptorIncrement(type);
        const D3D12_DESCRIPTOR_HEAP_TYPE heapType = isTexture ? D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                                                              : D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        for (unsigned int r = 0; r < p.registerCount; ++r)
        {
            const unsigned int reg = p.registerBase + r;
            const uint64_t src = (bindable && reg < bound.size() && bound[reg]) ? bound[reg] : fallback;
            device->CopyDescriptorsSimple(1, cpuHandle(cpuStart + static_cast<uint64_t>(r) * increment),
                                          cpuHandle(src), heapType);
        }
        commandList->SetGraphicsRootDescriptorTable(rootIndex, gpuHandle(gpuStart));
    }
}

MATERIALX_NAMESPACE_END
