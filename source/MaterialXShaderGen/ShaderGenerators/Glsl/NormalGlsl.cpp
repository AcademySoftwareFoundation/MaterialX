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
        shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, DataType::MATRIX4, "u_worldInverseTranspose");
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

    const string& blockInstance = shader.getVertexDataBlock().instance;
    const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;

    const SgInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (space == WORLD)
        {
            if (!shader.isCalculated("normalWorld"))
            {
                shader.setCalculated("normalWorld");
                shader.addLine(blockPrefix + "normalWorld = normalize((u_worldInverseTranspose * vec4(i_normal, 0)).xyz)");
            }
        }
        else if (space == MODEL)
        {
            if (!shader.isCalculated("normalModel"))
            {
                shader.setCalculated("normalModel");
                shader.addLine(blockPrefix + "normalModel = i_normal");
            }
        }
        else
        {
            if (!shader.isCalculated("normalObject"))
            {
                shader.setCalculated("normalObject");
                shader.addLine(blockPrefix + "normalObject = i_normal");
            }
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);
        if (space == WORLD)
        {
            shader.addStr(" = " + blockPrefix + "normalWorld");
        }
        else if (space == MODEL)
        {
            shader.addStr(" = " + blockPrefix + "normalModel");
        }
        else
        {
            shader.addStr(" = " + blockPrefix + "normalObject");
        }

        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
