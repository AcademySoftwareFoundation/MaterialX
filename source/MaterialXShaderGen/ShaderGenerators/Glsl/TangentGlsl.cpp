#include <MaterialXShaderGen/ShaderGenerators/Glsl/TangentGlsl.h>

namespace MaterialX
{

SgImplementationPtr TangentGlsl::creator()
{
    return std::make_shared<TangentGlsl>();
}

void TangentGlsl::createVariables(const SgNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createAppData(DataType::VECTOR3, "i_tangent");

    const SgInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;
    if (space == WORLD)
    {
        shader.createUniform(HwShader::GLOBAL_SCOPE, DataType::MATRIX4, "u_worldInverseTranspose");
        shader.createVertexData(DataType::VECTOR3, "tangentWorld");
    }
    else if (space == MODEL)
    {
        shader.createVertexData(DataType::VECTOR3, "tangentModel");
    }
    else
    {
        shader.createVertexData(DataType::VECTOR3, "tangentObject");
    }
}

void TangentGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const string& blockInstance = shader.getVertexDataBlock().instance;
    const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;

    const SgInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (space == WORLD)
        {
            if (!shader.isCalculated("tangentWorld"))
            {
                shader.setCalculated("tangentWorld");
                shader.addLine(blockPrefix + "tangentWorld = normalize(u_worldInverseTranspose * i_tangent)");
            }
        }
        else if (space == MODEL)
        {
            if (!shader.isCalculated("tangentModel"))
            {
                shader.setCalculated("tangentModel");
                shader.addLine(blockPrefix + "tangentModel = i_tangent");
            }
        }
        else
        {
            if (!shader.isCalculated("tangentObject"))
            {
                shader.setCalculated("tangentObject");
                shader.addLine(blockPrefix + "tangentObject = i_tangent");
            }
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

        BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
    shadergen.emitOutput(node.getOutput(), true, shader);
    if (space == WORLD)
    {
        shader.addStr(" = " + blockPrefix + "tangentWorld");
    }
    else if (space == MODEL)
    {
        shader.addStr(" = " + blockPrefix + "tangentModel");
    }
    else
    {
        shader.addStr(" = " + blockPrefix + "tangentObject");
    }
    shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
