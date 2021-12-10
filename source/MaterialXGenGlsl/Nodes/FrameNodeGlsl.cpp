//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/FrameNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr FrameNodeGlsl::create()
{
    return std::make_shared<FrameNodeGlsl>();
}

void FrameNodeGlsl::createVariables(const ShaderNode&, GenContext&, Shader& shader) const
{
    ShaderStage& ps = shader.getStage(Stage::PIXEL);
    addStageUniform(HW::PRIVATE_UNIFORMS, Type::FLOAT, HW::T_FRAME, ps);
}

void FrameNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + HW::T_FRAME, stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

MATERIALX_NAMESPACE_END
