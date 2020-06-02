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

    // Emit directives for stage
    void emitDirectives(GenContext& context, ShaderStage& stage) override;

    // Emit blocks with resource binding information
    void emitResourceBindingBlocks(GenContext& context, const VariableBlock& uniforms, SyntaxPtr syntax, ShaderStage& stage) override
    {
        const string uniformBlockName(uniforms.getName());
        if (uniformBlockName == HW::SAMPLER_UNIFORMS)
        {
            emitSamplerBlocks(context, uniforms, syntax, stage);
        }
        else
        {
            emitUniformBlock(context, uniforms, syntax, stage);
        }
    }

    // Emits each sampler as a separate block
    void emitSamplerBlocks(GenContext& context, const VariableBlock& uniforms, SyntaxPtr syntax, ShaderStage& stage);

    // Emits all uniforms group as a block
    void emitUniformBlock(GenContext& context, const VariableBlock& uniforms, SyntaxPtr syntax, ShaderStage& stage);

protected:

    // List of required extensions
    StringSet _requiredExtensions;

    // Binding location
    int _hwBindLocation = 0;

};

} // namespace MaterialX

#endif
