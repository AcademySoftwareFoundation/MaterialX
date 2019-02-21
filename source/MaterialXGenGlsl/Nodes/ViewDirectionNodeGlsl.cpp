#include <MaterialXGenGlsl/Nodes/ViewDirectionNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr ViewDirectionNodeGlsl::create()
{
    return std::make_shared<ViewDirectionNodeGlsl>();
}

void ViewDirectionNodeGlsl::createVariables(Shader& shader, const ShaderNode&, const ShaderGenerator&, GenContext&) const
{
    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(vs, HW::VERTEX_INPUTS, Type::VECTOR3, "i_position");
    addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "positionWorld");
    addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::VECTOR3, "u_viewPosition");
}

void ViewDirectionNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const
{
BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
    VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    Variable& position = vertexData["positionWorld"];
    if (!position.isCalculated())
    {
        position.setCalculated();
        shadergen.emitLine(stage, position.getFullName() + " = hPositionWorld.xyz");
    }
END_SHADER_STAGE(stage, HW::VERTEX_STAGE)

BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    Variable& position = vertexData["positionWorld"];
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(stage, context, node.getOutput(), true, false);
    shadergen.emitString(stage, " = normalize(" + position.getFullName() + " - u_viewPosition)");
    shadergen.emitLineEnd(stage);
END_SHADER_STAGE(stage, HW::PIXEL_STAGE)
}

} // namespace MaterialX
