//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SLANGRHI_H
#define MATERIALX_SLANGRHI_H

/// @file
/// Wraps the slang-rhi include with the necessary warning pragmas.
/// Provides convenience classes (e.g., lifetime management), and aliases.


#include <MaterialXRenderSlang/Export.h>

#ifdef _MSC_VER
    #pragma warning(disable : 4267)
#endif
#include <slang-rhi.h>
#include <slang-rhi/shader-cursor.h>
#ifdef _MSC_VER
    #pragma warning(default : 4267)
#endif

#include <optional>

MATERIALX_NAMESPACE_BEGIN

using SlangCommandEncoderPtr = rhi::ComPtr<rhi::ICommandEncoder>;
using SlangRenderState = rhi::RenderState;
using SlangRenderPipeline = rhi::IRenderPipeline;
using SlangShaderObject = rhi::IShaderObject;
using SlangDrawArguments = rhi::DrawArguments;
using SlangRenderPassEncoder = rhi::IRenderPassEncoder;
using SlangLoadOp = rhi::LoadOp;
using SlangStoreOp = rhi::StoreOp;
using SlangCommandEncoderPtr = rhi::ComPtr<rhi::ICommandEncoder>;
using SlangTexturePtr = rhi::ComPtr<rhi::ITexture>;

class SlangRenderPassColorAttachment
{
  public:
    void setView(rhi::ComPtr<rhi::ITextureView> view)
    {
        _view = view;
        _rhiColorAttachment.view = _view;
    }

    void setResolveTarget(rhi::ComPtr<rhi::ITextureView> resolveTarget)
    {
        _resolveTarget = _resolveTarget;
        _rhiColorAttachment.resolveTarget = resolveTarget;
    }

    void setLoadOp(rhi::LoadOp loadOp) { _rhiColorAttachment.loadOp = loadOp; }
    void setStoreOp(rhi::StoreOp storeOp) { _rhiColorAttachment.storeOp = storeOp; }

    template <typename T>
    void setClearValue(const T& clearValue)
    {
        static_assert(sizeof(T) == sizeof(_rhiColorAttachment.clearValue));
        memcpy(_rhiColorAttachment.clearValue, &clearValue, sizeof(T));
    }

    const rhi::RenderPassColorAttachment& getRhi() const { return _rhiColorAttachment; }

  private:
    rhi::ComPtr<rhi::ITextureView> _view;
    rhi::ComPtr<rhi::ITextureView> _resolveTarget;
    rhi::RenderPassColorAttachment _rhiColorAttachment;
};

class SlangRenderPassDepthStencilAttachment
{
  public:
    void setView(rhi::ComPtr<rhi::ITextureView> view)
    {
        _view = view;
        _rhiDepthStencilAttachment.view = _view;
    }

    void setDepthLoadOp(rhi::LoadOp depthLoadOp) { _rhiDepthStencilAttachment.depthLoadOp = depthLoadOp; }
    void setDepthStoreOp(rhi::StoreOp depthStoreOp) { _rhiDepthStencilAttachment.depthStoreOp = depthStoreOp; }
    void setDepthClearValue(float depthClearValue) { _rhiDepthStencilAttachment.depthClearValue = depthClearValue; }
    void setDepthReadOnly(bool depthReadOnly) { _rhiDepthStencilAttachment.depthReadOnly = depthReadOnly; }
    void setStencilLoadOp(rhi::LoadOp stencilLoadOp) { _rhiDepthStencilAttachment.stencilLoadOp = stencilLoadOp; }
    void setStencilStoreOp(rhi::StoreOp stencilStoreOp) { _rhiDepthStencilAttachment.stencilStoreOp = stencilStoreOp; }
    void setStencilClearValue(uint8_t stencilClearValue) { _rhiDepthStencilAttachment.stencilClearValue = stencilClearValue; }
    void setStencilReadOnly(uint8_t stencilReadOnly) { _rhiDepthStencilAttachment.stencilReadOnly = stencilReadOnly; }

    const rhi::RenderPassDepthStencilAttachment& getRhi() const { return _rhiDepthStencilAttachment; }

  private:
    rhi::ComPtr<rhi::ITextureView> _view;
    rhi::RenderPassDepthStencilAttachment _rhiDepthStencilAttachment;
};

struct SlangRenderPassDesc
{
    std::vector<SlangRenderPassColorAttachment> colorAttachments;
    std::optional<SlangRenderPassDepthStencilAttachment> depthStencilAttachment;
};

inline SlangRenderPassEncoder* beginRenderPass(rhi::ICommandEncoder* commandEncoder, const SlangRenderPassDesc& desc)
{
    rhi::RenderPassColorAttachment rhiColorAttachments[16];
    for (size_t i = 0; i < desc.colorAttachments.size(); ++i)
        rhiColorAttachments[i] = desc.colorAttachments[i].getRhi();

    rhi::RenderPassDesc renderPass = {};
    renderPass.colorAttachments = rhiColorAttachments;
    renderPass.colorAttachmentCount = (uint32_t) desc.colorAttachments.size();
    if (desc.depthStencilAttachment)
        renderPass.depthStencilAttachment = &desc.depthStencilAttachment->getRhi();

    return commandEncoder->beginRenderPass(renderPass);
};


MATERIALX_NAMESPACE_END

#endif
