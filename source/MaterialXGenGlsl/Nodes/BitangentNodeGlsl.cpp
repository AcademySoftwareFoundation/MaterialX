#include <MaterialXGenGlsl/Nodes/BitangentNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr BitangentNodeGlsl::create()
{
    return std::make_shared<BitangentNodeGlsl>();
}

void BitangentNodeGlsl::createVariables(Shader& shader, const ShaderNode& node, const ShaderGenerator&, GenContext&) const
{
    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(vs, HW::VERTEX_INPUTS, Type::VECTOR3, "i_bitangent");

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->value->asA<int>() : -1;
    if (space == WORLD_SPACE)
    {
        const string& path = spaceInput ? spaceInput->path : EMPTY_STRING;
        addStageUniform(vs, HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix", EMPTY_STRING, nullptr, path);
        addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "bitangentWorld");
    }
    else if (space == MODEL_SPACE)
    {
        addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "bitangentModel");
    }
    else
    {
        addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "bitangentObject");
    }
}

void BitangentNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const
{
    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->value->asA<int>() : -1;

BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
    VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    if (space == WORLD_SPACE)
    {
        Variable& bitangent = vertexData["bitangentWorld"];
        if (!bitangent.isCalculated())
        {
            bitangent.setCalculated();
            shadergen.emitLine(stage, bitangent.getFullName() + " = (u_worldInverseTransposeMatrix * vec4(i_bitangent,0.0)).xyz");
        }
    }
    else if (space == MODEL_SPACE)
    {
        Variable& bitangent = vertexData["bitangentModel"];
        if (!bitangent.isCalculated())
        {
            bitangent.setCalculated();
            shadergen.emitLine(stage, bitangent.getFullName() + " = i_bitangent");
        }
    }
    else
    {
        Variable& bitangent = vertexData["bitangentObject"];
        if (!bitangent.isCalculated())
        {
            bitangent.setCalculated();
            shadergen.emitLine(stage, bitangent.getFullName() + " = i_bitangent");
        }
    }
END_SHADER_STAGE(stage, HW::VERTEX_STAGE)

BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    if (space == WORLD_SPACE)
    {
        const Variable& bitangent = vertexData["bitangentWorld"];
        shadergen.emitString(stage, " = normalize(" + bitangent.getFullName() + ")");
    }
    else if (space == MODEL_SPACE)
    {
        const Variable& bitangent = vertexData["bitangentModel"];
        shadergen.emitString(stage, " = normalize(" + bitangent.getFullName() + ")");
    }
    else
    {
        const Variable& bitangent = vertexData["bitangentObject"];
        shadergen.emitString(stage, " = normalize(" + bitangent.getFullName() + ")");
    }
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
