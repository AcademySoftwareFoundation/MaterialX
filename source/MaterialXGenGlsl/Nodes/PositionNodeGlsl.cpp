#include <MaterialXGenGlsl/Nodes/PositionNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr PositionNodeGlsl::create()
{
    return std::make_shared<PositionNodeGlsl>();
}

void PositionNodeGlsl::createVariables(const ShaderNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createAppData(Type::VECTOR3, "i_position");

    const ShaderInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;
    if (space == WORLD)
    {
        shader.createVertexData(Type::VECTOR3, "positionWorld");
    }
    else if (space == MODEL)
    {
        shader.createVertexData(Type::VECTOR3, "positionModel");
    }
    else
    {
        shader.createVertexData(Type::VECTOR3, "positionObject");
    }
}

void PositionNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const string& blockInstance = shader.getVertexDataBlock().instance;
    const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;

    const ShaderInput* spaceInput = node.getInput(SPACE);
    string space = spaceInput ? spaceInput->value->getValueString() : EMPTY_STRING;

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (space == WORLD)
        {
            if (!shader.isCalculated("positionWorld"))
            {
                shader.setCalculated("positionWorld");
                shader.addLine(blockPrefix + "positionWorld = hPositionWorld.xyz");
            }
        }
        else if (space == MODEL)
        {
            if (!shader.isCalculated("positionModel"))
            {
                shader.setCalculated("positionModel");
                shader.addLine(blockPrefix + "positionModel = i_position");
            }
        }
        else
        {
            if (!shader.isCalculated("positionObject"))
            {
                shader.setCalculated("positionObject");
                shader.addLine(blockPrefix + "positionObject = i_position");
            }
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        if (space == WORLD)
        {
            shader.addStr(" = " + blockPrefix + "positionWorld");
        }
        else if (space == MODEL)
        {
            shader.addStr(" = " + blockPrefix + "positionModel");
        }
        else
        {
            shader.addStr(" = " + blockPrefix + "positionObject");
        }

        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
