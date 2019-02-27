#include <MaterialXGenGlsl/Nodes/NormalNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr NormalNodeGlsl::create()
{
    return std::make_shared<NormalNodeGlsl>();
}

void NormalNodeGlsl::createVariables(Shader& shader, const ShaderNode& node, const ShaderGenerator&, GenContext&) const
{
    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(vs, HW::VERTEX_INPUTS, Type::VECTOR3, "i_normal");

    const ShaderInput* spaceInput = node.getInput(SPACE);
        const int space = spaceInput ? spaceInput->getValue()->asA<int>() : -1;
    if (space == WORLD_SPACE)
    {
        addStageUniform(vs, HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix");
        addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "normalWorld");
    }
    else if (space == MODEL_SPACE)
    {
        addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "normalModel");
    }
    else
    {
        addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "normalObject");
    }
}

void NormalNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const
{
    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : -1;

BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
    VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    const string prefix = vertexData.getInstance() + ".";
    if (space == WORLD_SPACE)
    {
        ShaderPort* normal = vertexData["normalWorld"];
        if (!normal->isEmitted())
        {
            normal->setEmitted();
            shadergen.emitLine(stage, prefix + normal->getVariable() + " = (u_worldInverseTransposeMatrix * vec4(i_normal,0.0)).xyz");
        }
    }
    else if (space == MODEL_SPACE)
    {
        ShaderPort* normal = vertexData["normalModel"];
        if (!normal->isEmitted())
        {
            normal->setEmitted();
            shadergen.emitLine(stage, prefix + normal->getVariable() + " = i_normal");
        }
    }
    else
    {
        ShaderPort* normal = vertexData["normalObject"];
        if (!normal->isEmitted())
        {
            normal->setEmitted();
            shadergen.emitLine(stage, prefix + normal->getVariable() + " = i_normal");
        }
    }
END_SHADER_STAGE(shader, HW::VERTEX_STAGE)

BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    const string prefix = vertexData.getInstance() + ".";
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    if (space == WORLD_SPACE)
    {
        const ShaderPort* normal = vertexData["normalWorld"];
        shadergen.emitString(stage, " = normalize(" + prefix + normal->getVariable() + ")");
    }
    else if (space == MODEL_SPACE)
    {
        const ShaderPort* normal = vertexData["normalModel"];
        shadergen.emitString(stage, " = normalize(" + prefix + normal->getVariable() + ")");
    }
    else
    {
        const ShaderPort* normal = vertexData["normalObject"];
        shadergen.emitString(stage, " = normalize(" + prefix + normal->getVariable() + ")");
    }
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
