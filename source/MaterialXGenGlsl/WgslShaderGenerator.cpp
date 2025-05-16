//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/WgslShaderGenerator.h>
#include <MaterialXGenGlsl/WgslSyntax.h>

MATERIALX_NAMESPACE_BEGIN

//const string WgslShaderGenerator::TARGET = "genglsl";
//const string WgslShaderGenerator::VERSION = "450";

WgslShaderGenerator::WgslShaderGenerator(TypeSystemPtr typeSystem) :
    VkShaderGenerator(typeSystem)
{
    _syntax = WgslSyntax::create(typeSystem);
    // Add in WGSL specific keywords
    const StringSet reservedWords = { "type" };
    _syntax->registerReservedWords(reservedWords);

    // Set binding context to handle resource binding layouts
    _resourceBindingCtx = std::make_shared<MaterialX::WgslResourceBindingContext>(0);
}

// void WgslShaderGenerator::emitDirectives(GenContext&, ShaderStage& stage) const
// {
//     emitLine("#version " + getVersion(), stage, false);
//     emitLineBreak(stage);
// }

// void WgslShaderGenerator::emitInputs(GenContext& context, ShaderStage& stage) const
// {
//     DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
//     {
//         const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
//         if (!vertexInputs.empty())
//         {
//             emitComment("Inputs block: " + vertexInputs.getName(), stage);
//             for (size_t i = 0; i < vertexInputs.size(); ++i)
//             {
//                 emitLineBegin(stage);
//                 emitString("layout (location = " + std::to_string(i) + ") ", stage);
//                 emitVariableDeclaration(vertexInputs[i], _syntax->getInputQualifier(), context, stage, false);
//                 emitString(Syntax::SEMICOLON, stage);
//                 emitLineEnd(stage, false);
//             }
//             emitLineBreak(stage);
//         }
//     }

//     DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
//     {
//         const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
//         if (!vertexData.empty())
//         {
//             emitString("layout (location = " + std::to_string(vertexDataLocation) + ") " +
//                         _syntax->getInputQualifier() + " " + vertexData.getName(), stage);
//             emitLineBreak(stage);
//             emitScopeBegin(stage);
//             emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
//             emitScopeEnd(stage, false, false);
//             emitString(" " + vertexData.getInstance() + Syntax::SEMICOLON, stage);
//             emitLineBreak(stage);
//             emitLineBreak(stage);
//         }
//     }
// }

// string WgslShaderGenerator::getVertexDataPrefix(const VariableBlock& vertexData) const
// {
//     return vertexData.getInstance() + ".";
// }

// void WgslShaderGenerator::emitOutputs(GenContext& context, ShaderStage& stage) const
// {
//     DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
//     {
//         const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
//         if (!vertexData.empty())
//         {
//             emitString("layout (location = " + std::to_string(vertexDataLocation) + ") " +
//                         _syntax->getOutputQualifier() + " " + vertexData.getName(), stage);
//             emitLineBreak(stage);
//             emitScopeBegin(stage);
//             emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
//             emitScopeEnd(stage, false, false);
//             emitString(" " + vertexData.getInstance() + Syntax::SEMICOLON, stage);
//             emitLineBreak(stage);
//             emitLineBreak(stage);
//         }
//     }

//     DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
//     {
//         const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);

//         emitComment("Pixel shader outputs", stage);
//         for (size_t i = 0; i < outputs.size(); ++i)
//         {
//             emitLineBegin(stage);
//             emitString("layout (location = " + std::to_string(i) + ") ", stage);
//             emitVariableDeclaration(outputs[i], _syntax->getOutputQualifier(), context, stage, false);
//             emitString(Syntax::SEMICOLON, stage);
//             emitLineEnd(stage, false);
//         }
//         emitLineBreak(stage);
//     }
// }

HwResourceBindingContextPtr WgslShaderGenerator::getResourceBindingContext(GenContext& /*context*/) const
{
    return _resourceBindingCtx;
}

MATERIALX_NAMESPACE_END
