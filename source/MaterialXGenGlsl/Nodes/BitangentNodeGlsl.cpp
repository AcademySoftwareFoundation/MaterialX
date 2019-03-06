//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/BitangentNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr BitangentNodeGlsl::create()
{
    return std::make_shared<BitangentNodeGlsl>();
}

void BitangentNodeGlsl::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, "i_bitangent", vs);

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : -1;
    if (space == WORLD_SPACE)
    {
        addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix", vs);
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, "bitangentWorld", vs, ps);
    }
    else if (space == MODEL_SPACE)
    {
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, "bitangentModel", vs, ps);
    }
    else
    {
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, "bitangentObject", vs, ps);
    }
}

void BitangentNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : -1;

    BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        if (space == WORLD_SPACE)
        {
            ShaderPort* bitangent = vertexData["bitangentWorld"];
            if (!bitangent->isEmitted())
            {
                bitangent->setEmitted();
                shadergen.emitLine(prefix + bitangent->getVariable() + " = (u_worldInverseTransposeMatrix * vec4(i_bitangent,0.0)).xyz", stage);
            }
        }
        else if (space == MODEL_SPACE)
        {
            ShaderPort* bitangent = vertexData["bitangentModel"];
            if (!bitangent->isEmitted())
            {
                bitangent->setEmitted();
                shadergen.emitLine(prefix + bitangent->getVariable() + " = i_bitangent", stage);
            }
        }
        else
        {
            ShaderPort* bitangent = vertexData["bitangentObject"];
            if (!bitangent->isEmitted())
            {
                bitangent->setEmitted();
                shadergen.emitLine(prefix + bitangent->getVariable() + " = i_bitangent", stage);
            }
        }
    END_SHADER_STAGE(stage, HW::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        if (space == WORLD_SPACE)
        {
            const ShaderPort* bitangent = vertexData["bitangentWorld"];
            shadergen.emitString(" = normalize(" + prefix + bitangent->getVariable() + ")", stage);
        }
        else if (space == MODEL_SPACE)
        {
            const ShaderPort* bitangent = vertexData["bitangentModel"];
            shadergen.emitString(" = normalize(" + prefix + bitangent->getVariable() + ")", stage);
        }
        else
        {
            const ShaderPort* bitangent = vertexData["bitangentObject"];
            shadergen.emitString(" = normalize(" + prefix + bitangent->getVariable() + ")", stage);
        }
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
