//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/TransformNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

ShaderNodeImplPtr TransformNodeGlsl::create()
{
    return std::make_shared<TransformNodeGlsl>();
}

void TransformNodeGlsl::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    const ShaderInput* toSpaceInput = node.getInput(TO_SPACE);
    string toSpace = toSpaceInput ? toSpaceInput->getValue()->getValueString() : EMPTY_STRING;

    const ShaderInput* fromSpaceInput = node.getInput(FROM_SPACE);
    string fromSpace = fromSpaceInput ? fromSpaceInput->getValue()->getValueString() : EMPTY_STRING;

    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    if ((fromSpace == MODEL || fromSpace == OBJECT) && toSpace == WORLD)
    {
        if (node.hasClassification(ShaderNode::Classification::TRANSFORM_NORMAL))
        {
            addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix", ps);
        }
        else
        {
            addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldMatrix", ps);
        }
    }
    else if (fromSpace == WORLD && (toSpace == MODEL || toSpace == OBJECT))
    {
        if (node.hasClassification(ShaderNode::Classification::TRANSFORM_NORMAL))
        {
            addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldTransposeMatrix", ps);
        }
        else
        {
            addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseMatrix", ps);
        }
    }
}

void TransformNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        const ShaderInput* inInput = node.getInput("in");
        if (inInput->getType() != Type::VECTOR3 && inInput->getType() != Type::VECTOR4)
        {
            throw ExceptionShaderGenError("Transform node must have 'in' type of vector3 or vector4.");
        }

        const ShaderInput* toSpaceInput = node.getInput(TO_SPACE);
        string toSpace = toSpaceInput ? toSpaceInput->getValue()->getValueString() : EMPTY_STRING;

        const ShaderInput* fromSpaceInput = node.getInput(FROM_SPACE);
        string fromSpace = fromSpaceInput ? fromSpaceInput->getValue()->getValueString() : EMPTY_STRING;

        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);

        shadergen.emitString(" = ", stage);
        if (inInput->getType() == Type::VECTOR3)
        {
            shadergen.emitString("(", stage);
        }
        if ((fromSpace == MODEL || fromSpace == OBJECT) && toSpace == WORLD)
        {
            if (node.hasClassification(ShaderNode::Classification::TRANSFORM_NORMAL))
            {
                shadergen.emitString("u_worldInverseTransposeMatrix * ", stage);
            }
            else
            {
                shadergen.emitString("u_worldMatrix * ", stage);
            }
        }
        else if (fromSpace == WORLD && (toSpace == MODEL || toSpace == OBJECT))
        {
            if (node.hasClassification(ShaderNode::Classification::TRANSFORM_NORMAL))
            {
                shadergen.emitString("u_worldTransposeMatrix * ", stage);
            }
            else
            {
                shadergen.emitString("u_worldInverseMatrix * ", stage);
            }
        }

        if (inInput->getType() == Type::VECTOR3)
        {
            shadergen.emitString("vec4(", stage);
            if (node.hasClassification(ShaderNode::Classification::TRANSFORM_POINT))
            {
                shadergen.emitInput(inInput, context, stage);
                shadergen.emitString(", 1.0)", stage);
            }
            else
            {
                shadergen.emitInput(inInput, context, stage);
                shadergen.emitString(", 0.0)", stage);
            }
            shadergen.emitString(").xyz", stage);
        }
        else
        {
            shadergen.emitInput(inInput, context, stage);
        }
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
