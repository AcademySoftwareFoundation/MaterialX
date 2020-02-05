//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/GLFramebuffer.h>
#include <MaterialXRenderGlsl/GlslProgram.h>
#include <MaterialXRenderGlsl/GlslRenderer.h>

#include <MaterialXRenderGlsl/External/GLew/glew.h>

namespace MaterialX
{

//
// GLFramebuffer methods
//

GLFrameBufferPtr GLFramebuffer::create(unsigned int width, unsigned int height, unsigned channelCount, Image::BaseType baseType)
{
    return GLFrameBufferPtr(new GLFramebuffer(width, height, channelCount, baseType));
}

GLFramebuffer::GLFramebuffer(unsigned int width, unsigned int height, unsigned int channelCount, Image::BaseType baseType) :
    _width(width),
    _height(height),
    _channelCount(channelCount),
    _baseType(baseType),
    _frameBuffer(0),
    _colorTexture(0),
    _depthTexture(0)
{
    StringVec errors;
    const string errorType("OpenGL target creation failure.");

    if (!glGenFramebuffers)
    {
        glewInit();
    }

    // Set up frame buffer
    glGenFramebuffers(1, &_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);

    // Compute texture formats.
    GLenum format = GL_RGBA;
    GLenum internalFormat = GL_SRGB8_ALPHA8;
    GLenum type = GL_UNSIGNED_BYTE;
    if (channelCount == 3)
    {
        format = GL_RGB;
    }
    else if (channelCount == 2)
    {
        format = GL_RG;
    }
    else if (channelCount == 1)
    {
        format = GL_RED;
    }
    if (baseType == Image::BaseType::FLOAT)
    {
        internalFormat = GL_RGBA32F;
        type = GL_FLOAT;
    }
    else if (baseType == Image::BaseType::HALF)
    {
        internalFormat = GL_RGB16F;
        type = GL_HALF_FLOAT;
    }

    // Create the offscreen color target and attach to the framebuffer.
    _colorTexture = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
    glGenTextures(1, &_colorTexture);
    glBindTexture(GL_TEXTURE_2D, _colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _width, _height, 0, format, type, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexture, 0);

    // Create the offscreen depth target and attach to the framebuffer.
    _depthTexture = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
    glGenTextures(1, &_depthTexture);
    glBindTexture(GL_TEXTURE_2D, _depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _width, _height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);

    glBindTexture(GL_TEXTURE_2D, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
    glDrawBuffer(GL_NONE);

    // Validate the framebuffer.
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glDeleteFramebuffers(1, &_frameBuffer);
        _frameBuffer = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;

        string errorMessage("Frame buffer object setup failed: ");
        switch (status)
        {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            errorMessage += "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            errorMessage += "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            errorMessage += "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            errorMessage += "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            errorMessage += "GL_FRAMEBUFFER_UNSUPPORTED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            errorMessage += "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            break;
        case GL_FRAMEBUFFER_UNDEFINED:
            errorMessage += "GL_FRAMEBUFFER_UNDEFINED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            errorMessage += "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
            break;
        default:
            errorMessage += std::to_string(status);
            break;
        }

        errors.push_back(errorMessage);
        throw ExceptionShaderRenderError(errorType, errors);
    }

    // Unbind on cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
}

GLFramebuffer::~GLFramebuffer()
{
    if (_frameBuffer)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glDeleteTextures(1, &_colorTexture);
        glDeleteTextures(1, &_depthTexture);
        glDeleteFramebuffers(1, &_frameBuffer);
    }
}

void GLFramebuffer::bind()
{
    if (!_frameBuffer)
    {
        StringVec errors;
        errors.push_back("No framebuffer exists to bind.");
        throw ExceptionShaderRenderError("OpenGL target bind failure.", errors);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
    GLenum colorList[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, colorList);
}

void GLFramebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
    glDrawBuffer(GL_NONE);
}

ImagePtr GLFramebuffer::createColorImage()
{
    ImagePtr image = Image::create(_width, _height, _channelCount, _baseType);
    image->createResourceBuffer();

    GLenum format = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE;
    if (_channelCount == 3)
    {
        format = GL_RGB;
    }
    else if (_channelCount == 2)
    {
        format = GL_RG;
    }
    else if (_channelCount == 1)
    {
        format = GL_RED;
    }
    if (_baseType == Image::BaseType::FLOAT)
    {
        type = GL_FLOAT;
    }
    else if (_baseType == Image::BaseType::HALF)
    {
        type = GL_HALF_FLOAT;
    }

    bind();
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, image->getWidth(), image->getHeight(), format, type, image->getResourceBuffer());
    unbind();

    return image;
}

void GLFramebuffer::blit()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _frameBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);

    glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace MaterialX
