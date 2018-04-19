#include <MaterialXShaderGen/ShaderGenerators/Glsl/ViewDirectionGlsl.h>

namespace MaterialX
{

SgImplementationPtr ViewDirectionGlsl::create()
{
    return std::make_shared<ViewDirectionGlsl>();
}

void ViewDirectionGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, DataType::VECTOR3, "u_viewDirection");
}

void ViewDirectionGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);
        shader.addStr(" = u_viewDirection");
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
