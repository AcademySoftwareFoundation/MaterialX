//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SHADERVALIDATOR_H
#define MATERIALX_SHADERVALIDATOR_H

/// @file
/// Base class for shader validation

#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/ViewHandler.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

/// Shared pointer to a shader validator
using ShaderValidatorPtr = std::shared_ptr<class ShaderValidator>;

/// @class ShaderValidator
/// Base class for a shader validator
///
class ShaderValidator
{
  public:
    /// A map with name and source code for each shader stage.
    using StageMap = StringMap;

  public:
    virtual ~ShaderValidator() { }

    /// @name Setup
    /// @{

    /// Validator initialization 
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
    GeometryHandlerPtr getGeometryHandler()
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
    /// @name Validation
    /// @{

    /// Validate creation of program based on an input shader
    /// @param shader Input Shader
    virtual void validateCreation(const ShaderPtr shader) = 0;

    /// Validate creation of program based on shader stage source code.
    /// @param stages Map of name and source code for the shader stages.
    virtual void validateCreation(const StageMap& stages) = 0;

    /// Validate inputs for the program 
    virtual void validateInputs() = 0;

    /// Perform validation that inputs can be bound to and 
    /// Uendered with. Rendering is to an offscreen hardware buffer.
    virtual void validateRender() = 0;

    /// @}
    /// @name Utilities
    /// @{

    /// Save the current contents the offscreen hardware buffer to disk.
    /// @param filePath Path to file to save rendered image to.
    /// @param floatingPoint Format of output image is floating point.
    virtual void save(const FilePath& filePath, bool floatingPoint) = 0;
    
    /// @}

  protected:
    // Protected constructor
    ShaderValidator() { }

  protected:
    ImageHandlerPtr _imageHandler;
    GeometryHandlerPtr _geometryHandler;
    LightHandlerPtr _lightHandler;
    ViewHandlerPtr _viewHandler;
};

/// Error string list type
using ShaderValidationErrorList = StringVec;

/// @class ExceptionShaderValidationError
/// An exception that is thrown when shader validation fails.
/// An error log of shader errors is cached as part of the exception.
/// For example, if shader compilation fails, then a list of compilation errors is cached.
class ExceptionShaderValidationError : public Exception
{
  public:
    ExceptionShaderValidationError(const string& msg, const ShaderValidationErrorList& errorList) :
        Exception(msg),
        _errorLog(errorList)
    {
    }

    ExceptionShaderValidationError(const ExceptionShaderValidationError& e) :
        Exception(e),
        _errorLog(e._errorLog)
    {
    }

    ExceptionShaderValidationError& operator=(const ExceptionShaderValidationError& e)         
    {
        Exception::operator=(e);
        _errorLog = e._errorLog;
        return *this;
    }

    const ShaderValidationErrorList& errorLog() const
    {
        return _errorLog;
    }

  private:
    ShaderValidationErrorList _errorLog;
};

} // namespace MaterialX

#endif
