//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SHADERVALIDATOR_H
#define MATERIALX_SHADERVALIDATOR_H

#include <MaterialXGenShader/Shader.h>
#include <MaterialXRender/ShaderValidators/ExceptionShaderValidationError.h>
#include <MaterialXRender/Handlers/ImageHandler.h>
#include <MaterialXRender/Handlers/GeometryHandler.h>
#include <MaterialXRender/Handlers/ViewHandler.h>
#include <MaterialXRender/Handlers/HwLightHandler.h>
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
    /// A map with name and source code for each shader stage.
    using StageMap = std::unordered_map<string, string>;

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
    /// @param lightHandler Handler used for lights
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

    /// Get geometry handler
    /// @return Reference to a geometry handler
    GeometryHandler& getGeometryHandler()
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

    /// Validate creation of program based on shader stage source code.
    /// @param stages Map of name and source code for the shader stages.
    virtual void validateCreation(const StageMap& stages) = 0;

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
    /// @param floatingPoint Format of output image is floating point.
    virtual void save(const std::string& fileName, bool floatingPoint) = 0;
    
    /// @}

  protected:
    /// Constructor
    ShaderValidator() {};

    /// Utility image handler
    ImageHandlerPtr _imageHandler;

    /// Utility geometry handler
    GeometryHandler _geometryHandler;

    /// Utility light handler
    HwLightHandlerPtr _lightHandler;

    /// Viewing utilities handler
    ViewHandlerPtr _viewHandler;
};

} // namespace MaterialX
#endif
