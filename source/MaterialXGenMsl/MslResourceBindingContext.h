//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_MSLRESOURCEBINDING_H
#define MATERIALX_MSLRESOURCEBINDING_H

/// @file
/// MSL resource binding context

#include <MaterialXGenMsl/Export.h>

#include <MaterialXGenShader/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a MslResourceBindingContext
using MslResourceBindingContextPtr = shared_ptr<class MslResourceBindingContext>;

/// @class MslResourceBindingContext
/// Class representing a resource binding for Msl shader resources.
class MX_GENMSL_API MslResourceBindingContext : public HwResourceBindingContext
{
  public:
    MslResourceBindingContext(size_t uniformBindingLocation, size_t samplerBindingLocation);

    static MslResourceBindingContextPtr create(
        size_t uniformBindingLocation = 0, size_t samplerBindingLocation = 0)
    {
        return std::make_shared<MslResourceBindingContext>(
            uniformBindingLocation, samplerBindingLocation);
    }

    // Initialize the context before generation starts.
    void initialize() override;

    // Emit directives for stage
    void emitDirectives(GenContext& context, ShaderStage& stage) override;

    // Emit uniforms with binding information
    void emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage) override;

    // Emit structured uniforms with binding information and align members where possible
    void emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
                                        ShaderStage& stage, const std::string& structInstanceName,
                                        const std::string& arraySuffix) override;

    // Emit separate binding locations for sampler and uniform table
    void enableSeparateBindingLocations(bool separateBindingLocation) { _separateBindingLocation = separateBindingLocation; };

  protected:
    // Binding location for uniform blocks
    size_t _hwUniformBindLocation = 0;

    // Initial value of uniform binding location
    size_t _hwInitUniformBindLocation = 0;

    // Binding location for sampler blocks
    size_t _hwSamplerBindLocation = 0;

    // Initial value of sampler binding location
    size_t _hwInitSamplerBindLocation = 0;

    // Separate binding locations flag
    // Indicates whether to use a shared binding counter for samplers and uniforms or separate ones.
    // By default a shader counter is used.
    bool _separateBindingLocation = false;
};

MATERIALX_NAMESPACE_END

#endif
