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

void GlslResourceBindingContext::emitDirectives(GenContext& context, ShaderStage& stage)
{
    ShaderGenerator& generator = context.getShaderGenerator();

    for (auto& extension : _requiredExtensions)
    {
        generator.emitLine("#extension " + extension + " : enable", stage, false);
    }
}

void GlslResourceBindingContext::emitUniformBlock(GenContext& context, const VariableBlock& uniforms, SyntaxPtr syntax, ShaderStage& stage)
{
    ShaderGenerator& generator = context.getShaderGenerator();

    generator.emitString("layout (binding=" + std::to_string(_hwBindLocation) + ") " + syntax->getUniformQualifier() + " " + uniforms.getName(), stage);
    generator.emitScopeBegin(stage);
    generator.emitVariableDeclarations(uniforms, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
    generator.emitScopeEnd(stage, true);
    generator.emitLineBreak(stage);
    _hwBindLocation++;
}

void GlslResourceBindingContext::emitSamplerBlocks(GenContext& context, const VariableBlock& uniforms, SyntaxPtr syntax, ShaderStage& stage)
{
    ShaderGenerator& generator = context.getShaderGenerator();

    for (size_t i = 0; i < uniforms.size(); ++i)
    {
        generator.emitString("layout (binding=" + std::to_string(_hwBindLocation) + ") " + syntax->getUniformQualifier() + " ", stage);
        generator.emitVariableDeclaration(uniforms[i], EMPTY_STRING, context, stage, false);
        generator.emitLineEnd(stage, true);
        _hwBindLocation++;
    }
}


}
