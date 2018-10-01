#include <MaterialXGenGlsl/Nodes/ViewDirectionGlsl.h>

namespace MaterialX
{

SgImplementationPtr ViewDirectionGlsl::create()
{
    return std::make_shared<ViewDirectionGlsl>();
}

void ViewDirectionGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createAppData(Type::VECTOR3, "i_position");
    shader.createVertexData(Type::VECTOR3, "positionWorld");
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, Type::VECTOR3, "u_viewPosition");
}

void ViewDirectionGlsl::emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (!shader.isCalculated("positionWorld"))
        {
            shader.setCalculated("positionWorld");
            shader.addLine("vd.positionWorld = hPositionWorld.xyz");
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        shader.addStr(" = normalize(vd.positionWorld - u_viewPosition)");
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
