#include <MaterialXShaderGen/ShaderGenerators/Glsl/GeomAttrValueGlsl.h>

namespace MaterialX
{

SgImplementationPtr GeomAttrValueGlsl::create()
{
    return std::make_shared<GeomAttrValueGlsl>();
}

void GeomAttrValueGlsl::createVariables(const SgNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const SgInput* attrNameInput = node.getInput(ATTRNAME);
    if (!attrNameInput)
    {
        throw ExceptionShaderGenError("No 'attrname' parameter found on geomattrvalue node '" + node.getName() + "', don't know what attribute to bind");
    }
    const string attrName = attrNameInput->value->getValueString();
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, node.getOutput()->type, "u_geomattr_" + attrName);
}

void GeomAttrValueGlsl::emitFunctionCall(const SgNode& node, const SgNodeContext& /*context*/, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        const SgInput* attrNameInput = node.getInput(ATTRNAME);
        if (!attrNameInput)
        {
            throw ExceptionShaderGenError("No 'attrname' parameter found on geomattrvalue node '" + node.getName() + "', don't know what attribute to bind");
        }
        const string attrName = attrNameInput->value->getValueString();

        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);
        shader.addStr(" = u_geomattr_" + attrName);
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
