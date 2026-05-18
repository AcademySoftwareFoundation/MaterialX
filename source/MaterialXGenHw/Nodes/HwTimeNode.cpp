//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHw/Nodes/HwTimeNode.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr HwTimeNode::create()
{
    return std::make_shared<HwTimeNode>();
}

void HwTimeNode::createVariables(const ShaderNode&, GenContext&, Shader& shader) const
{
    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);
    // Time uniform is needed in both stages (displacement may use time)
    addStageUniform(HW::PRIVATE_UNIFORMS, Type::FLOAT, HW::T_TIME, vs);
    addStageUniform(HW::PRIVATE_UNIFORMS, Type::FLOAT, HW::T_TIME, ps);
}

void HwTimeNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        // Emit in vertex stage when displacement is being evaluated
        if (context.getEmitVertexDisplacement())
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(node.getOutput(), true, false, context, stage);
            shadergen.emitString(" = " + HW::T_TIME, stage);
            shadergen.emitLineEnd(stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + HW::T_TIME, stage);
        shadergen.emitLineEnd(stage);
    }
}

MATERIALX_NAMESPACE_END
