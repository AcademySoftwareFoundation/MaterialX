#include <MaterialXShaderGen/ShaderGenerators/Glsl/NormalGlsl.h>

namespace MaterialX
{

SgImplementationPtr NormalGlsl::creator()
{
    return std::make_shared<NormalGlsl>();
}

void NormalGlsl::createVariables(const SgNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createAppData(DataType::VECTOR3, "i_normal");

    const SgInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;
    if (space == WORLD)
    {
        shader.createUniform(HwShader::GLOBAL_SCOPE, DataType::MATRIX4, "u_normalMatrix");
        shader.createVertexData(DataType::VECTOR3, "normalWorld");
    }
    else if (space == MODEL)
    {
        shader.createVertexData(DataType::VECTOR3, "normalModel");
    }
    else
    {
        shader.createVertexData(DataType::VECTOR3, "normalObject");
    }
}

void NormalGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const SgInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (space == WORLD)
        {
            if (!shader.isCalculated("normalWorld"))
            {
                shader.setCalculated("normalWorld");
                shader.addLine("vd.normalWorld = normalize((u_normalMatrix * vec4(i_normal, 0)).xyz)");
            }
        }
        else if (space == MODEL)
        {
            if (!shader.isCalculated("normalModel"))
            {
                shader.setCalculated("normalModel");
                shader.addLine("vd.normalModel = i_normal");
            }
        }
        else
        {
            if (!shader.isCalculated("normalObject"))
            {
                shader.setCalculated("normalObject");
                shader.addLine("vd.normalObject = i_normal");
            }
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);
        if (space == WORLD)
        {
            shader.addStr(" = vd.normalWorld");
        }
        else if (space == MODEL)
        {
            shader.addStr(" = vd.normalModel");
        }
        else
        {
            shader.addStr(" = vd.normalObject");
        }

        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
