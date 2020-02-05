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

    /// Bind the frame buffer for rendering.
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

    /// Blit our color image to the back buffer.
    void blit();

  protected:
    GLFramebuffer(unsigned int width, unsigned int height, unsigned int channelCount, Image::BaseType baseType);

  protected:
    unsigned int _width;
    unsigned int _height;
    unsigned int _channelCount;
    Image::BaseType _baseType;

    unsigned int _frameBuffer;
    unsigned int _colorTexture;
    unsigned int _depthTexture;
};

} // namespace MaterialX

#endif
