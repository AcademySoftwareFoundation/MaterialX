#include <MaterialXGenGlsl/Nodes/GeomColorNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr GeomColorNodeGlsl::create()
{
    return std::make_shared<GeomColorNodeGlsl>();
}

void GeomColorNodeGlsl::createVariables(Shader& shader, const ShaderNode& node, const ShaderGenerator&, GenContext&) const
{
    const ShaderInput* indexInput = node.getInput(INDEX);
    const string index = indexInput ? indexInput->getValue()->getValueString() : "0";

    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);
    addStageInput(vs, HW::VERTEX_INPUTS, Type::COLOR4, "i_color_" + index);
    addStageConnector(vs, ps, HW::VERTEX_DATA, Type::COLOR4, "color_" + index);
}

void GeomColorNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const
{
    const ShaderOutput* output = node.getOutput();
    const ShaderInput* indexInput = node.getInput(INDEX);
    string index = indexInput ? indexInput->getValue()->getValueString() : "0";
    string variable = "color_" + index;

BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
    VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    const string prefix = vertexData.getInstance() + ".";
    ShaderPort* color = vertexData[variable];
    if (!color->isEmitted())
    {
        color->setEmitted();
        shadergen.emitLine(stage, prefix + color->getVariable() + " = i_" + variable);
    }
END_SHADER_STAGE(shader, HW::VERTEX_STAGE)

BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    string suffix = "";
    if (output->getType() == Type::FLOAT)
    {
        suffix = ".r";
    }
    else if (output->getType() == Type::COLOR2)
    {
        suffix = ".rg";
    }
    else if (output->getType() == Type::COLOR3)
    {
        suffix = ".rgb";
    }
    VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    const string prefix = vertexData.getInstance() + ".";
    ShaderPort* color = vertexData[variable];
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    shadergen.emitString(stage, " = " + prefix + color->getVariable() + suffix);
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
