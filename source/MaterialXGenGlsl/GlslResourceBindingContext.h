//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GLSLRESOURCEBINDING_H
#define MATERIALX_GLSLRESOURCEBINDING_H

/// @file
/// GLSL resource binding context

#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

/// Shared pointer to a GlslResourceBindingContext
using GlslResourceBindingContextPtr = shared_ptr<class GlslResourceBindingContext>;

/// @class GlslResourceBindingContext
/// Class representing a resource binding for Glsl shader resources.
class GlslResourceBindingContext : public HwResourceBindingContext
{
public:

    GlslResourceBindingContext(size_t uniformBindingLocation, size_t samplerBindingLocation);

    static GlslResourceBindingContextPtr create(
        size_t uniformBindingLocation=0, size_t samplerBindingLocation=0)
    {
        return std::make_shared<GlslResourceBindingContext>(
            uniformBindingLocation, samplerBindingLocation);
    }

    // Initialize the context before generation starts.
    void initialize() override;

    // Emit directives for stage
    void emitDirectives(GenContext& context, ShaderStage& stage) override;

    // Emit uniforms with binding information
    void emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage) override;

    // Emit Structured uniforms with binding information and align members where possible
    void emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
        ShaderStage& stage, const std::string& structInstanceName,
        const std::string& arraySuffix) override;

protected:
    // List of required extensions
    StringSet _requiredExtensions;

    // Binding location for Uniform Blocks
    size_t _hwUniformBindLocation = 0;

    // Initial value of uniform binding location
    size_t _hwInitUniformBindLocation = 0;

    // Binding location for Sampler Blocks
    size_t _hwSamplerBindLocation = 0;

    // Initial value of sampler binding location
    size_t _hwInitSamplerBindLocation = 0;

};

} // namespace MaterialX

#endif
