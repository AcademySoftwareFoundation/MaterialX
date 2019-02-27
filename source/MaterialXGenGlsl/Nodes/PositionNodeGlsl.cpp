#include <MaterialXGenGlsl/Nodes/PositionNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr PositionNodeGlsl::create()
{
    return std::make_shared<PositionNodeGlsl>();
}

void PositionNodeGlsl::createVariables(Shader& shader, const ShaderNode& node, const ShaderGenerator&, GenContext&) const
{
    ShaderStage vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(vs, HW::VERTEX_INPUTS, Type::VECTOR3, "i_position");

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : -1;
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

void PositionNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const
{
    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : -1;

BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
    VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    const string prefix = vertexData.getInstance() + ".";
    if (space == WORLD_SPACE)
    {
        ShaderPort* position = vertexData["positionWorld"];
        if (!position->isEmitted())
        {
            position->setEmitted();
            shadergen.emitLine(stage, prefix + position->getVariable() + " = hPositionWorld.xyz");
        }
    }
    else if (space == MODEL_SPACE)
    {
        ShaderPort* position = vertexData["positionModel"];
        if (!position->isEmitted())
        {
            position->setEmitted();
            shadergen.emitLine(stage, prefix + position->getVariable() + " = i_position");
        }
    }
    else
    {
        ShaderPort* position = vertexData["positionObject"];
        if (!position->isEmitted())
        {
            position->setEmitted();
            shadergen.emitLine(stage, prefix + position->getVariable() + " = i_position");
        }
    }
END_SHADER_STAGE(shader, HW::VERTEX_STAGE)

BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    const string prefix = vertexData.getInstance() + ".";
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    if (space == WORLD_SPACE)
    {
        const ShaderPort* position = vertexData["positionWorld"];
        shadergen.emitString(stage, " = " + prefix + position->getVariable());
    }
    else if (space == MODEL_SPACE)
    {
        const ShaderPort* position = vertexData["positionModel"];
        shadergen.emitString(stage, " = " + prefix + position->getVariable());
    }
    else
    {
        const ShaderPort* position = vertexData["positionObject"];
        shadergen.emitString(stage, " = " + prefix + position->getVariable());
    }
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
