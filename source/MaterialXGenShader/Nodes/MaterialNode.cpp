//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/MaterialNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/GenContext.h>

#include <iostream>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr MaterialNode::create()
{
    return std::make_shared<MaterialNode>();
}

void MaterialNode::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
BEGIN_SHADER_STAGE(stage, Stage::PIXEL)

    ShaderNode& node = const_cast<ShaderNode&>(_node);
    ShaderInput* surfaceshaderInput = node.getInput(ShaderNode::SURFACESHADER);

    if (!surfaceshaderInput->getConnection())
    {
        // Just declare the output variable with default value.
        emitOutputVariables(node, context, stage);
        return;
    }

    const ShaderGenerator& shadergen = context.getShaderGenerator();
    const Syntax& syntax = shadergen.getSyntax();

    // Emit the function call for upstream surface shader.
    const ShaderNode* surfaceshaderNode = surfaceshaderInput->getConnection()->getNode();
    shadergen.emitFunctionCall(*surfaceshaderNode, context, stage);

    // Assing this result to the material output variable.
    const ShaderOutput* output = node.getOutput();
    shadergen.emitLine(syntax.getTypeName(output->getType()) + " " + output->getVariable() + " = " + surfaceshaderInput->getConnection()->getVariable(), stage);

END_SHADER_STAGE(stage, Stage::PIXEL)
}

MATERIALX_NAMESPACE_END
