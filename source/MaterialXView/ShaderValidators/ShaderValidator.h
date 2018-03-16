#ifndef MATERIALX_SHADERVALIDATOR_H
#define MATERIALX_SHADERVALIDATOR_H

#include <MaterialXShaderGen/Shader.h>
#include <MaterialXView/ShaderValidators/ExceptionShaderValidationError.h>
#include <MaterialXView/Handlers/ImageHandler.h>
#include <MaterialXView/Handlers/GeometryHandler.h>
#include <MaterialXView/Handlers/LightHandler.h>
#include <vector>
#include <string>

namespace MaterialX
{
/// Shared pointer to a shader validator
using ShaderValidatorPtr = std::shared_ptr<class ShaderValidator>;

/// @class @ShaderValidator
/// Base class for a shader validator
///
class ShaderValidator
{
  public:
    /// Destructor
    virtual ~ShaderValidator() {};

    /// @name Setup
    /// @{

    /// Validator initialization 
    virtual void initialize() = 0;

    /// Set image handler to use for image load and save
    /// @param imageHandler Handler used to save image
    void setImageHandler(const ImageHandlerPtr imageHandler)
    {
        _imageHandler = imageHandler;
    }

    /// Set light handler to use for light bindings
    /// @param imageHandler Handler used for lights
    void setLightHandler(const LightHandlerPtr lightHandler)
    {
        _lightHandler = lightHandler;
    }

    /// Set geometry handler for geometry load.
    /// By default a validator will use the DefaultGeometryHanlder
    /// @param geometryHandler Handler to use
    void setGeometryHandler(const GeometryHandlerPtr geometryHandler)
    {
        _geometryHandler = geometryHandler;
    }

    /// @}
    /// @name Validation
    /// @{

    /// Validate creation of program based on an input shader
    /// @param shader Input Shader
    virtual void validateCreation(const ShaderPtr shader) = 0;

    /// Validate creation of program based input shader stage strings
    /// @param shader Input stages List of stage string
    virtual void validateCreation(const std::vector<std::string>& stages) = 0;

    /// Validate inputs for the program 
    virtual void validateInputs() = 0;

    /// Perform validation that inputs can be bound to and 
    /// rendered with. Rendering is to an offscreen hardware buffer.
    virtual void validateRender() = 0;

    /// @}
    /// @name Utilities
    /// @{

    /// Save the current contents the offscreen hardware buffer to disk.
    /// @param fileName Name of file to save rendered image to.
    /// @return true if successful
    virtual void save(const std::string& fileName) = 0;
    
    /// @}

  protected:
    /// Constructor
    ShaderValidator() {};

    /// Utility image handler
    ImageHandlerPtr _imageHandler;

    /// Utility geometry handler
    GeometryHandlerPtr _geometryHandler;

    /// Utility light handler
    LightHandlerPtr _lightHandler;
};

} // namespace MaterialX
#endif
