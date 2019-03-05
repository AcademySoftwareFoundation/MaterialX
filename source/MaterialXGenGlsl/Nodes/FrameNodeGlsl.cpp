#include <MaterialXGenGlsl/Nodes/FrameNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr FrameNodeGlsl::create()
{
    return std::make_shared<FrameNodeGlsl>();
}

void FrameNodeGlsl::createVariables(const ShaderNode&, GenContext&, Shader& shader) const
{
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);
    addStageUniform(HW::PRIVATE_UNIFORMS, Type::FLOAT, "u_frame", ps);
}

void FrameNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = u_frame", stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
