#include <MaterialXShaderGen/ShaderGenerators/Glsl/BitangentGlsl.h>

namespace MaterialX
{

SgImplementationPtr BitangentGlsl::creator()
{
    return std::make_shared<BitangentGlsl>();
}

void BitangentGlsl::createVariables(const SgNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createAppData(DataType::VECTOR3, "i_bitangent");

    const SgInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;
    if (space == WORLD)
    {
        shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, DataType::MATRIX4, "u_worldInverseTransposeMatrix");
        shader.createVertexData(DataType::VECTOR3, "bitangentWorld");
    }
    else if (space == MODEL)
    {
        shader.createVertexData(DataType::VECTOR3, "bitangentModel");
    }
    else
    {
        shader.createVertexData(DataType::VECTOR3, "bitangentObject");
    }
}

void BitangentGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const string& blockInstance = shader.getVertexDataBlock().instance;
    const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;

    const SgInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (space == WORLD)
        {
            if (!shader.isCalculated("bitangentWorld"))
            {
                shader.setCalculated("bitangentWorld");
                shader.addLine(blockPrefix + "bitangentWorld = normalize(u_worldInverseTransposeMatrix * i_bitangent)");
            }
        }
        else if (space == MODEL)
        {
            if (!shader.isCalculated("bitangentModel"))
            {
                shader.setCalculated("bitangentModel");
                shader.addLine(blockPrefix + "bitangentModel = i_bitangent");
            }
        }
        else
        {
            if (!shader.isCalculated("bitangentObject"))
            {
                shader.setCalculated("bitangentObject");
                shader.addLine(blockPrefix + "bitangentObject = i_bitangent");
            }
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);
        if (space == WORLD)
        {
            shader.addStr(" = " + blockPrefix + "bitangentWorld");
        }
        else if (space == MODEL)
        {
            shader.addStr(" = " + blockPrefix + "bitangentModel");
        }
        else
        {
            shader.addStr(" = " + blockPrefix + "bitangentObject");
        }
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
