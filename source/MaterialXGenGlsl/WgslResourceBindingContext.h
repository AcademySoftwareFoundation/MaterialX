//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLRESOURCEBINDING_H
#define MATERIALX_WGSLRESOURCEBINDING_H

/// @file
/// Vulkan GLSL resource binding context for WGSL

#include <MaterialXGenGlsl/Export.h>

#include <MaterialXGenGlsl/VkShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a WgslResourceBindingContext
using WgslResourceBindingContextPtr = shared_ptr<class WgslResourceBindingContext>;

/// @class WgslResourceBindingContext
/// Class representing a resource binding for Vulkan Glsl shader resources.
class MX_GENGLSL_API WgslResourceBindingContext : public VkResourceBindingContext
{
  public:
    WgslResourceBindingContext(size_t uniformBindingLocation);

    static WgslResourceBindingContextPtr create(size_t uniformBindingLocation = 0)
    {
        return std::make_shared<WgslResourceBindingContext>(uniformBindingLocation);
    }

    // Emit uniforms with binding information
    void emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage) override;
};

MATERIALX_NAMESPACE_END

#endif
