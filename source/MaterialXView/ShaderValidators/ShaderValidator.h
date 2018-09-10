#ifndef MATERIALX_SHADERVALIDATOR_H
#define MATERIALX_SHADERVALIDATOR_H

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/HwLightHandler.h>
#include <MaterialXView/ShaderValidators/ExceptionShaderValidationError.h>
#include <MaterialXView/Handlers/ImageHandler.h>
#include <MaterialXView/Handlers/GeometryHandler.h>
#include <MaterialXView/Handlers/ViewHandler.h>
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

    /// Get image handler
    /// @return Shared pointer to an image handler
    const ImageHandlerPtr getImageHandler() const
    {
        return _imageHandler;
    }

    /// Set light handler to use for light bindings
    /// @param imageHandler Handler used for lights
    void setLightHandler(const HwLightHandlerPtr lightHandler)
    {
        _lightHandler = lightHandler;
    }

    /// Get light handler
    /// @return Shared pointer to a light handler
    const HwLightHandlerPtr getLightHandler() const
    {
        return _lightHandler;
    }

    /// Set geometry handler for geometry load.
    /// By default a validator will use the DefaultGeometryHanlder
    /// @param geometryHandler Handler to use
    void setGeometryHandler(const GeometryHandlerPtr geometryHandler)
    {
        _geometryHandler = geometryHandler;
    }

    /// Get geometry handler
    /// @return Shared pointer to a geometry handler
    const GeometryHandlerPtr getGeometryHandler() const
    {
        return _geometryHandler;
    }

    /// Set viewing utilities handler.
    /// @param viewHandler Handler to use
    void setViewHandler(const ViewHandlerPtr viewHandler)
    {
        _viewHandler = viewHandler;
    }

    /// Get viewing utilities handler
    /// @return Shared pointer to a view utilities handler
    const ViewHandlerPtr getViewHandler() const
    {
        return _viewHandler;
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
    /// Uendered with. Rendering is to an offscreen hardware buffer.
    /// @param orthographicView Render orthographically
    virtual void validateRender(bool orthographicView = true) = 0;

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
    HwLightHandlerPtr _lightHandler;

    /// Viewing utilities handler
    ViewHandlerPtr _viewHandler;
};

} // namespace MaterialX
#endif
