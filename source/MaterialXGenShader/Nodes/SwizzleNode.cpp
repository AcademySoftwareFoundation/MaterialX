//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

static const string IN_STRING("in");
static const string CHANNELS_STRING("channels");

ShaderNodeImplPtr SwizzleNode::create()
{
    return std::make_shared<SwizzleNode>();
}

void SwizzleNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        const ShaderInput* in = node.getInput(IN_STRING);
        const ShaderInput* channels = node.getInput(CHANNELS_STRING);
        if (!in || !channels)
        {
            throw ExceptionShaderGenError("Node '" + node.getName() +"' is not a valid swizzle node");
        }
        if (!in->getConnection() && !in->getValue())
        {
            throw ExceptionShaderGenError("No connection or value found to swizzle on node '" + node.getName() + "'");
        }

        const string& swizzle = channels->getValue() ? channels->getValue()->getValueString() : EMPTY_STRING;
        string variableName = in->getConnection() ? in->getConnection()->getVariable() : in->getVariable();

        // If the input is unconnected we must declare a variable
        // for it first, in order to swizzle it below.
        if (!in->getConnection())
        {
            string variableValue = in->getValue() ? shadergen.getSyntax().getValue(in->getType(), *in->getValue()) : shadergen.getSyntax().getDefaultValue(in->getType());
            shadergen.emitLine(shadergen.getSyntax().getTypeName(in->getType()) + " " + variableName + " = " + variableValue, stage);
        }

        if (!swizzle.empty())
        {
            const TypeDesc* type = in->getConnection() ? in->getConnection()->getType() : in->getType();
            variableName = shadergen.getSyntax().getSwizzledVariable(variableName, type, swizzle, node.getOutput()->getType());
        }

        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + variableName, stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

bool SwizzleNode::isEditable(const ShaderInput& input) const
{
    return (input.getName() != CHANNELS_STRING);
}

MATERIALX_NAMESPACE_END
