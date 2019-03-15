//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GENOPTIONS_H
#define MATERIALX_GENOPTIONS_H

/// @file
/// Shader generation options class

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

/// Method to use for specular environment lighting
enum HwSpecularEnvironmentMethod
{
    /// Use pre-filtered environment maps for
    /// specular environment/indirect lighting.
    SPECULAR_ENVIRONMENT_PREFILTER,

    /// Use Filtered Importance Sampling for
    /// specular environment/indirect lighting.
    SPECULAR_ENVIRONMENT_FIS
};

/// @class GenOptions 
/// Class holding options to configure shader generation.
class GenOptions
{
  public:
    GenOptions() :
        shaderInterfaceType(SHADER_INTERFACE_COMPLETE),
        fileTextureVerticalFlip(false),
        hwTransparency(false),
        hwSpecularEnvironmentMethod(SPECULAR_ENVIRONMENT_PREFILTER),
        hwMaxActiveLightSources(3)
    {
    }
    virtual ~GenOptions() { }

    // TODO: Add options for:
    //  - shader gen optimization level
    //  - graph flattening or not

    /// Sets the type of shader interface to be generated
    int shaderInterfaceType;

    /// If true the y-component of texture coordinates used for sampling
    /// file textures will be flipped before sampling. This can be used if
    /// file textures need to be flipped vertically to match the target's
    /// texture space convention. By default this option is false.
    bool fileTextureVerticalFlip;

    /// An optional override for the target color space.
    /// Shader fragments will be generated to transform
    /// input values and textures into this color space.
    string targetColorSpaceOverride;

    /// Sets if transparency is needed or not for HW shaders.
    /// If a surface shader has potential of being transparent
    /// this must be set to true, otherwise no transparency
    /// code fragments will be generated for the shader and
    /// the surface will be fully opaque.
    bool hwTransparency;

    /// Sets the method to use for specular environment 
    /// lighting for HW shader targets.
    int hwSpecularEnvironmentMethod;

    /// Sets the maximum number of light sources that can
    /// be active at once.
    unsigned int hwMaxActiveLightSources;
};

} // namespace MaterialX

#endif
