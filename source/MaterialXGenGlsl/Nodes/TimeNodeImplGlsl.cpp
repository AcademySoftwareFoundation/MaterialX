#include <MaterialXGenGlsl/Nodes/TimeNodeImplGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr TimeNodeImplGlsl::create()
{
    return std::make_shared<TimeNodeImplGlsl>();
}

void TimeNodeImplGlsl::createVariables(const ShaderNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, Type::FLOAT, "u_frame");
}

void TimeNodeImplGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        shader.addStr(" = u_frame / ");
        const ShaderInput* fpsInput = node.getInput("fps");
        const string fps = fpsInput->value->getValueString();
        shader.addStr(fps);
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
