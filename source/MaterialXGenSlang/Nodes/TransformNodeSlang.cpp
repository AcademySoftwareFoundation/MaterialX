//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenSlang/Nodes/TransformNodeSlang.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

void TransformNodeSlang::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        const ShaderOutput* output = node.getOutput();
        const ShaderInput* inInput = node.getInput("in");
        if (inInput->getType() != Type::VECTOR3 && inInput->getType() != Type::VECTOR4)
        {
            throw ExceptionShaderGenError("Transform node must have 'in' type of vector3 or vector4.");
        }

        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(output, true, false, context, stage);

        const string toSpace = getToSpace(node);
        const string fromSpace = getFromSpace(node);
        const string& matrix = getMatrix(fromSpace, toSpace);
        if (!matrix.empty())
        {
            shadergen.emitString("= mul(", stage);
        }
        else
        {
            shadergen.emitString(" = (", stage);
        }

        const string type = shadergen.getSyntax().getTypeName(Type::VECTOR4);
        const string input = shadergen.getUpstreamResult(inInput, context);
        shadergen.emitString(type + "(" + input + ", " + getHomogeneousCoordinate() + ")", stage);
        if (!matrix.empty())
        {
            shadergen.emitString(", " + matrix, stage);
        }
        shadergen.emitString(").xyz", stage);


        shadergen.emitLineEnd(stage);

        if (shouldNormalize())
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(output, false, false, context, stage);
            shadergen.emitString(" = normalize(" + output->getVariable() + ")", stage);
            shadergen.emitLineEnd(stage);
        }
    }
}

ShaderNodeImplPtr TransformVectorNodeSlang::create()
{
    return std::make_shared<TransformVectorNodeSlang>();
}

ShaderNodeImplPtr TransformPointNodeSlang::create()
{
    return std::make_shared<TransformPointNodeSlang>();
}

ShaderNodeImplPtr TransformNormalNodeSlang::create()
{
    return std::make_shared<TransformNormalNodeSlang>();
}

MATERIALX_NAMESPACE_END
