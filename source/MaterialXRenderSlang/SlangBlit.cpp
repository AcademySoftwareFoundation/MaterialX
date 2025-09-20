//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderSlang/SlangBlit.h>
#include <MaterialXRenderSlang/SlangContext.h>

MATERIALX_NAMESPACE_BEGIN

SlangBlit::SlangBlit(SlangContext* context) :
    _context(context)
{
    rhi::SamplerDesc samplerDesc = {};
    samplerDesc.minFilter = rhi::TextureFilteringMode::Linear;
    samplerDesc.magFilter = rhi::TextureFilteringMode::Linear;
    _linearSampler = _context->getDevice()->createSampler(samplerDesc);

    samplerDesc.minFilter = rhi::TextureFilteringMode::Point;
    samplerDesc.magFilter = rhi::TextureFilteringMode::Point;
    _pointSampler = _context->getDevice()->createSampler(samplerDesc);

    getProgram();
}

void SlangBlit::blit(
    SlangCommandEncoderPtr commandEncoder,
    rhi::ITextureView* dst,
    rhi::ITextureView* src,
    rhi::TextureFilteringMode filter)
{
    rhi::ITexture* dstTexture = dst->getTexture();

    uint32_t dstMipLevel = dst->getDesc().subresourceRange.mip;

    auto getMipSize = [](rhi::ITexture* texture, uint32_t mipLevel) -> rhi::Extent3D
    {
        rhi::SubresourceLayout layout;
        texture->getSubresourceLayout(mipLevel, &layout);
        return layout.size;
    };

    rhi::Extent3D dstSize = getMipSize(dstTexture, dstMipLevel);

    auto pipeline = getPipeline(dstTexture->getDesc().format);
    SlangRenderPassDesc passDesc;
    passDesc.colorAttachments.resize(1);
    passDesc.colorAttachments[0].setView(rhi::ComPtr(dst));

    auto passEncoder = beginRenderPass(commandEncoder, passDesc);
    rhi::ShaderCursor cursor = rhi::ShaderCursor(passEncoder->bindPipeline(pipeline));

    SlangRenderState renderState;
    renderState.viewports[0] = rhi::Viewport::fromSize((float) dstSize.width, (float) dstSize.height);
    renderState.viewportCount = 1;
    renderState.scissorRects[0] = rhi::ScissorRect::fromSize(dstSize.width, dstSize.height);
    renderState.scissorRectCount = 1;

    passEncoder->setRenderState(renderState);
    cursor["src"].setBinding(src);
    cursor["sampler"].setBinding(filter == rhi::TextureFilteringMode::Linear ? _linearSampler : _pointSampler);

    rhi::DrawArguments args;
    args.vertexCount = 3;
    passEncoder->draw(args);
    passEncoder->end();
}

void SlangBlit::blit(
    SlangCommandEncoderPtr commandEncoder,
    rhi::ITexture* dst,
    rhi::ITexture* src,
    rhi::TextureFilteringMode filter)
{
    blit(commandEncoder, dst->createView({}), src->createView({}), filter);
}

void SlangBlit::generateMips(
    SlangCommandEncoderPtr commandEncoder,
    rhi::ITexture* texture)
{
    for (uint32_t i = 0; i < texture->getDesc().mipCount - 1; ++i)
    {
        rhi::TextureViewDesc srcDesc;
        srcDesc.subresourceRange.mip = i;
        srcDesc.subresourceRange.mipCount = 1;
        srcDesc.subresourceRange.layer = 0;
        srcDesc.subresourceRange.layerCount = 1;
        auto src = texture->createView(srcDesc);

        rhi::TextureViewDesc dstDesc;
        dstDesc.subresourceRange.mip = i + 1;
        dstDesc.subresourceRange.mipCount = 1;
        dstDesc.subresourceRange.layer = 0;
        dstDesc.subresourceRange.layerCount = 1;
        auto dst = texture->createView(dstDesc);

        blit(commandEncoder, dst, src);
    }
}

rhi::ComPtr<rhi::IShaderProgram> SlangBlit::getProgram()
{
    using namespace rhi;

    if (_program)
        return _program;

    ComPtr<slang::ISession> slangSession = _context->getDevice()->getSlangSession();
    ComPtr<slang::IBlob> diagnosticsBlob;
    std::vector<slang::IComponentType*> componentTypes;
    ComPtr<slang::IEntryPoint> entryPoint;
    ComPtr<slang::IComponentType> linkedProgram;
    StringVec diagnosticVec;

    auto validateResult = [&](bool success)
    {
        if (diagnosticsBlob)
            diagnosticVec.push_back((const char*) diagnosticsBlob->getBufferPointer());

        if (!success)
            throw ExceptionRenderError("Failed to compile blit shaders", diagnosticVec);
    };

    const std::string& source = BLIT_CODE;

    std::string name = _context->deduplicateName("blit");
    slang::IModule* module = slangSession->loadModuleFromSourceString(
        name.c_str(), name.c_str(), source.c_str(), diagnosticsBlob.writeRef());

    validateResult(module);

    Result result;

    componentTypes.push_back(module);
    result = module->findEntryPointByName("vs_main", entryPoint.writeRef());
    validateResult(SLANG_SUCCEEDED(result));
    componentTypes.push_back(entryPoint);

    result = module->findEntryPointByName("fs_main", entryPoint.writeRef());
    validateResult(SLANG_SUCCEEDED(result));
    componentTypes.push_back(entryPoint);

    ComPtr<slang::IComponentType> composedProgram;
    result = slangSession->createCompositeComponentType(
        componentTypes.data(),
        componentTypes.size(),
        composedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    validateResult(SLANG_SUCCEEDED(result));

    result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
    validateResult(SLANG_SUCCEEDED(result));

    _program = _context->getDevice()->createShaderProgram(linkedProgram, diagnosticsBlob.writeRef());
    validateResult(_program);

    return _program;
}
rhi::ComPtr<rhi::IRenderPipeline> SlangBlit::getPipeline(rhi::Format dstFormat)
{
    if (auto it = _pipelineCache.find(dstFormat); it != _pipelineCache.end())
        return it->second;

    auto program = getProgram();

    rhi::ColorTargetDesc colorTarget = {};
    colorTarget.format = dstFormat;

    rhi::RenderPipelineDesc pipelineDesc = {};
    pipelineDesc.program = program;
    pipelineDesc.targets = &colorTarget;
    pipelineDesc.targetCount = 1;

    auto pipeline = _context->getDevice()->createRenderPipeline(pipelineDesc);
    if (!pipeline)
        throw ExceptionRenderError("Failed to compile pipeline.");

    _pipelineCache[dstFormat] = pipeline;
    return pipeline;
}

const std::string SlangBlit::BLIT_CODE = R"(
Texture2D src;
SamplerState sampler;

struct VSOut {
    float4 pos : SV_Position;
    float2 uv : UV;
};

[shader("vertex")]
VSOut vs_main(uint vid: SV_VertexID)
{
    VSOut vs_out;
    vs_out.uv = float2((vid << 1) & 2, vid & 2);
    vs_out.pos = float4(vs_out.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return vs_out;
}

[shader("fragment")]
float4 fs_main(VSOut vs_out)
    : SV_Target
{
    float2 uv = vs_out.uv;
    let value = src.Sample(sampler, uv);
    return value;
}
)";

MATERIALX_NAMESPACE_END
