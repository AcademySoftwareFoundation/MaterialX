//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/Nodes/HwTransformNode.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

string HwTransformNode::MODEL = "model";
string HwTransformNode::OBJECT = "object";
string HwTransformNode::WORLD = "world";

static inline string getSpaceName(const ShaderInput* spaceInput)
{
    return spaceInput && spaceInput->getValue() ? spaceInput->getValue()->getValueString() : EMPTY_STRING;
}

void HwTransformNode::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    string toSpace   = getSpaceName(getToSpaceInput(node));
    string fromSpace = getSpaceName(getFromSpaceInput(node));

    const string& matrix = getMatrix(fromSpace, toSpace);
    if (!matrix.empty())
    {
        ShaderStage& ps = shader.getStage(Stage::PIXEL);
        addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, matrix, ps);
    }
}

void HwTransformNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        const ShaderInput* inInput = node.getInput("in");
        if (inInput->getType() != Type::VECTOR3 && inInput->getType() != Type::VECTOR4)
        {
            throw ExceptionShaderGenError("Transform node must have 'in' type of vector3 or vector4.");
        }

        string toSpace   = getSpaceName(getToSpaceInput(node));
        string fromSpace = getSpaceName(getFromSpaceInput(node));
        const ShaderOutput* output = node.getOutput();

        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(output, true, false, context, stage);
        shadergen.emitString(" = (", stage);
        const string& matrix = getMatrix(fromSpace, toSpace);
        if (!matrix.empty())
        {
            shadergen.emitString(matrix + " * ", stage);
        }
        const string type = shadergen.getSyntax().getTypeName(Type::VECTOR4);
        const string input = shadergen.getUpstreamResult(inInput, context);
        shadergen.emitString(type + "(" + input + ", " + getHomogeneousCoordinate() + ")).xyz", stage);
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

const ShaderInput* HwTransformNode::getFromSpaceInput(const ShaderNode& node) const
{
    return node.getInput("fromspace");
}

const ShaderInput* HwTransformNode::getToSpaceInput(const ShaderNode& node) const
{
    return node.getInput("tospace");
}

ShaderNodeImplPtr HwTransformVectorNode::create()
{
    return std::make_shared<HwTransformVectorNode>();
}

const string& HwTransformVectorNode::getMatrix(const string& fromSpace, const string& toSpace) const
{
    if ((fromSpace == MODEL || fromSpace == OBJECT) && toSpace == WORLD)
    {
        return HW::T_WORLD_MATRIX;
    }
    else if (fromSpace == WORLD && (toSpace == MODEL || toSpace == OBJECT))
    {
        return HW::T_WORLD_INVERSE_MATRIX;
    }
    return EMPTY_STRING;
}

string HwTransformVectorNode::getHomogeneousCoordinate() const
{
    return "0.0";
}

ShaderNodeImplPtr HwTransformPointNode::create()
{
    return std::make_shared<HwTransformPointNode>();
}

string HwTransformPointNode::getHomogeneousCoordinate() const
{
    return "1.0";
}

ShaderNodeImplPtr HwTransformNormalNode::create()
{
    return std::make_shared<HwTransformNormalNode>();
}

const string& HwTransformNormalNode::getMatrix(const string& fromSpace, const string& toSpace) const
{
    if ((fromSpace == MODEL || fromSpace == OBJECT) && toSpace == WORLD)
    {
        return HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX;
    }
    else if (fromSpace == WORLD && (toSpace == MODEL || toSpace == OBJECT))
    {
        return HW::T_WORLD_TRANSPOSE_MATRIX;
    }
    return EMPTY_STRING;
}

string HwTransformNormalNode::getHomogeneousCoordinate() const
{
    return "0.0";
}

MATERIALX_NAMESPACE_END
