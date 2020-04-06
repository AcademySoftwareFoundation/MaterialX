//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GLFRAMEBUFFER_H
#define MATERIALX_GLFRAMEBUFFER_H

/// @file
/// OpenGL framebuffer handling

#include <MaterialXRender/ImageHandler.h>

namespace MaterialX
{

class GLFramebuffer;

/// Shared pointer to a GLFramebuffer
using GLFrameBufferPtr = std::shared_ptr<GLFramebuffer>;

/// @class GLFramebuffer
/// Wrapper for an OpenGL framebuffer
class GLFramebuffer
{
  public:
    /// Create a new framebuffer
    static GLFrameBufferPtr create(unsigned int width, unsigned int height, unsigned int channelCount, Image::BaseType baseType);

    /// Destructor
    virtual ~GLFramebuffer();

    /// Resize the framebuffer
    void resize(unsigned int width, unsigned int height);

    /// Set the encode sRGB flag, which controls whether values written
    /// to the framebuffer are encoded to the sRGB color space.
    void setEncodeSrgb(bool encode)
    {
        _encodeSrgb = encode;
    }

    /// Return the encode sRGB flag.
    bool getEncodeSrgb()
    {
        return _encodeSrgb;
    }

    /// Bind the framebuffer for rendering.
    void bind();

    /// Unbind the frame buffer after rendering.
    void unbind();

    /// Return our color texture handle.
    unsigned int getColorTexture() const
    {
        return _colorTexture;
    }

    /// Return our depth texture handle.
    unsigned int getDepthTexture() const
    {
        return _depthTexture;
    }

    /// Create an image from our color texture.
    ImagePtr createColorImage();

    /// Blit our color texture to the back buffer.
    void blit();

  protected:
    GLFramebuffer(unsigned int width, unsigned int height, unsigned int channelCount, Image::BaseType baseType);

  protected:
    unsigned int _width;
    unsigned int _height;
    unsigned int _channelCount;
    Image::BaseType _baseType;
    bool _encodeSrgb;

    unsigned int _frameBuffer;
    unsigned int _colorTexture;
    unsigned int _depthTexture;
};

} // namespace MaterialX

#endif
