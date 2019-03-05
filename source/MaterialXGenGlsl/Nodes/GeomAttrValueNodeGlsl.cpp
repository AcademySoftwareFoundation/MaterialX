#include <MaterialXGenGlsl/Nodes/GeomAttrValueNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr GeomAttrValueNodeGlsl::create()
{
    return std::make_shared<GeomAttrValueNodeGlsl>();
}

void GeomAttrValueNodeGlsl::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    const ShaderInput* attrNameInput = node.getInput(ATTRNAME);
    if (!attrNameInput || !attrNameInput->getValue())
    {
        throw ExceptionShaderGenError("No 'attrname' parameter found on geomattrvalue node '" + node.getName() + "', don't know what attribute to bind");
    }
    const string attrName = attrNameInput->getValue()->getValueString();
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);
    ShaderPort* uniform = addStageUniform(HW::PRIVATE_UNIFORMS, node.getOutput()->getType(), "u_geomattr_" + attrName, ps);
    uniform->setPath(attrNameInput->getPath());
}

void GeomAttrValueNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const ShaderInput* attrNameInput = node.getInput(ATTRNAME);
        if (!attrNameInput)
        {
            throw ExceptionShaderGenError("No 'attrname' parameter found on geomattrvalue node '" + node.getName() + "', don't know what attribute to bind");
        }
        const string attrName = attrNameInput->getValue()->getValueString();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = u_geomattr_" + attrName, stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
