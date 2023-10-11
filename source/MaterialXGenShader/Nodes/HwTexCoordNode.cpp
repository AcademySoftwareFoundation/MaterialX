//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/Nodes/HwTexCoordNode.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr HwTexCoordNode::create()
{
    return std::make_shared<HwTexCoordNode>();
}

void HwTexCoordNode::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    const ShaderOutput* output = node.getOutput();
    const ShaderInput* indexInput = getIndexInput(node);
    const string index = indexInput ? indexInput->getValue()->getValueString() : "0";

    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    addStageInput(HW::VERTEX_INPUTS, output->getType(), HW::T_IN_TEXCOORD + "_" + index, vs);
    addStageConnector(HW::VERTEX_DATA, output->getType(), HW::T_TEXCOORD + "_" + index, vs, ps);
}

void HwTexCoordNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());

    const ShaderInput* indexInput = getIndexInput(node);
    const string index = indexInput ? indexInput->getValue()->getValueString() : "0";
    const string variable = HW::T_TEXCOORD + "_" + index;

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* texcoord = vertexData[variable];
        if (!texcoord->isEmitted())
        {
            shadergen.emitLine(prefix + texcoord->getVariable() + " = " + HW::T_IN_TEXCOORD + "_" + index, stage);
            texcoord->setEmitted();
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* texcoord = vertexData[variable];
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);

        // Extract the requested number of components from the texture coordinates (which may be a
        // larger datatype than the requested number of texture coordinates, if several texture
        // coordinate nodes with different width coexist).
        std::array<const char*, 5> components { "", "x", "xy", "xyz", "xyzw" };
        const string texCoord = shadergen.getSyntax().getSwizzledVariable(
            prefix + texcoord->getVariable(), texcoord->getType(),
            components[node.getOutput()->getType()->getSize()],
            node.getOutput()->getType());

        shadergen.emitString(" = " + texCoord, stage);
        shadergen.emitLineEnd(stage);
    }
}

const ShaderInput* HwTexCoordNode::getIndexInput(const ShaderNode& node) const
{
    return node.getInput("index");
}

MATERIALX_NAMESPACE_END

