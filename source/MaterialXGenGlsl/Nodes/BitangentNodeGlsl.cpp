#include <MaterialXGenGlsl/Nodes/BitangentNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr BitangentNodeGlsl::create()
{
    return std::make_shared<BitangentNodeGlsl>();
}

void BitangentNodeGlsl::createVariables(Shader& shader, GenContext&, const ShaderGenerator&, const ShaderNode& node) const
{
    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(vs, HW::VERTEX_INPUTS, Type::VECTOR3, "i_bitangent");

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : -1;
    if (space == WORLD_SPACE)
    {
        addStageUniform(vs, HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix");
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

void BitangentNodeGlsl::emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const
{
    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : -1;

BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
    VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    const string prefix = vertexData.getInstance() + ".";
    if (space == WORLD_SPACE)
    {
        ShaderPort* bitangent = vertexData["bitangentWorld"];
        if (!bitangent->isEmitted())
        {
            bitangent->setEmitted();
            shadergen.emitLine(stage, prefix + bitangent->getVariable() + " = (u_worldInverseTransposeMatrix * vec4(i_bitangent,0.0)).xyz");
        }
    }
    else if (space == MODEL_SPACE)
    {
        ShaderPort* bitangent = vertexData["bitangentModel"];
        if (!bitangent->isEmitted())
        {
            bitangent->setEmitted();
            shadergen.emitLine(stage, prefix + bitangent->getVariable() + " = i_bitangent");
        }
    }
    else
    {
        ShaderPort* bitangent = vertexData["bitangentObject"];
        if (!bitangent->isEmitted())
        {
            bitangent->setEmitted();
            shadergen.emitLine(stage, prefix + bitangent->getVariable() + " = i_bitangent");
        }
    }
END_SHADER_STAGE(stage, HW::VERTEX_STAGE)

BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    const string prefix = vertexData.getInstance() + ".";
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    if (space == WORLD_SPACE)
    {
        const ShaderPort* bitangent = vertexData["bitangentWorld"];
        shadergen.emitString(stage, " = normalize(" + prefix + bitangent->getVariable() + ")");
    }
    else if (space == MODEL_SPACE)
    {
        const ShaderPort* bitangent = vertexData["bitangentModel"];
        shadergen.emitString(stage, " = normalize(" + prefix + bitangent->getVariable() + ")");
    }
    else
    {
        const ShaderPort* bitangent = vertexData["bitangentObject"];
        shadergen.emitString(stage, " = normalize(" + prefix + bitangent->getVariable() + ")");
    }
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
