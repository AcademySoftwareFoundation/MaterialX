#ifndef MATERIALX_SGOPTIONS_H
#define MATERIALX_SGOPTIONS_H

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

enum HwTransparencyMethod
{
    /// No transparency will be used.
    /// The surface will appear fully opaque
    TRANSPARENCY_NONE,

    /// Alpha blending will be used for transparency.
    TRANSPARENCY_ALPHA_BLENDING
};

/// Class holding options to configure shader generation.
class SgOptions
{
public:
    SgOptions();

    virtual ~SgOptions() {}

    // TODO: Add options for:
    //  - shader gen optimization level
    //  - graph flattening or not

    // Sets the type of shader interface to be generated
    int shaderInterfaceType;

    // Sets the transparency method to use for HW shaders
    // If a surface shader has potential of being transparent
    // this option must be set, otherwise the surface will be
    // fully opaque.
    int hwTransparencyMethod;
};

} // namespace MaterialX

#endif
