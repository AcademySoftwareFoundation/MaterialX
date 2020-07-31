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

    GlslResourceBindingContext();

    static GlslResourceBindingContextPtr create() { return std::make_shared<GlslResourceBindingContext>(); }

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

    // Binding location
    size_t _hwBindLocation = 0;
};

} // namespace MaterialX

#endif
