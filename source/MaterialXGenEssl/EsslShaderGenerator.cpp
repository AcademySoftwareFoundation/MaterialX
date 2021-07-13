//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenEssl/EsslShaderGenerator.h>
#include <MaterialXGenEssl/EsslSyntax.h>

#include <MaterialXGenShader/Nodes/HwImageNode.h>

namespace MaterialX
{

const string EsslShaderGenerator::TARGET = "essl";
const string EsslShaderGenerator::VERSION = "300 es"; // Target WebGL 2.0

EsslShaderGenerator::EsslShaderGenerator()
    : GlslShaderGenerator()
{
    _syntax = EsslSyntax::create();
    // ESSL specific keywords
    const StringSet reservedWords = { "precision", "highp", "mediump", "lowp" };
    _syntax->registerReservedWords(reservedWords);

    // Temporary overwrites for three.js
    _tokenSubstitutions[HW::T_IN_POSITION] = "position";
    _tokenSubstitutions[HW::T_IN_NORMAL] = "normal";
    _tokenSubstitutions[HW::T_IN_TEXCOORD] = "uv";
    _tokenSubstitutions[HW::T_IN_TANGENT] = "tangent";
}

void EsslShaderGenerator::emitDirectives(GenContext&, ShaderStage& stage) const
{
    emitLine("#version " + getVersion(), stage, false);
    emitLineBreak(stage);
    emitLine("precision mediump float", stage);
}

void EsslShaderGenerator::emitUniforms(GenContext& context, ShaderStage& stage, HwResourceBindingContextPtr&) const
{
    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;

        // Skip light uniforms as they are handled separately
        if (!uniforms.empty() && uniforms.getName() != HW::LIGHT_DATA)
        {
            emitComment("Uniform block: " + uniforms.getName(), stage);
            emitVariableDeclarations(uniforms, _syntax->getUniformQualifier(), Syntax::SEMICOLON, context, stage, false);
            emitLineBreak(stage);
        }
    }
}

void EsslShaderGenerator::emitInputs(GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
    const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
    if (!vertexInputs.empty())
    {
        emitComment("Inputs block: " + vertexInputs.getName(), stage);
        emitVariableDeclarations(vertexInputs, _syntax->getInputQualifier(), Syntax::SEMICOLON, context, stage, false);
        emitLineBreak(stage);
    }
END_SHADER_STAGE(stage, Stage::VERTEX)

BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    if (!vertexData.empty())
    {
        emitVariableDeclarations(vertexData, _syntax->getInputQualifier(), Syntax::SEMICOLON, context, stage, false);
        emitLineBreak(stage);
    }
END_SHADER_STAGE(stage, Stage::PIXEL)
}

void EsslShaderGenerator::emitOutputs(GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
    const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    if (!vertexData.empty())
    {
        emitVariableDeclarations(vertexData,  _syntax->getOutputQualifier(), Syntax::SEMICOLON, context, stage, false);
        emitLineBreak(stage);
    }
END_SHADER_STAGE(stage, Stage::VERTEX)

BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    emitComment("Pixel shader outputs", stage);
    const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
    emitVariableDeclarations(outputs, _syntax->getOutputQualifier(), Syntax::SEMICOLON, context, stage, false);
    emitLineBreak(stage);
END_SHADER_STAGE(stage, Stage::PIXEL)
}

const string EsslShaderGenerator::getVertexDataPrefix(const VariableBlock&) const
{
    return "";
}

const HwResourceBindingContextPtr EsslShaderGenerator::getResourceBindingContext(GenContext& context) const
{
    HwResourceBindingContextPtr resoureBindingCtx = GlslShaderGenerator::getResourceBindingContext(context);
    if (resoureBindingCtx) {
      throw ExceptionShaderGenError("The EsslShaderGenerator does not support resource binding.");
    }
    return resoureBindingCtx;
}

}
