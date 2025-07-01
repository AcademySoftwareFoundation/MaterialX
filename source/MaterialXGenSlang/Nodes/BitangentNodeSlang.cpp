//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenSlang/Nodes/BitangentNodeSlang.h>

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr BitangentNodeSlang::create()
{
    return std::make_shared<BitangentNodeSlang>();
}

void BitangentNodeSlang::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());
    const GenOptions& options = context.getOptions();

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : OBJECT_SPACE;

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        if (space == WORLD_SPACE)
        {
            ShaderPort* bitangent = vertexData[HW::T_BITANGENT_WORLD];

            if (!bitangent->isEmitted())
            {
                bitangent->setEmitted();

                if (options.hwImplicitBitangents)
                {
                    ShaderPort* normal = vertexData[HW::T_NORMAL_WORLD];
                    if (!normal->isEmitted())
                    {
                        normal->setEmitted();
                        shadergen.emitLine(prefix + normal->getVariable() + " = normalize(mul(float4(" + HW::T_IN_NORMAL + ", 0.0), " + HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX + ").xyz)", stage);
                    }
                    ShaderPort* tangent = vertexData[HW::T_TANGENT_WORLD];
                    if (!tangent->isEmitted())
                    {
                        tangent->setEmitted();
                        shadergen.emitLine(prefix + tangent->getVariable() + " = normalize(mul(float4(" + HW::T_IN_TANGENT + ", 0.0), " + HW::T_WORLD_MATRIX + ").xyz)", stage);
                    }
                    shadergen.emitLine(prefix + bitangent->getVariable() + " = cross(" + prefix + normal->getVariable() + ", " + prefix + tangent->getVariable() + ")", stage);
                }
                else
                {
                    shadergen.emitLine(prefix + bitangent->getVariable() + " = normalize(mul(float4(" + HW::T_IN_BITANGENT + ", 0.0), " + HW::T_WORLD_MATRIX + ").xyz)", stage);
                }
            }
        }
        else
        {
            ShaderPort* bitangent = vertexData[HW::T_BITANGENT_OBJECT];
            if (!bitangent->isEmitted())
            {
                bitangent->setEmitted();

                if (options.hwImplicitBitangents)
                {
                    shadergen.emitLine(prefix + bitangent->getVariable() + " = cross(" + HW::T_IN_NORMAL + ", " + HW::T_IN_TANGENT + ")", stage);
                }
                else
                {
                    shadergen.emitLine(prefix + bitangent->getVariable() + " = " + HW::T_IN_BITANGENT, stage);
                }
            }
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
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
    }
}

MATERIALX_NAMESPACE_END
