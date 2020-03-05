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

    /// Render the current program to produce an image
    virtual void render() = 0;

    /// @}
    /// @name Utilities
    /// @{

    /// Save the current contents the offscreen hardware buffer to disk.
    /// @param filePath Path to file to save rendered image to.
    virtual void save(const FilePath& filePath) = 0;
    
    /// @}

  protected:
    // Protected constructor
    ShaderRenderer() { }

  protected:
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
