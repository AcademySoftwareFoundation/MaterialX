#include <MaterialXGenGlsl/Nodes/GeomAttrValueNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr GeomAttrValueNodeGlsl::create()
{
    return std::make_shared<GeomAttrValueNodeGlsl>();
}

void GeomAttrValueNodeGlsl::createVariables(Shader& shader, const ShaderNode& node, const ShaderGenerator&, GenContext&) const
{
    const ShaderInput* attrNameInput = node.getInput(ATTRNAME);
    if (!attrNameInput || !attrNameInput->value)
    {
        throw ExceptionShaderGenError("No 'attrname' parameter found on geomattrvalue node '" + node.getName() + "', don't know what attribute to bind");
    }
    const string attrName = attrNameInput->value->getValueString();
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);
    addStageUniform(ps, HW::PRIVATE_UNIFORMS, node.getOutput()->type, "u_geomattr_" + attrName, EMPTY_STRING, nullptr, attrNameInput->path);
}

void GeomAttrValueNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const
{
BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    const ShaderInput* attrNameInput = node.getInput(ATTRNAME);
    if (!attrNameInput)
    {
        throw ExceptionShaderGenError("No 'attrname' parameter found on geomattrvalue node '" + node.getName() + "', don't know what attribute to bind");
    }
    const string attrName = attrNameInput->value->getValueString();
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    shadergen.emitString(stage, " = u_geomattr_" + attrName);
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
