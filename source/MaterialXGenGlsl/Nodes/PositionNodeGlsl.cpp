#include <MaterialXGenGlsl/Nodes/PositionNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr PositionNodeGlsl::create()
{
    return std::make_shared<PositionNodeGlsl>();
}

void PositionNodeGlsl::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    ShaderStage vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, "i_position", vs);

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : -1;
    if (space == WORLD_SPACE)
    {
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, "positionWorld", vs, ps);
    }
    else if (space == MODEL_SPACE)
    {
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, "positionModel", vs, ps);
    }
    else
    {
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, "positionObject", vs, ps);
    }
}

void PositionNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

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
                shadergen.emitLine(prefix + position->getVariable() + " = hPositionWorld.xyz", stage);
            }
        }
        else if (space == MODEL_SPACE)
        {
            ShaderPort* position = vertexData["positionModel"];
            if (!position->isEmitted())
            {
                position->setEmitted();
                shadergen.emitLine(prefix + position->getVariable() + " = i_position", stage);
            }
        }
        else
        {
            ShaderPort* position = vertexData["positionObject"];
            if (!position->isEmitted())
            {
                position->setEmitted();
                shadergen.emitLine(prefix + position->getVariable() + " = i_position", stage);
            }
        }
    END_SHADER_STAGE(shader, HW::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        if (space == WORLD_SPACE)
        {
            const ShaderPort* position = vertexData["positionWorld"];
            shadergen.emitString(" = " + prefix + position->getVariable(), stage);
        }
        else if (space == MODEL_SPACE)
        {
            const ShaderPort* position = vertexData["positionModel"];
            shadergen.emitString(" = " + prefix + position->getVariable(), stage);
        }
        else
        {
            const ShaderPort* position = vertexData["positionObject"];
            shadergen.emitString(" = " + prefix + position->getVariable(), stage);
        }
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
