#ifndef MATERIALX_SGOPTIONS_H
#define MATERIALX_SGOPTIONS_H

namespace MaterialX
{

/// Type of shader interface to be generated
enum class ShaderInterfaceType
{
    /// Create a complete interface with uniforms for all
    /// editable inputs on all nodes used by the shader.
    /// This interface makes the shader fully editable by
    /// value without requiring any rebuilds.
    /// This is the default interface type.
    COMPLETE,

    /// Create a reduced interface with uniforms only for
    /// the inputs that has been declared in the shaders
    /// nodedef interface. If values on other inputs are
    /// changed the shader needs to be rebuilt.
    REDUCED
};

/// Class holding options to configure shader generation.
class SgOptions
{
public:
	SgOptions();

    // TODO: Add options for:
    //  - shader gen optimization level
    //  - graph flattening or not

	// Sets the type of shader interface to be generated
    ShaderInterfaceType shaderInterfaceType;
};

} // namespace MaterialX

#endif
