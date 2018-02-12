#include <MaterialXShaderGen/ShaderGenerators/Glsl/ViewGlsl.h>

namespace MaterialX
{

SgImplementationPtr ViewGlsl::creator()
{
    return std::make_shared<ViewGlsl>();
}

void ViewGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createUniform(HwShader::GLOBAL_SCOPE, DataType::MATRIX4, "u_viewInverseMatrix");
    shader.createVertexData(DataType::VECTOR3, "viewWorld");
}

void ViewGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (!shader.isCalculated("viewWorld"))
        {
            shader.setCalculated("viewWorld");
            shader.addLine("vd.viewWorld = normalize(u_viewInverseMatrix[3].xyz - hPositionWorld.xyz)");
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);
        shader.addStr(" = vd.viewWorld");
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
