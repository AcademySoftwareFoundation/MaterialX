#include <MaterialXGenGlsl/Nodes/FrameGlsl.h>

namespace MaterialX
{

SgImplementationPtr FrameGlsl::create()
{
    return std::make_shared<FrameGlsl>();
}

void FrameGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, Type::FLOAT, "u_frame");
}

void FrameGlsl::emitFunctionCall(const SgNode& node, const SgNodeContext& /*context*/, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, false, shader);
        shader.addStr(" = u_frame");
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
