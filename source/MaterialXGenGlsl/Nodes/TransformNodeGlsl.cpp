#include <MaterialXGenGlsl/Nodes/TransformNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr TransformNodeGlsl::create()
{
    return std::make_shared<TransformNodeGlsl>();
}

void TransformNodeGlsl::createVariables(Shader& shader, GenContext&, const ShaderGenerator&, const ShaderNode& node) const
{
    const ShaderInput* toSpaceInput = node.getInput(TO_SPACE);
    string toSpace = toSpaceInput ? toSpaceInput->getValue()->getValueString() : EMPTY_STRING;

    const ShaderInput* fromSpaceInput = node.getInput(FROM_SPACE);
    string fromSpace = fromSpaceInput ? fromSpaceInput->getValue()->getValueString() : EMPTY_STRING;

    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    if ((fromSpace == MODEL || fromSpace == OBJECT) && toSpace == WORLD)
    {
        if (node.hasClassification(ShaderNode::Classification::TRANSFORM_NORMAL))
        {
            addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix");
        }
        else
        {
            addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldMatrix");
        }
    }
    else if (fromSpace == WORLD && (toSpace == MODEL || toSpace == OBJECT))
    {
        if (node.hasClassification(ShaderNode::Classification::TRANSFORM_NORMAL))
        {
            addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldTransposeMatrix");
        }
        else
        {
            addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseMatrix");
        }
    }
}

void TransformNodeGlsl::emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const
{
    const ShaderInput* inInput = node.getInput("in");
    if (inInput->getType() != Type::VECTOR3 && inInput->getType() != Type::VECTOR4)
    {
        throw ExceptionShaderGenError("Transform node must have 'in' type of vector3 or vector4.");
    }

    const ShaderInput* toSpaceInput = node.getInput(TO_SPACE);
    string toSpace = toSpaceInput ? toSpaceInput->getValue()->getValueString() : EMPTY_STRING;

    const ShaderInput* fromSpaceInput = node.getInput(FROM_SPACE);
    string fromSpace = fromSpaceInput ? fromSpaceInput->getValue()->getValueString() : EMPTY_STRING;

    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(stage, context, node.getOutput(), true, false);

        shadergen.emitString(stage, " = ");
        if (inInput->getType() == Type::VECTOR3)
        {
            shadergen.emitString(stage, "(");
        }
        if ((fromSpace == MODEL || fromSpace == OBJECT) && toSpace == WORLD)
        {
            if (node.hasClassification(ShaderNode::Classification::TRANSFORM_NORMAL))
            {
                shadergen.emitString(stage, "u_worldInverseTransposeMatrix * ");
            }
            else
            {
                shadergen.emitString(stage, "u_worldMatrix * ");
            }
        }
        else if (fromSpace == WORLD && (toSpace == MODEL || toSpace == OBJECT))
        {
            if (node.hasClassification(ShaderNode::Classification::TRANSFORM_NORMAL))
            {
                shadergen.emitString(stage, "u_worldTransposeMatrix * ");
            }
            else
            {
                shadergen.emitString(stage, "u_worldInverseMatrix * ");
            }
        }

        if (inInput->getType() == Type::VECTOR3)
        {
            shadergen.emitString(stage, "vec4(");
            if (node.hasClassification(ShaderNode::Classification::TRANSFORM_POINT))
            {
                shadergen.emitInput(stage, context, inInput);
                shadergen.emitString(stage, ", 1.0)");
            }
            else
            {
                shadergen.emitInput(stage, context, inInput);
                shadergen.emitString(stage, ", 0.0)");
            }
            shadergen.emitString(stage, ").xyz");
        }
        else
        {
            shadergen.emitInput(stage, context, inInput);
        }
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(stage, HW::PIXEL_STAGE)
}

} // namespace MaterialX
