//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/WgslResourceBindingContext.h>

MATERIALX_NAMESPACE_BEGIN

//
// WgslResourceBindingContext methods
//

WgslResourceBindingContext::WgslResourceBindingContext(size_t uniformBindingLocation) :
    VkResourceBindingContext(uniformBindingLocation)
{
}

// Copied from VkResourceBindingContext::emitResourceBindings().  
// Modified the Type::FILENAME uniform codegen.
void WgslResourceBindingContext::emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage)
{
    const ShaderGenerator& generator = context.getShaderGenerator();
    const Syntax& syntax = generator.getSyntax();

    // First, emit all value uniforms in a block with single layout binding
    bool hasValueUniforms = false;
    for (auto uniform : uniforms.getVariableOrder())
    {
        if (uniform->getType() != Type::FILENAME)
        {
            hasValueUniforms = true;
            break;
        }
    }
    if (hasValueUniforms)
    {
        generator.emitLine("layout (std140, binding=" + std::to_string(_hwUniformBindLocation++) + ") " +
                               syntax.getUniformQualifier() + " " + uniforms.getName() + "_" + stage.getName(),
                           stage, false);
        generator.emitScopeBegin(stage);
        for (auto uniform : uniforms.getVariableOrder())
        {
            if (uniform->getType() != Type::FILENAME)
            {
                generator.emitLineBegin(stage);
                generator.emitVariableDeclaration(uniform, EMPTY_STRING, context, stage, false);
                generator.emitString(Syntax::SEMICOLON, stage);
                generator.emitLineEnd(stage, false);
            }
        }
        generator.emitScopeEnd(stage, true);
    }

    // Second, emit all sampler uniforms as separate uniforms with separate layout bindings
    for (auto uniform : uniforms.getVariableOrder())
    {
        if (uniform->getType() == Type::FILENAME)
        {
            // Bind separately as texture2D + sampler
            //
            // NOTE: the *_texture and *_sampler binding names method below expect that
            //       variables from HwShaderGenerator.cpp (HW::ENV_RADIANCE_SPLIT and HW::ENV_IRRADIANCE_SPLIT)
            //       use the same naming convention as here.
            //
            generator.emitString("layout (binding=" + std::to_string(_hwUniformBindLocation++) + ") " + syntax.getUniformQualifier() + " ", stage);
            generator.emitString(string("texture2D ")+uniform->getVariable()+"_texture", stage);
            generator.emitLineEnd(stage, true);

            generator.emitString("layout (binding=" + std::to_string(_hwUniformBindLocation++) + ") " + syntax.getUniformQualifier() + " ", stage);
            generator.emitString(string("sampler ")+uniform->getVariable()+"_sampler", stage);
            generator.emitLineEnd(stage, true);
        }
    }

    generator.emitLineBreak(stage);
}

MATERIALX_NAMESPACE_END
