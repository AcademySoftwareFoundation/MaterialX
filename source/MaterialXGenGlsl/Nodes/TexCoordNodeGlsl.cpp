#include <MaterialXGenGlsl/Nodes/TexCoordNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr TexCoordNodeGlsl::create()
{
    return std::make_shared<TexCoordNodeGlsl>();
}

void TexCoordNodeGlsl::createVariables(Shader& shader, const ShaderNode& node, const ShaderGenerator&, GenContext&) const
{
    const ShaderOutput* output = node.getOutput();
    const ShaderInput* indexInput = node.getInput(INDEX);
    const string index = indexInput ? indexInput->getValue()->getValueString() : "0";

    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(vs, HW::VERTEX_INPUTS, output->getType(), "i_texcoord_" + index);
    addStageConnector(vs, ps, HW::VERTEX_DATA, output->getType(), "texcoord_" + index);
}

void TexCoordNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const
{
    const ShaderInput* indexInput = node.getInput(INDEX);
    const string index = indexInput ? indexInput->getValue()->getValueString() : "0";
    const string variable = "texcoord_" + index;

    BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        ShaderPort* texcoord = vertexData[variable];
        if (!texcoord->isEmitted())
        {
            shadergen.emitLine(stage, prefix + texcoord->getVariable() + " = i_" + variable);
            texcoord->setEmitted();
        }
    END_SHADER_STAGE(shader, HW::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        ShaderPort* texcoord = vertexData[variable];
            shadergen.emitLineBegin(stage);
        shadergen.emitOutput(stage, context, node.getOutput(), true, false);
        shadergen.emitString(stage, " = " + prefix + texcoord->getVariable());
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
