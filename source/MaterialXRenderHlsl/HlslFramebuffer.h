//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLFRAMEBUFFER_H
#define MATERIALX_HLSLFRAMEBUFFER_H

/// @file
/// Off-screen render target with CPU readback for the HLSL renderer.

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslContext.h>

#include <MaterialXRender/Image.h>

struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;

MATERIALX_NAMESPACE_BEGIN

class HlslFramebuffer;
using HlslFramebufferPtr = shared_ptr<class HlslFramebuffer>;

/// @class HlslFramebuffer
/// Off-screen render target backed by an `ID3D11Texture2D`, paired with a
/// depth-stencil texture and a CPU-mappable staging texture for readback.
///
/// Workflow:
/// @code
///   auto fb = HlslFramebuffer::create(ctx, 512, 512);
///   fb->bind();
///   fb->clear(Color4(0,0,0,1));
///   // ... draw calls into the immediate context ...
///   fb->unbind();
///   ImagePtr img = fb->readColor();        // CPU-side image
/// @endcode
///
/// Color format defaults to DXGI_FORMAT_R8G8B8A8_UNORM. A future
/// extension can take an explicit format for HDR / float targets.
class MX_RENDERHLSL_API HlslFramebuffer
{
  public:
    HlslFramebuffer(HlslContextPtr context, unsigned int width, unsigned int height);
    ~HlslFramebuffer();

    HlslFramebuffer(const HlslFramebuffer&) = delete;
    HlslFramebuffer& operator=(const HlslFramebuffer&) = delete;

    static HlslFramebufferPtr create(HlslContextPtr context, unsigned int width, unsigned int height)
    {
        return std::make_shared<HlslFramebuffer>(context, width, height);
    }

    unsigned int getWidth()  const { return _width; }
    unsigned int getHeight() const { return _height; }

    /// Set the color and depth-stencil targets and a full viewport. After
    /// this the immediate context's OM and RS state target this framebuffer.
    void bind();

    /// Restore the immediate context to a null render target.
    void unbind();

    /// Clear the bound color attachment to `color` and the depth attachment
    /// to 1.0. Must be called between bind() and unbind(), or with the
    /// framebuffer otherwise made the active render target.
    void clear(const Color4& color);

    /// Read the color attachment back to CPU memory and return it as a
    /// MaterialX Image. The image is RGBA8 with origin in the top-left
    /// (D3D's natural orientation).
    ImagePtr readColor();

    /// Borrowed accessor for the color shader-resource view, so a sampled
    /// pass can read this framebuffer as an input texture.
    ID3D11ShaderResourceView* getColorSrv() const { return _colorSrv; }

  private:
    HlslContextPtr _context;
    unsigned int _width;
    unsigned int _height;

    ID3D11Texture2D* _colorTexture = nullptr;
    ID3D11Texture2D* _depthTexture = nullptr;
    ID3D11Texture2D* _stagingTexture = nullptr;
    ID3D11RenderTargetView* _colorRtv = nullptr;
    ID3D11DepthStencilView* _depthDsv = nullptr;
    ID3D11ShaderResourceView* _colorSrv = nullptr;
};

MATERIALX_NAMESPACE_END

#endif
