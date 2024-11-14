//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/Nodes/HwViewDirectionNode.h>

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr HwViewDirectionNode::create()
{
    return std::make_shared<HwViewDirectionNode>();
}

void HwViewDirectionNode::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, HW::T_IN_POSITION, vs);

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : OBJECT_SPACE;

    addStageUniform(HW::PRIVATE_UNIFORMS, Type::VECTOR3, HW::T_VIEW_POSITION, ps);
    addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, HW::T_POSITION_WORLD, vs, ps);
    if (space == OBJECT_SPACE || space == MODEL_SPACE)
    {
        addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX, ps);
    }
}

void HwViewDirectionNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : OBJECT_SPACE;

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* position = vertexData[HW::T_POSITION_WORLD];
        if (!position->isEmitted())
        {
            position->setEmitted();
            shadergen.emitLine(prefix + position->getVariable() + " = hPositionWorld.xyz", stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        ShaderPort* position = vertexData[HW::T_POSITION_WORLD];
        if (space == WORLD_SPACE)
        {
            shadergen.emitString(" = normalize(" + prefix + position->getVariable() + " - " + HW::T_VIEW_POSITION + ")", stage);
        }
        else
        {
            shadergen.emitString(" = normalize((" + HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX + " * vec4(" + prefix + position->getVariable() + " - " + HW::T_VIEW_POSITION + ", 0.0)).xyz)", stage);
        }
        shadergen.emitLineEnd(stage);
    }
}

MATERIALX_NAMESPACE_END
