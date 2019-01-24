#ifndef MATERIALX_GENOPTIONS_H
#define MATERIALX_GENOPTIONS_H

#include <MaterialXCore/Library.h>

namespace MaterialX
{

/// Type of shader interface to be generated
enum ShaderInterfaceType
{
    /// Create a complete interface with uniforms for all
    /// editable inputs on all nodes used by the shader.
    /// This interface makes the shader fully editable by
    /// value without requiring any rebuilds.
    /// This is the default interface type.
    SHADER_INTERFACE_COMPLETE,

    /// Create a reduced interface with uniforms only for
    /// the inputs that has been declared in the shaders
    /// nodedef interface. If values on other inputs are
    /// changed the shader needs to be rebuilt.
    SHADER_INTERFACE_REDUCED
};

/// Class holding options to configure shader generation.
class GenOptions
{
  public:
    GenOptions();

    virtual ~GenOptions() {}

    // TODO: Add options for:
    //  - shader gen optimization level
    //  - graph flattening or not

    /// Sets the type of shader interface to be generated
    int shaderInterfaceType;

    /// Sets if transparency is needed or not for HW shaders.
    /// If a surface shader has potential of being transparent
    /// this must be set to true, otherwise no transparency
    /// code fragments will be generated for the shader and
    /// the surface will be fully opaque.
    bool hwTransparency;

    /// An optional override for the target color space.
    /// Shader fragments will be generated to transform
    /// input values and textures into this color space.
    string targetColorSpaceOverride;
};

} // namespace MaterialX

#endif
