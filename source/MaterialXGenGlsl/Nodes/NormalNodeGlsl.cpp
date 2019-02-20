#include <MaterialXGenGlsl/Nodes/NormalNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr NormalNodeGlsl::create()
{
    return std::make_shared<NormalNodeGlsl>();
}

void NormalNodeGlsl::createVariables(Shader& shader, const ShaderNode& node, ShaderGenerator&, GenContext&) const
{
    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(vs, HW::VERTEX_INPUTS, Type::VECTOR3, "i_normal");

    const ShaderInput* spaceInput = node.getInput(SPACE);
        const int space = spaceInput ? spaceInput->value->asA<int>() : -1;
    const string& path = spaceInput ? spaceInput->path : EMPTY_STRING;
    if (space == WORLD_SPACE)
    {
        addStageUniform(vs, HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix", EMPTY_STRING, nullptr, path);
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

void NormalNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context) const
{
    const ShaderInput* spaceInput = node.getInput(SPACE);
        const int space = spaceInput ? spaceInput->value->asA<int>() : -1;

BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
    VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    if (space == WORLD_SPACE)
    {
        Variable& normal = vertexData["normalWorld"];
        if (!normal.isCalculated())
        {
            normal.setCalculated();
            shadergen.emitLine(stage, normal.getFullName() + " = (u_worldInverseTransposeMatrix * vec4(i_normal,0.0)).xyz");
        }
    }
    else if (space == MODEL_SPACE)
    {
        Variable& normal = vertexData["normalModel"];
        if (!normal.isCalculated())
        {
            normal.setCalculated();
            shadergen.emitLine(stage, normal.getFullName() + " = i_normal");
        }
    }
    else
    {
        Variable& normal = vertexData["normalObject"];
        if (!normal.isCalculated())
        {
            normal.setCalculated();
            shadergen.emitLine(stage, normal.getFullName() + " = i_normal");
        }
    }
END_SHADER_STAGE(shader, HW::VERTEX_STAGE)

BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    if (space == WORLD_SPACE)
    {
        shadergen.emitString(stage, " = normalize(" + vertexData["normalWorld"].getFullName() + ")");
    }
    else if (space == MODEL_SPACE)
    {
        shadergen.emitString(stage, " = normalize(" + vertexData["normalModel"].getFullName() + ")");
    }
    else
    {
        shadergen.emitString(stage, " = normalize(" + vertexData["normalObject"].getFullName() + ")");
    }
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
