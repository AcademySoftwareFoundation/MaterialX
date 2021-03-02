//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SHADERRENDERER_H
#define MATERIALX_SHADERRENDERER_H

/// @file
/// Base class for shader rendering

#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/ViewHandler.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

/// Shared pointer to a shader renderer
using ShaderRendererPtr = std::shared_ptr<class ShaderRenderer>;

/// @class ShaderRenderer
/// Helper class for rendering generated shader code to produce images.
class ShaderRenderer
{
  public:
    /// A map with name and source code for each shader stage.
    using StageMap = StringMap;

  public:
    virtual ~ShaderRenderer() { }

    /// @name Setup
    /// @{

    /// Renderer initialization 
    virtual void initialize() = 0;

    /// Set image handler to use for image load and save
    /// @param imageHandler Handler used to save image
    void setImageHandler(ImageHandlerPtr imageHandler)
    {
        _imageHandler = imageHandler;
    }

    /// Get image handler
    /// @return Shared pointer to an image handler
    ImageHandlerPtr getImageHandler() const
    {
        return _imageHandler;
    }

    /// Set light handler to use for light bindings
    /// @param lightHandler Handler used for lights
    void setLightHandler(LightHandlerPtr lightHandler)
    {
        _lightHandler = lightHandler;
    }

    /// Get light handler
    /// @return Shared pointer to a light handler
    LightHandlerPtr getLightHandler() const
    {
        return _lightHandler;
    }

    /// Get geometry handler
    /// @return Reference to a geometry handler
    GeometryHandlerPtr getGeometryHandler() const
    {
        return _geometryHandler;
    }

    /// Set viewing utilities handler.
    /// @param viewHandler Handler to use
    void setViewHandler(ViewHandlerPtr viewHandler)
    {
        _viewHandler = viewHandler;
    }

    /// Get viewing utilities handler
    /// @return Shared pointer to a view utilities handler
    ViewHandlerPtr getViewHandler() const
    {
        return _viewHandler;
    }

    /// @}
    /// @name Rendering
    /// @{

    /// Create program based on an input shader
    /// @param shader Input Shader
    virtual void createProgram(ShaderPtr shader) = 0;

    /// Create program based on shader stage source code.
    /// @param stages Map of name and source code for the shader stages.
    virtual void createProgram(const StageMap& stages) = 0;

    /// Validate inputs for the program 
    virtual void validateInputs() = 0;

    /// Set the size of the rendered image
    virtual void setSize(unsigned int /*width*/, unsigned int /*height*/) = 0;

    /// Render the current program to produce an image
    virtual void render() = 0;

    /// @}
    /// @name Utilities
    /// @{

    /// Capture the current contents of the off-screen hardware buffer as an image.
    virtual ImagePtr captureImage() = 0;

    /// Save the current contents of the off-screen hardware buffer to disk.
    virtual void saveImage(const FilePath& filePath, ConstImagePtr image, bool verticalFlip) = 0;

    /// Load images referenced by shader program and return list of images loaded
    virtual ImageVec getReferencedImages (const ShaderPtr& shader);

    /// @}

  protected:
    // Protected constructor
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

    ImageHandlerPtr _imageHandler;
    GeometryHandlerPtr _geometryHandler;
    LightHandlerPtr _lightHandler;
    ViewHandlerPtr _viewHandler;
};

/// @class ExceptionShaderRenderError
/// An exception that is thrown when shader rendering fails.
/// An error log of shader errors is cached as part of the exception.
/// For example, if shader compilation fails, then a list of compilation errors is cached.
class ExceptionShaderRenderError : public Exception
{
  public:
    ExceptionShaderRenderError(const string& msg, const StringVec& errorList) :
        Exception(msg),
        _errorLog(errorList)
    {
    }

    ExceptionShaderRenderError(const ExceptionShaderRenderError& e) :
        Exception(e),
        _errorLog(e._errorLog)
    {
    }

    ExceptionShaderRenderError& operator=(const ExceptionShaderRenderError& e)         
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

} // namespace MaterialX

#endif
