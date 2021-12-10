//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/PositionNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr PositionNodeGlsl::create()
{
    return std::make_shared<PositionNodeGlsl>();
}

void PositionNodeGlsl::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    ShaderStage vs = shader.getStage(Stage::VERTEX);
    ShaderStage ps = shader.getStage(Stage::PIXEL);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, HW::T_IN_POSITION, vs);

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : OBJECT_SPACE;
    if (space == WORLD_SPACE)
    {
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, HW::T_POSITION_WORLD, vs, ps);
    }
    else
    {
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, HW::T_POSITION_OBJECT, vs, ps);
    }
}

void PositionNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const GlslShaderGenerator& shadergen = static_cast<const GlslShaderGenerator&>(context.getShaderGenerator());

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : OBJECT_SPACE;

    BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        if (space == WORLD_SPACE)
        {
            ShaderPort* position = vertexData[HW::T_POSITION_WORLD];
            if (!position->isEmitted())
            {
                position->setEmitted();
                shadergen.emitLine(prefix + position->getVariable() + " = hPositionWorld.xyz", stage);
            }
        }
        else
        {
            ShaderPort* position = vertexData[HW::T_POSITION_OBJECT];
            if (!position->isEmitted())
            {
                position->setEmitted();
                shadergen.emitLine(prefix + position->getVariable() + " = " + HW::T_IN_POSITION, stage);
            }
        }
    END_SHADER_STAGE(shader, Stage::VERTEX)

    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        if (space == WORLD_SPACE)
        {
            const ShaderPort* position = vertexData[HW::T_POSITION_WORLD];
            shadergen.emitString(" = " + prefix + position->getVariable(), stage);
        }
        else
        {
            const ShaderPort* position = vertexData[HW::T_POSITION_OBJECT];
            shadergen.emitString(" = " + prefix + position->getVariable(), stage);
        }
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

MATERIALX_NAMESPACE_END
