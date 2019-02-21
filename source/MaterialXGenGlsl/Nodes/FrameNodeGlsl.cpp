#include <MaterialXGenGlsl/Nodes/FrameNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr FrameNodeGlsl::create()
{
    return std::make_shared<FrameNodeGlsl>();
}

void FrameNodeGlsl::createVariables(Shader& shader, const ShaderNode&, const ShaderGenerator&, GenContext&) const
{
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);
    addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::FLOAT, "u_frame");
}

void FrameNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const
{
BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    shadergen.emitString(stage, " = u_frame");
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
