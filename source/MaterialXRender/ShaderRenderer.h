//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SHADERRENDERER_H
#define MATERIALX_SHADERRENDERER_H

/// @file
/// Base class for shader rendering

#include <MaterialXRender/Camera.h>
#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/LightHandler.h>

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a shader renderer
using ShaderRendererPtr = std::shared_ptr<class ShaderRenderer>;

/// @class ShaderRenderer
/// Base class for renderers that generate shader code to produce images.
class MX_RENDER_API ShaderRenderer
{
  public:
    /// A map with name and source code for each shader stage.
    using StageMap = StringMap;

  public:
    virtual ~ShaderRenderer() { }

    /// @name Setup
    /// @{

    /// Initialize the renderer.
    virtual void initialize() { }

    /// Set the camera.
    void setCamera(CameraPtr camera)
    {
        _camera = camera;
    }

    /// Return the camera.
    CameraPtr getCamera() const
    {
        return _camera;
    }

    /// Set the image handler used by this renderer for image I/O.
    void setImageHandler(ImageHandlerPtr imageHandler)
    {
        _imageHandler = imageHandler;
    }

    /// Return the image handler.
    ImageHandlerPtr getImageHandler() const
    {
        return _imageHandler;
    }

    /// Set the light handler used by this renderer for light bindings.
    void setLightHandler(LightHandlerPtr lightHandler)
    {
        _lightHandler = lightHandler;
    }

    /// Return the light handler.
    LightHandlerPtr getLightHandler() const
    {
        return _lightHandler;
    }

    /// Set the geometry handler.
    void setGeometryHandler(GeometryHandlerPtr geometryHandler)
    {
        _geometryHandler = geometryHandler;
    }

    /// Return the geometry handler.
    GeometryHandlerPtr getGeometryHandler() const
    {
        return _geometryHandler;
    }

    /// @}
    /// @name Rendering
    /// @{

    /// Create program based on an input shader.
    virtual void createProgram(ShaderPtr shader);

    /// Create program based on shader stage source code.
    /// @param stages Map of name and source code for the shader stages.
    virtual void createProgram(const StageMap& stages);

    /// Validate inputs for the program.
    virtual void validateInputs() { }

    /// Set the size of the rendered image.
    virtual void setSize(unsigned int width, unsigned int height);

    /// Render the current program to produce an image.
    virtual void render() { }

    /// @}
    /// @name Utilities
    /// @{

    /// Capture the current rendered output as an image.
    virtual ImagePtr captureImage(ImagePtr image = nullptr)
    {
        return nullptr;
    }

    /// @}

  protected:
    ShaderRenderer() :
        _width(0),
        _height(0),
        _baseType(Image::BaseType::UINT8)
    { }

    ShaderRenderer(unsigned int width, unsigned int height, Image::BaseType baseType) :
        _width(width),
        _height(height),
        _baseType(baseType)
    { }

  protected:
    unsigned int _width;
    unsigned int _height;
    Image::BaseType _baseType;

    CameraPtr _camera;
    ImageHandlerPtr _imageHandler;
    GeometryHandlerPtr _geometryHandler;
    LightHandlerPtr _lightHandler;
};

/// @class ExceptionRenderError
/// An exception that is thrown when a rendering operation fails.
/// Optionally stores an additional error log, which can be used to
/// store and retrieve shader compilation errors.
class MX_RENDER_API ExceptionRenderError : public Exception
{
  public:
    ExceptionRenderError(const string& msg, const StringVec& errorLog = StringVec()) :
        Exception(msg),
        _errorLog(errorLog)
    {
    }

    ExceptionRenderError(const ExceptionRenderError& e) :
        Exception(e),
        _errorLog(e._errorLog)
    {
    }

    ExceptionRenderError& operator=(const ExceptionRenderError& e)         
    {
        Exception::operator=(e);
        _errorLog = e._errorLog;
        return *this;
    }

    const StringVec& errorLog() const
    {
        return _errorLog;
    }

  private:
    StringVec _errorLog;
};

MATERIALX_NAMESPACE_END

#endif
