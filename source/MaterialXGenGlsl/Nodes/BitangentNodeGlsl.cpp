//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/BitangentNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

ShaderNodeImplPtr BitangentNodeGlsl::create()
{
    return std::make_shared<BitangentNodeGlsl>();
}

void BitangentNodeGlsl::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, HW::T_IN_NORMAL, vs);
    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, HW::T_IN_TANGENT, vs);

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : OBJECT_SPACE;
    if (space == WORLD_SPACE)
    {
        addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX, vs);
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, HW::T_BITANGENT_WORLD, vs, ps);
    }
    else
    {
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, HW::T_BITANGENT_OBJECT, vs, ps);
    }
}

void BitangentNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : OBJECT_SPACE;

    BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        if (space == WORLD_SPACE)
        {
            ShaderPort* bitangent = vertexData[HW::T_BITANGENT_WORLD];
            if (!bitangent->isEmitted())
            {
                bitangent->setEmitted();
                shadergen.emitLine(prefix + bitangent->getVariable() +
                    " = (" + HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX + " * vec4(cross(" + HW::T_IN_NORMAL + ", " + HW::T_IN_TANGENT + "), 0.0)).xyz", stage);
            }
        }
        else
        {
            ShaderPort* bitangent = vertexData[HW::T_BITANGENT_OBJECT];
            if (!bitangent->isEmitted())
            {
                bitangent->setEmitted();
                shadergen.emitLine(prefix + bitangent->getVariable() + " = cross(" + HW::T_IN_NORMAL + ", " + HW::T_IN_TANGENT + ")", stage);
            }
        }
    END_SHADER_STAGE(stage, Stage::VERTEX)

    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        if (space == WORLD_SPACE)
        {
            const ShaderPort* bitangent = vertexData[HW::T_BITANGENT_WORLD];
            shadergen.emitString(" = normalize(" + prefix + bitangent->getVariable() + ")", stage);
        }
        else
        {
            const ShaderPort* bitangent = vertexData[HW::T_BITANGENT_OBJECT];
            shadergen.emitString(" = normalize(" + prefix + bitangent->getVariable() + ")", stage);
        }
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

} // namespace MaterialX
