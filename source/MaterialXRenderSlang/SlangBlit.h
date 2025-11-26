//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SLANGBLIT_H
#define MATERIALX_SLANGBLIT_H

#include <MaterialXRenderSlang/Export.h>
#include <MaterialXRenderSlang/SlangRhi.h>
#include <map>

MATERIALX_NAMESPACE_BEGIN
class SlangContext;

class SlangBlit
{
  public:
    SlangBlit(SlangContext* context);

    void blit(
        SlangCommandEncoderPtr commandEncoder,
        rhi::ITextureView* dst,
        rhi::ITextureView* src,
        rhi::TextureFilteringMode filter = rhi::TextureFilteringMode::Linear);

    void blit(
        SlangCommandEncoderPtr commandEncoder,
        rhi::ITexture* dst,
        rhi::ITexture* src,
        rhi::TextureFilteringMode filter = rhi::TextureFilteringMode::Linear);

    void generateMips(
        SlangCommandEncoderPtr commandEncoder,
        rhi::ITexture* texture);

  private:
    rhi::ComPtr<rhi::IRenderPipeline> SlangBlit::getPipeline(rhi::Format dstFormat);

  private:
    SlangContext* _context;
    rhi::ComPtr<rhi::ISampler> _linearSampler;
    rhi::ComPtr<rhi::ISampler> _pointSampler;

    rhi::ComPtr<rhi::IShaderProgram> _program;
    std::map<rhi::Format, rhi::ComPtr<rhi::IRenderPipeline>> _pipelineCache;

    rhi::ComPtr<rhi::IShaderProgram> SlangBlit::getProgram();

    static const std::string BLIT_CODE;
};

MATERIALX_NAMESPACE_END

#endif
