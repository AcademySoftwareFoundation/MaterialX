#include <MaterialXGenGlsl/Nodes/TangentNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr TangentNodeGlsl::create()
{
    return std::make_shared<TangentNodeGlsl>();
}

void TangentNodeGlsl::createVariables(Shader& shader, const ShaderNode& node, ShaderGenerator&, GenContext&) const
{
    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(vs, HW::VERTEX_INPUTS, Type::VECTOR3, "i_tangent");

    const ShaderInput* spaceInput = node.getInput(SPACE);
        const int space = spaceInput ? spaceInput->value->asA<int>() : -1;
    const string& path = spaceInput ? spaceInput->path : EMPTY_STRING;
    if (space == WORLD_SPACE)
    {
        addStageUniform(vs, HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix", EMPTY_STRING, nullptr, path);
        addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "tangentWorld");
    }
    else if (space == MODEL_SPACE)
    {
        addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "tangentModel");
    }
    else
    {
        addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "tangentObject");
    }
}

void TangentNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context) const
{
    const ShaderInput* spaceInput = node.getInput(SPACE);
        const int space = spaceInput ? spaceInput->value->asA<int>() : -1;

BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
    VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    if (space == WORLD_SPACE)
    {
        Variable& tangent = vertexData["tangentWorld"];
        if (!tangent.isCalculated())
        {
            tangent.setCalculated();
            shadergen.emitLine(stage, tangent.getFullName() + " = (u_worldInverseTransposeMatrix * vec4(i_tangent,0.0)).xyz");
        }
    }
    else if (space == MODEL_SPACE)
    {
        Variable& tangent = vertexData["tangentModel"];
        if (!tangent.isCalculated())
        {
            tangent.setCalculated();
            shadergen.emitLine(stage, tangent.getFullName() + " = i_tangent");
        }
    }
    else
    {
        Variable& tangent = vertexData["tangentObject"];
        if (!tangent.isCalculated())
        {
            tangent.setCalculated();
            shadergen.emitLine(stage, tangent.getFullName() + " = i_tangent");
        }
    }
END_SHADER_STAGE(shader, HW::VERTEX_STAGE)

BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    if (space == WORLD_SPACE)
    {
        shadergen.emitString(stage, " = normalize(" + vertexData["tangentWorld"].getFullName() + ")");
    }
    else if (space == MODEL_SPACE)
    {
        shadergen.emitString(stage, " = normalize(" + vertexData["tangentModel"].getFullName() + ")");
    }
    else
    {
        shadergen.emitString(stage, " = normalize(" + vertexData["tangentObject"].getFullName() + ")");
    }
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
