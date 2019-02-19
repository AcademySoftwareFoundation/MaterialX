#include <MaterialXGenGlsl/Nodes/TransformNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr TransformNodeGlsl::create()
{
    return std::make_shared<TransformNodeGlsl>();
}

void TransformNodeGlsl::createVariables(const ShaderNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const ShaderInput* toSpaceInput = node.getInput(TO_SPACE);
    string toSpace = toSpaceInput ? toSpaceInput->value->getValueString() : EMPTY_STRING;

    const ShaderInput* fromSpaceInput = node.getInput(FROM_SPACE);
    string fromSpace = fromSpaceInput ? fromSpaceInput->value->getValueString() : EMPTY_STRING;

    if ((fromSpace == MODEL || fromSpace == OBJECT) && toSpace == WORLD)
    {
        if (node.hasClassification(ShaderNode::Classification::TRANSFORM_NORMAL))
        {
            shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix");
        }
        else
        {
            shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldMatrix");
        }
    }
    else if (fromSpace == WORLD && (toSpace == MODEL || toSpace == OBJECT))
    {
        if (node.hasClassification(ShaderNode::Classification::TRANSFORM_NORMAL))
        {
            shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldTransposeMatrix");
        }
        else
        {
            shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseMatrix");
        }
    }
}

void TransformNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const ShaderInput* inInput = node.getInput("in");
    if (inInput->type != Type::VECTOR3 && inInput->type != Type::VECTOR4)
    {
        throw ExceptionShaderGenError("Transform node must have 'in' type of vector3 or vector4.");
    }

    const ShaderInput* toSpaceInput = node.getInput(TO_SPACE);
    string toSpace = toSpaceInput ? toSpaceInput->value->getValueString() : EMPTY_STRING;

    const ShaderInput* fromSpaceInput = node.getInput(FROM_SPACE);
    string fromSpace = fromSpaceInput ? fromSpaceInput->value->getValueString() : EMPTY_STRING;

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);

        shader.addStr( " = ");
        if (inInput->type == Type::VECTOR3)
        {
            shader.addStr("(");
        }
        if ((fromSpace == MODEL || fromSpace == OBJECT) && toSpace == WORLD)
        {
            if (node.hasClassification(ShaderNode::Classification::TRANSFORM_NORMAL))
            {
                shader.addStr("u_worldInverseTransposeMatrix * ");
            }
            else
            {
                shader.addStr("u_worldMatrix * ");
            }
        }
        else if (fromSpace == WORLD && (toSpace == MODEL || toSpace == OBJECT))
        {
            if (node.hasClassification(ShaderNode::Classification::TRANSFORM_NORMAL))
            {
                shader.addStr("u_worldTransposeMatrix * ");
            }
            else
            {
                shader.addStr("u_worldInverseMatrix * ");
            }
        }

        if (inInput->type == Type::VECTOR3)
        {
            shader.addStr("vec4(");
            if (node.hasClassification(ShaderNode::Classification::TRANSFORM_POINT))
            {
                shadergen.emitInput(context, inInput, shader);
                shader.addStr(", 1.0)");
            }
            else
            {
                shadergen.emitInput(context, inInput, shader);
                shader.addStr(", 0.0)");
            }
            shader.addStr(").xyz");
        }
        else
        {
            shadergen.emitInput(context, inInput, shader);
        }
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
