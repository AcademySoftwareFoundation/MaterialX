#include <MaterialXGenGlsl/Nodes/TexCoordNodeImplGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr TexCoordNodeImplGlsl::create()
{
    return std::make_shared<TexCoordNodeImplGlsl>();
}

void TexCoordNodeImplGlsl::createVariables(const ShaderNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const ShaderOutput* output = node.getOutput();
    const ShaderInput* indexInput = node.getInput(INDEX);
    const string index = indexInput ? indexInput->value->getValueString() : "0";

    shader.createAppData(output->type, "i_texcoord_" + index);
    shader.createVertexData(output->type, "texcoord_" + index);
}

void TexCoordNodeImplGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const string& blockInstance = shader.getVertexDataBlock().instance;
    const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;

    const ShaderInput* indexInput = node.getInput(INDEX);
    const string index = indexInput ? indexInput->value->getValueString() : "0";
    const string variable = "texcoord_" + index;

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (!shader.isCalculated(variable))
        {
            shader.addLine(blockPrefix + variable + " = i_" + variable);
            shader.setCalculated(variable);
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        shader.addStr(" = " + blockPrefix + variable);
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
