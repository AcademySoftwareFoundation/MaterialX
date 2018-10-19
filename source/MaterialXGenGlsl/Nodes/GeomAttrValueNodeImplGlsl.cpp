#include <MaterialXGenGlsl/Nodes/GeomAttrValueNodeImplGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr GeomAttrValueNodeImplGlsl::create()
{
    return std::make_shared<GeomAttrValueNodeImplGlsl>();
}

void GeomAttrValueNodeImplGlsl::createVariables(const ShaderNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const ShaderInput* attrNameInput = node.getInput(ATTRNAME);
    if (!attrNameInput)
    {
        throw ExceptionShaderGenError("No 'attrname' parameter found on geomattrvalue node '" + node.getName() + "', don't know what attribute to bind");
    }
    const string attrName = attrNameInput->value->getValueString();
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, node.getOutput()->type, "u_geomattr_" + attrName);
}

void GeomAttrValueNodeImplGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        const ShaderInput* attrNameInput = node.getInput(ATTRNAME);
        if (!attrNameInput)
        {
            throw ExceptionShaderGenError("No 'attrname' parameter found on geomattrvalue node '" + node.getName() + "', don't know what attribute to bind");
        }
        const string attrName = attrNameInput->value->getValueString();

        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        shader.addStr(" = u_geomattr_" + attrName);
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
