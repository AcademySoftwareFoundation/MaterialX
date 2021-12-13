//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/TransformVectorNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr TransformVectorNodeGlsl::create()
{
    return std::make_shared<TransformVectorNodeGlsl>();
}

void TransformVectorNodeGlsl::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    const ShaderInput* toSpaceInput = node.getInput(TO_SPACE);
    string toSpace = toSpaceInput ? toSpaceInput->getValue()->getValueString() : EMPTY_STRING;

    const ShaderInput* fromSpaceInput = node.getInput(FROM_SPACE);
    string fromSpace = fromSpaceInput ? fromSpaceInput->getValue()->getValueString() : EMPTY_STRING;

    const string& matrix = getMatrix(fromSpace, toSpace);
    if (!matrix.empty())
    {
        ShaderStage& ps = shader.getStage(Stage::PIXEL);
        addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, matrix, ps);
    }
}

void TransformVectorNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
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
        shadergen.emitString(" = (", stage);
        const string& matrix = getMatrix(fromSpace, toSpace);
        if (!matrix.empty())
        {
            shadergen.emitString(matrix + " * ", stage);
        }
        shadergen.emitString(getHomogeneousCoordinate(inInput, context), stage);
        shadergen.emitString(").xyz", stage);
        shadergen.emitLineEnd(stage);

    END_SHADER_STAGE(stage, Stage::PIXEL)
}

const string& TransformVectorNodeGlsl::getMatrix(const string& fromSpace, const string& toSpace) const
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

string TransformVectorNodeGlsl::getHomogeneousCoordinate(const ShaderInput* in, GenContext& context) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    return "vec4(" + shadergen.getUpstreamResult(in, context) + ", 0.0)";
}

MATERIALX_NAMESPACE_END
