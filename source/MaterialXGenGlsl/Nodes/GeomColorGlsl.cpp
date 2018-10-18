#include <MaterialXGenGlsl/Nodes/GeomColorGlsl.h>

namespace MaterialX
{

ShaderImplementationPtr GeomColorGlsl::create()
{
    return std::make_shared<GeomColorGlsl>();
}

void GeomColorGlsl::createVariables(const ShaderNode& node, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const ShaderInput* indexInput = node.getInput(INDEX);
    const string index = indexInput ? indexInput->value->getValueString() : "0";

    shader.createAppData(Type::COLOR4, "i_color_" + index);
    shader.createVertexData(Type::COLOR4, "color_" + index);
}

void GeomColorGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    const string& blockInstance = shader.getVertexDataBlock().instance;
    const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;

    const ShaderOutput* output = node.getOutput();
    const ShaderInput* indexInput = node.getInput(INDEX);
    string index = indexInput ? indexInput->value->getValueString() : "0";
    string variable = "color_" + index;

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (!shader.isCalculated(variable))
        {
            shader.addLine(blockPrefix + variable + " = i_" + variable);
            shader.setCalculated(variable);
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        string suffix = "";
        if (output->type == Type::FLOAT)
        {
            suffix = ".r";
        }
        else if (output->type == Type::COLOR2)
        {
            suffix = ".rg";
        }
        else if (output->type == Type::COLOR3)
        {
            suffix = ".rgb";
        }
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        shader.addStr(" = " + blockPrefix + variable + suffix);
        shader.endLine();
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
