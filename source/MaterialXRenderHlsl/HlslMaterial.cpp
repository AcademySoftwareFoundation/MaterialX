//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslMaterial.h>

#include <MaterialXRender/ShaderRenderer.h>

#define NOMINMAX 1
#include <Windows.h>
#include <d3d11.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include <algorithm>
#include <cstring>

MATERIALX_NAMESPACE_BEGIN

namespace
{

using Microsoft::WRL::ComPtr;

void releaseAndNull(IUnknown** ptr)
{
    if (ptr && *ptr)
    {
        (*ptr)->Release();
        *ptr = nullptr;
    }
}

// Round size up to a 16-byte boundary, the alignment D3D11 requires for
// constant buffer creation.
UINT alignCbufferSize(UINT s)
{
    return (s + 15) & ~15u;
}

// Resolve the byte size of a cbuffer by its reflected name. We re-reflect
// here (rather than caching it in HlslProgram) because the binding list
// only carries name + slot, not byte size.
UINT lookupCbufferSize(const std::vector<uint8_t>& bytecode, const std::string& name)
{
    if (bytecode.empty())
        return 0;

    ComPtr<ID3D11ShaderReflection> refl;
    if (FAILED(::D3DReflect(bytecode.data(), bytecode.size(),
                            IID_PPV_ARGS(refl.GetAddressOf()))) || !refl)
        return 0;

    D3D11_SHADER_DESC desc = {};
    if (FAILED(refl->GetDesc(&desc)))
        return 0;

    for (UINT i = 0; i < desc.ConstantBuffers; ++i)
    {
        ID3D11ShaderReflectionConstantBuffer* cb = refl->GetConstantBufferByIndex(i);
        if (!cb)
            continue;
        D3D11_SHADER_BUFFER_DESC bd = {};
        if (FAILED(cb->GetDesc(&bd)))
            continue;
        if (bd.Name && name == bd.Name)
            return bd.Size;
    }
    return 0;
}

// Allocate a cbuffer pool for one stage given the bindings and bytecode
// for that stage. Stores the resulting buffers indexed by slot, a
// name->slot map for either-key access, and a CPU mirror for partial
// (read-modify-write) updates via setCbufferRange.
void allocateStageCbuffers(ID3D11Device* device,
                           const std::vector<HlslResourceBinding>& bindings,
                           const std::vector<uint8_t>& bytecode,
                           std::vector<ID3D11Buffer*>& cbuffersOut,
                           std::unordered_map<std::string, unsigned int>& nameToSlotOut,
                           std::vector<std::vector<uint8_t>>& mirrorsOut)
{
    unsigned int maxSlot = 0;
    bool hasAny = false;
    for (const auto& b : bindings)
    {
        if (b.type == HlslResourceType::CBuffer)
        {
            if (b.slot > maxSlot)
                maxSlot = b.slot;
            hasAny = true;
        }
    }
    if (!hasAny)
        return;

    cbuffersOut.assign(static_cast<std::size_t>(maxSlot) + 1, nullptr);
    mirrorsOut.assign(static_cast<std::size_t>(maxSlot) + 1, std::vector<uint8_t>{});
    for (const auto& b : bindings)
    {
        if (b.type != HlslResourceType::CBuffer)
            continue;
        const UINT size = alignCbufferSize(lookupCbufferSize(bytecode, b.name));
        if (size == 0)
            continue;
        D3D11_BUFFER_DESC bd = {};
        bd.ByteWidth = size;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0;
        std::vector<uint8_t> zero(size, 0);
        D3D11_SUBRESOURCE_DATA sd = {};
        sd.pSysMem = zero.data();
        ID3D11Buffer* buf = nullptr;
        if (SUCCEEDED(device->CreateBuffer(&bd, &sd, &buf)))
        {
            cbuffersOut[b.slot] = buf;
            nameToSlotOut.emplace(b.name, b.slot);
            mirrorsOut[b.slot] = std::move(zero);
        }
    }
}

} // namespace

HlslMaterial::HlslMaterial(HlslContextPtr context, HlslProgramPtr program) :
    _context(std::move(context)),
    _program(std::move(program))
{
    if (!_context || !_context->getDevice())
        throw ExceptionRenderError("HlslMaterial: null device.");
    if (!_program || !_program->isValid())
        throw ExceptionRenderError("HlslMaterial: program not built.");

    ID3D11Device* device = _context->getDevice();

    _vs = _program->createVertexShader(device);
    _ps = _program->createPixelShader(device);
    if (!_vs || !_ps)
        throw ExceptionRenderError("HlslMaterial: failed to instantiate shader objects.");

    // Allocate per-stage cbuffer pools from each stage's reflection. The
    // HLSL generator emits a separate cbuffer per stage (vertexCB,
    // pixelCB) so the two pools usually have different layouts and can't
    // be unified.
    _pixelBindings = _program->getPixelBindings();
    allocateStageCbuffers(device, _program->getVertexBindings(),
                          _program->getVertexBytecode(),
                          _vsCbuffers, _vsCbufferNameToSlot, _vsCbufferMirrors);
    allocateStageCbuffers(device, _pixelBindings,
                          _program->getPixelBytecode(),
                          _psCbuffers, _psCbufferNameToSlot, _psCbufferMirrors);
}

HlslMaterial::~HlslMaterial()
{
    releaseAndNull(reinterpret_cast<IUnknown**>(&_inputLayout));
    releaseAndNull(reinterpret_cast<IUnknown**>(&_ps));
    releaseAndNull(reinterpret_cast<IUnknown**>(&_vs));
    auto releaseAll = [](std::vector<ID3D11Buffer*>& v) {
        for (auto*& b : v)
            if (b) { b->Release(); b = nullptr; }
    };
    releaseAll(_vsCbuffers);
    releaseAll(_psCbuffers);
}

bool HlslMaterial::createInputLayout(const D3D11_INPUT_ELEMENT_DESC* elements,
                                     unsigned int count)
{
    releaseAndNull(reinterpret_cast<IUnknown**>(&_inputLayout));
    if (!elements || count == 0)
        return false;
    const auto& bc = _program->getVertexBytecode();
    if (bc.empty())
        return false;
    HRESULT hr = _context->getDevice()->CreateInputLayout(
        elements, count, bc.data(), bc.size(), &_inputLayout);
    return SUCCEEDED(hr);
}

bool HlslMaterial::setCbufferDataByName(Stage stage, const std::string& name, const void* data, std::size_t size)
{
    const auto& nameMap = (stage == Stage::Vertex) ? _vsCbufferNameToSlot : _psCbufferNameToSlot;
    auto it = nameMap.find(name);
    if (it == nameMap.end())
        return false;
    return setCbufferDataBySlot(stage, it->second, data, size);
}

bool HlslMaterial::setCbufferDataBySlot(Stage stage, unsigned int slot, const void* data, std::size_t size)
{
    auto& pool   = (stage == Stage::Vertex) ? _vsCbuffers      : _psCbuffers;
    auto& mirror = (stage == Stage::Vertex) ? _vsCbufferMirrors : _psCbufferMirrors;
    if (slot >= pool.size() || !pool[slot] || !data || size == 0)
        return false;
    D3D11_BUFFER_DESC bd = {};
    pool[slot]->GetDesc(&bd);
    const UINT bytesToWrite = static_cast<UINT>(std::min<std::size_t>(size, bd.ByteWidth));

    // Update the CPU mirror so subsequent setCbufferRange calls see the
    // bytes the caller just wrote, then push the (possibly zero-padded)
    // payload to the GPU.
    if (slot < mirror.size())
    {
        if (mirror[slot].size() < bd.ByteWidth)
            mirror[slot].resize(bd.ByteWidth, 0);
        std::memcpy(mirror[slot].data(), data, bytesToWrite);
        if (bytesToWrite < bd.ByteWidth)
            std::memset(mirror[slot].data() + bytesToWrite, 0, bd.ByteWidth - bytesToWrite);
        _context->getDeviceContext()->UpdateSubresource(pool[slot], 0, nullptr,
                                                        mirror[slot].data(), 0, 0);
    }
    else if (bytesToWrite < bd.ByteWidth)
    {
        std::vector<uint8_t> staging(bd.ByteWidth, 0);
        std::memcpy(staging.data(), data, bytesToWrite);
        _context->getDeviceContext()->UpdateSubresource(pool[slot], 0, nullptr,
                                                        staging.data(), 0, 0);
    }
    else
    {
        _context->getDeviceContext()->UpdateSubresource(pool[slot], 0, nullptr,
                                                        data, 0, 0);
    }
    return true;
}

bool HlslMaterial::setCbufferRange(Stage stage, unsigned int slot,
                                   std::size_t offset, const void* data, std::size_t count)
{
    auto& pool   = (stage == Stage::Vertex) ? _vsCbuffers       : _psCbuffers;
    auto& mirror = (stage == Stage::Vertex) ? _vsCbufferMirrors : _psCbufferMirrors;
    if (slot >= pool.size() || !pool[slot] || !data || count == 0)
        return false;
    if (slot >= mirror.size() || mirror[slot].empty())
        return false;
    if (offset + count > mirror[slot].size())
        return false;

    std::memcpy(mirror[slot].data() + offset, data, count);
    _context->getDeviceContext()->UpdateSubresource(pool[slot], 0, nullptr,
                                                    mirror[slot].data(), 0, 0);
    return true;
}

bool HlslMaterial::setCbufferRange(Stage stage, const std::string& cbufferName,
                                   std::size_t offset, const void* data, std::size_t count)
{
    const auto& nameMap = (stage == Stage::Vertex) ? _vsCbufferNameToSlot : _psCbufferNameToSlot;
    auto it = nameMap.find(cbufferName);
    if (it == nameMap.end())
        return false;
    return setCbufferRange(stage, it->second, offset, data, count);
}

bool HlslMaterial::patchVariable(Stage stage, const std::string& memberName,
                                 const void* data, std::size_t count)
{
    // Search every cbuffer on the requested stage and write into the
    // first one that owns the uniform. The HLSL generator emits each
    // uniform into exactly one cbuffer per stage, so the first match
    // is also the only match.
    const auto& nameMap = (stage == Stage::Vertex) ? _vsCbufferNameToSlot : _psCbufferNameToSlot;
    for (const auto& kv : nameMap)
    {
        const std::size_t off = lookupVariableOffset(stage, kv.first, memberName);
        if (off == std::size_t(-1))
            continue;
        return setCbufferRange(stage, kv.second, off, data, count);
    }
    return false;
}

std::size_t HlslMaterial::lookupVariableOffset(Stage stage, const std::string& cbufferName,
                                               const std::string& memberName) const
{
    const auto& bytecode = (stage == Stage::Vertex)
                         ? _program->getVertexBytecode()
                         : _program->getPixelBytecode();
    if (bytecode.empty())
        return std::size_t(-1);

    ComPtr<ID3D11ShaderReflection> refl;
    if (FAILED(::D3DReflect(bytecode.data(), bytecode.size(),
                            IID_PPV_ARGS(refl.GetAddressOf()))) || !refl)
        return std::size_t(-1);

    ID3D11ShaderReflectionConstantBuffer* cb = refl->GetConstantBufferByName(cbufferName.c_str());
    if (!cb)
        return std::size_t(-1);

    ID3D11ShaderReflectionVariable* var = cb->GetVariableByName(memberName.c_str());
    if (!var)
        return std::size_t(-1);

    D3D11_SHADER_VARIABLE_DESC vd = {};
    if (FAILED(var->GetDesc(&vd)))
        return std::size_t(-1);

    return vd.StartOffset;
}

void HlslMaterial::setTexture(unsigned int slot, ID3D11ShaderResourceView* srv)
{
    if (slot >= _textures.size())
        _textures.resize(static_cast<std::size_t>(slot) + 1, nullptr);
    _textures[slot] = srv;
}

void HlslMaterial::setSampler(unsigned int slot, ID3D11SamplerState* sampler)
{
    if (slot >= _samplers.size())
        _samplers.resize(static_cast<std::size_t>(slot) + 1, nullptr);
    _samplers[slot] = sampler;
}

void HlslMaterial::bind()
{
    ID3D11DeviceContext* dc = _context->getDeviceContext();

    dc->VSSetShader(_vs, nullptr, 0);
    dc->PSSetShader(_ps, nullptr, 0);

    if (!_vsCbuffers.empty())
    {
        dc->VSSetConstantBuffers(0, static_cast<UINT>(_vsCbuffers.size()), _vsCbuffers.data());
    }
    if (!_psCbuffers.empty())
    {
        dc->PSSetConstantBuffers(0, static_cast<UINT>(_psCbuffers.size()), _psCbuffers.data());
    }
    if (!_textures.empty())
    {
        dc->PSSetShaderResources(0, static_cast<UINT>(_textures.size()), _textures.data());
    }
    if (!_samplers.empty())
    {
        dc->PSSetSamplers(0, static_cast<UINT>(_samplers.size()), _samplers.data());
    }
}

MATERIALX_NAMESPACE_END
