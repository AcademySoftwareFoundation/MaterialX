//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/GlslResourceBindingContext.h>

namespace MaterialX
{

//
// GlslResourceBindingContext
//
GlslResourceBindingContext::GlslResourceBindingContext()
{
    _requiredExtensions.insert("GL_ARB_shading_language_420pack");
}

void GlslResourceBindingContext::initialize()
{
    // Reset bind location counter.
    _hwBindLocation = 0;
}

void GlslResourceBindingContext::emitDirectives(GenContext& context, ShaderStage& stage)
{
    ShaderGenerator& generator = context.getShaderGenerator();

    for (auto& extension : _requiredExtensions)
    {
        generator.emitLine("#extension " + extension + " : enable", stage, false);
    }
}

void GlslResourceBindingContext::emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage)
{
    ShaderGenerator& generator = context.getShaderGenerator();
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
        generator.emitLine("layout (binding=" + std::to_string(_hwBindLocation++) + ") " + 
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
            generator.emitString("layout (binding=" + std::to_string(_hwBindLocation++) + ") " + syntax.getUniformQualifier() + " ", stage);
            generator.emitVariableDeclaration(uniform, EMPTY_STRING, context, stage, false);
            generator.emitLineEnd(stage, true);
        }
    }

    generator.emitLineBreak(stage);
}

}
