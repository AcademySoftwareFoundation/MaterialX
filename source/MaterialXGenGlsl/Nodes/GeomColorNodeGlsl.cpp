//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/GeomColorNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

ShaderNodeImplPtr GeomColorNodeGlsl::create()
{
    return std::make_shared<GeomColorNodeGlsl>();
}

void GeomColorNodeGlsl::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    const ShaderInput* indexInput = node.getInput(INDEX);
    const string index = indexInput ? indexInput->getValue()->getValueString() : "0";

    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);
    addStageInput(HW::VERTEX_INPUTS, Type::COLOR4, "i_color_" + index, vs);
    addStageConnector(HW::VERTEX_DATA, Type::COLOR4, "color_" + index, vs, ps);
}

void GeomColorNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    const ShaderOutput* output = node.getOutput();
    const ShaderInput* indexInput = node.getInput(INDEX);
    string index = indexInput ? indexInput->getValue()->getValueString() : "0";
    string variable = "color_" + index;

    BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        ShaderPort* color = vertexData[variable];
        if (!color->isEmitted())
        {
            color->setEmitted();
            shadergen.emitLine(prefix + color->getVariable() + " = i_" + variable, stage);
        }
    END_SHADER_STAGE(shader, Stage::VERTEX)

    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
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
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + prefix + color->getVariable() + suffix, stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

} // namespace MaterialX
