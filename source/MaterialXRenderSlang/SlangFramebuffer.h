//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SLANGFRAMEBUFFER_H
#define MATERIALX_SLANGFRAMEBUFFER_H

/// @file
/// OpenGL texture handler

#include <MaterialXRenderSlang/Export.h>
#include <MaterialXRenderSlang/SlangContext.h>

#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/ShaderRenderer.h>

#include <map>
#include <unordered_map>

MATERIALX_NAMESPACE_BEGIN

using SlangFramebufferPtr = std::shared_ptr<class SlangFramebuffer>;

/// @class SlangFramebuffer
/// Wrapper for a Slang framebuffer
class MX_RENDERSLANG_API SlangFramebuffer
{
  public:
    /// Create a new framebuffer
    static SlangFramebufferPtr create(SlangContextPtr context,
                                      unsigned int width, unsigned int height,
                                      unsigned channelCount,
                                      Image::BaseType baseType,
                                      SlangTexturePtr colorTexture = nullptr,
                                      bool encodeSrgb = false,
                                      rhi::Format pixelFormat = rhi::Format::Undefined)
    {
        return SlangFramebufferPtr(new SlangFramebuffer(std::move(context), width, height, channelCount, baseType, std::move(colorTexture), encodeSrgb, pixelFormat));
    }
    static SlangFramebufferPtr create(SlangContextPtr context,
                                      SlangTexturePtr colorTexture)
    {
        return SlangFramebufferPtr(new SlangFramebuffer(std::move(context), std::move(colorTexture)));
    }

    /// Destructor
    virtual ~SlangFramebuffer();

    void resize(unsigned int width, unsigned int height, bool forceRecreate = false,
                rhi::Format pixelFormat = rhi::Format::Undefined,
                SlangTexturePtr extColorTexture = nullptr);

    SlangTexturePtr getDepthTexture() { return _depthTexture; }
    SlangTexturePtr getColorTexture() { return _colorTexture; }

    void setColorTexture(SlangTexturePtr newColorTexture);

    /// Return the width of the framebuffer.
    unsigned int getWidth() const
    {
        return _width;
    }

    /// Return the height of the framebuffer.
    unsigned int getHeight() const
    {
        return _height;
    }

    unsigned int getChannelCount() const
    {
        return _channelCount;
    }

    Image::BaseType getBaseType() const
    {
        return _baseType;
    }

    /// Set the encode sRGB flag, which controls whether values written
    /// to the framebuffer are encoded to the sRGB color space.
    void setEncodeSrgb(bool encode)
    {
        if (encode != _encodeSrgb)
        {
            _encodeSrgb = encode;
            resize(_width, _height, true);
        }
    }

    /// Return the encode sRGB flag.
    bool getEncodeSrgb()
    {
        return _encodeSrgb;
    }

    /// Return the color data of this framebuffer as an image.
    /// If an input image is provided, it will be used to store the color data;
    /// otherwise a new image of the required format will be created.
    ImagePtr getColorImage(ImagePtr image = nullptr, bool flipVertically = false);

    void bindRenderPassDesc(SlangRenderPassDesc& desc, int mipMapLevel = 0);
    void bindRenderState(SlangRenderState& state);

  protected:
    SlangFramebuffer(SlangContextPtr context,
                     unsigned int width, unsigned int height,
                     unsigned channelCount,
                     Image::BaseType baseType,
                     SlangTexturePtr colorTexture = nullptr,
                     bool encodeSrgb = false,
                     rhi::Format pixelFormat = rhi::Format::Undefined);
    SlangFramebuffer(SlangContextPtr context,
                     SlangTexturePtr colorTexture);

  protected:
    SlangContextPtr _context;
    rhi::ComPtr<rhi::IDevice> _device;
    unsigned int _width;
    unsigned int _height;
    unsigned int _channelCount;
    Image::BaseType _baseType;
    bool _encodeSrgb;
    rhi::Format _format;

    unsigned int _framebuffer;
    SlangTexturePtr _depthTexture;
    SlangTexturePtr _colorTexture;
};

MATERIALX_NAMESPACE_END

#endif