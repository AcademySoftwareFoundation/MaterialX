#include <MaterialXShaderGen/ShaderGenerators/Glsl/GeomAttrValueGlsl.h>

namespace MaterialX
{

SgImplementationPtr GeomAttrValueGlsl::creator()
{
    return std::make_shared<GeomAttrValueGlsl>();
}

void GeomAttrValueGlsl::createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const SgInput* attrNameInput = node.getInput(ATTRNAME);
    if (!attrNameInput)
    {
        throw ExceptionShaderGenError("No 'attrname' parameter found on geomattrvalue node '" + node.getName() + "', don't know what attribute to bind");
    }
    const string name = attrNameInput->value->getValueString();
    const string type = shadergen.getSyntax()->getTypeName(node.getOutput()->type);
    const string variable = "geomattr_" + name;

    shader.createAppData(type, "i_" + variable);
    shader.createVertexData(type, variable);
}

void GeomAttrValueGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const string& blockInstance = shader.getVertexDataBlock().instance;
    const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;

    const SgInput* attrNameInput = node.getInput(ATTRNAME);
    if (!attrNameInput)
    {
        throw ExceptionShaderGenError("No 'attrname' parameter found on geomattrvalue node '" + node.getName() + "', don't know what attribute to bind");
    }
    const string name = attrNameInput->value->getValueString();
    const string type = shadergen.getSyntax()->getTypeName(node.getOutput()->type);
    const string variable = "geomattr_" + name;

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (!shader.isCalculated(variable))
        {
            shader.addLine(blockPrefix + variable + " = i_" + variable);
            shader.setCalculated(variable);
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);
        shader.addStr(" = " + blockPrefix + variable);
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
