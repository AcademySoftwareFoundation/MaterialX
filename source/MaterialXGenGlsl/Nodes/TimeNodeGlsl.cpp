#include <MaterialXGenGlsl/Nodes/TimeNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr TimeNodeGlsl::create()
{
    return std::make_shared<TimeNodeGlsl>();
}

void TimeNodeGlsl::createVariables(const ShaderNode&, GenContext&, Shader& shader) const
{
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);
    addStageUniform(HW::PRIVATE_UNIFORMS, Type::FLOAT, "u_frame", ps);
}

void TimeNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = u_frame / ", stage);
        const ShaderInput* fpsInput = node.getInput("fps");
        const string fps = fpsInput->getValue()->getValueString();
        shadergen.emitString(fps, stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(stage, HW::PIXEL_STAGE)
}

} // namespace MaterialX
