#include <MaterialXGenGlsl/Nodes/PositionNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr PositionNodeGlsl::create()
{
    return std::make_shared<PositionNodeGlsl>();
}

void PositionNodeGlsl::createVariables(Shader& shader, const ShaderNode& node, ShaderGenerator&, GenContext&) const
{
    ShaderStage vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(vs, HW::VERTEX_INPUTS, Type::VECTOR3, "i_position");

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->value->asA<int>() : -1;
    if (space == WORLD_SPACE)
    {
        addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "positionWorld");
    }
    else if (space == MODEL_SPACE)
    {
        addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "positionModel");
    }
    else
    {
        addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "positionObject");
    }
}

void PositionNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context) const
{
    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->value->asA<int>() : -1;

BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
    VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    if (space == WORLD_SPACE)
    {
        Variable& position = vertexData["positionWorld"];
        if (!position.isCalculated())
        {
            position.setCalculated();
            shadergen.emitLine(stage, position.getFullName() + " = hPositionWorld.xyz");
        }
    }
    else if (space == MODEL_SPACE)
    {
        Variable& position = vertexData["positionModel"];
        if (!position.isCalculated())
        {
            position.setCalculated();
            shadergen.emitLine(stage, position.getFullName() + " = i_position");
        }
    }
    else
    {
        Variable& position = vertexData["positionObject"];
        if (!position.isCalculated())
        {
            position.setCalculated();
            shadergen.emitLine(stage, position.getFullName() + " = i_position");
        }
    }
END_SHADER_STAGE(shader, HW::VERTEX_STAGE)

BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    if (space == WORLD_SPACE)
    {
        shadergen.emitString(stage, " = " + vertexData["positionWorld"].getFullName());
    }
    else if (space == MODEL_SPACE)
    {
        shadergen.emitString(stage, " = " + vertexData["positionModel"].getFullName());
    }
    else
    {
        shadergen.emitString(stage, " = " + vertexData["positionObject"].getFullName());
    }
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
