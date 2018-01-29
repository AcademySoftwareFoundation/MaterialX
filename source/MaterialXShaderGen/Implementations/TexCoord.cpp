#include <MaterialXShaderGen/Implementations/TexCoord.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/SgNode.h>
#include <MaterialXShaderGen/HwShader.h>

namespace MaterialX
{

SgImplementationPtr TexCoordOgsFx::creator()
{
    return std::make_shared<TexCoordOgsFx>();
}

void TexCoordOgsFx::registerInputs(const SgNode& node, ShaderGenerator& shadergen, Shader& shader)
{
    const SgInput* indexInput = node.getInput(INDEX);
    const string index = indexInput ? indexInput->value->getValueString() : "0";
    const string type = shadergen.getSyntax()->getTypeName(node.getOutput()->type);

    shader.registerAttribute(Shader::Variable(type, "inUV" + index, "TEXCOORD" + index));
    shader.registerVarying(Shader::Variable(type, "UV" + index, "TEXCOORD" + index));
}

void TexCoordOgsFx::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const SgInput* indexInput = node.getInput(INDEX);
    string index = indexInput ? indexInput->value->getValueString() : "0";
    string variable = "UV" + index;

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (!shader.isCalculated(variable))
        {
            shader.addLine("VS_OUT." + variable + " = in" + variable);
            shader.setCalculated(variable);
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);
        shader.addStr(" = PS_IN." + variable);
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
