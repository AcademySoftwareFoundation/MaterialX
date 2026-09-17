//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLD3D12FRAMEBUFFER_H
#define MATERIALX_HLSLD3D12FRAMEBUFFER_H

/// @file
/// Off-screen D3D12 render target with CPU readback.

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslD3D12Context.h>

#include <MaterialXRender/Image.h>

struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

MATERIALX_NAMESPACE_BEGIN

class HlslD3D12Framebuffer;
using HlslD3D12FramebufferPtr = shared_ptr<class HlslD3D12Framebuffer>;

/// @class HlslD3D12Framebuffer
/// Off-screen color and depth targets for HlslD3D12Renderer, with CPU
/// readback of the color target.
///
/// The color target is RGBA8 for UINT8 framebuffers, stored typeless so
/// that an sRGB-encoding and a linear render target view can both be used,
/// matching HlslFramebuffer and the GLSL renderer's GL_FRAMEBUFFER_SRGB.
/// HALF and FLOAT framebuffers use RGBA16F and RGBA32F targets, which are
/// never sRGB encoded.
///
/// Workflow:
/// @code
///   ID3D12GraphicsCommandList* list = context->beginFrame();
///   fb->bind(list);
///   fb->clear(list, Color4(0, 0, 0, 1));
///   // ... draws ...
///   context->endFrame();
///   ImagePtr img = fb->readColor();
/// @endcode
class MX_RENDERHLSL_API HlslD3D12Framebuffer
{
  public:
    HlslD3D12Framebuffer(HlslD3D12ContextPtr context, unsigned int width, unsigned int height,
                         Image::BaseType baseType = Image::BaseType::UINT8);
    ~HlslD3D12Framebuffer();

    HlslD3D12Framebuffer(const HlslD3D12Framebuffer&) = delete;
    HlslD3D12Framebuffer& operator=(const HlslD3D12Framebuffer&) = delete;

    static HlslD3D12FramebufferPtr create(HlslD3D12ContextPtr context, unsigned int width, unsigned int height,
                                          Image::BaseType baseType = Image::BaseType::UINT8)
    {
        return std::make_shared<HlslD3D12Framebuffer>(context, width, height, baseType);
    }

    unsigned int getWidth() const;
    unsigned int getHeight() const;
    Image::BaseType getBaseType() const;

    /// Select whether shader writes are gamma-encoded (sRGB view) or stored
    /// linearly. Defaults to true. Only UINT8 framebuffers are affected.
    void setEncodeSrgb(bool encode);
    bool getEncodeSrgb() const;

    /// DXGI_FORMAT of the render target view selected by bind().
    uint32_t getRenderTargetFormat() const;

    /// DXGI_FORMAT of the depth stencil view.
    uint32_t getDepthFormat() const;

    /// Transition the color target for rendering and set the render
    /// targets, viewport and scissor rectangle on `commandList`.
    void bind(ID3D12GraphicsCommandList* commandList);

    /// Clear the color target to `color` and depth to 1.0. Call after bind().
    void clear(ID3D12GraphicsCommandList* commandList, const Color4& color);

    /// Clear depth to 1.0 without touching the color target.
    void clearDepth(ID3D12GraphicsCommandList* commandList);

    /// Copy the color target to CPU memory as a 4-channel image with its
    /// origin at the top-left. When `image` matches the framebuffer size and
    /// format it is filled and returned; otherwise a new image is returned.
    /// Must not be called while the context records a frame.
    ImagePtr readColor(ImagePtr image = nullptr);

    /// Borrowed color target resource.
    ID3D12Resource* getColorResource() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

MATERIALX_NAMESPACE_END

#endif
