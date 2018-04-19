#include <MaterialXShaderGen/ShaderGenerators/Glsl/TexCoordGlsl.h>

namespace MaterialX
{

SgImplementationPtr TexCoordGlsl::create()
{
    return std::make_shared<TexCoordGlsl>();
}

void TexCoordGlsl::createVariables(const SgNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const SgOutput* output = node.getOutput();
    const SgInput* indexInput = node.getInput(INDEX);
    const string index = indexInput ? indexInput->value->getValueString() : "0";

    shader.createAppData(output->type, "i_texcoord_" + index);
    shader.createVertexData(output->type, "texcoord_" + index);
}

void TexCoordGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const string& blockInstance = shader.getVertexDataBlock().instance;
    const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;

    const SgInput* indexInput = node.getInput(INDEX);
    string index = indexInput ? indexInput->value->getValueString() : "0";
    string variable = "texcoord_" + index;

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
