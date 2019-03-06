//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/ViewDirectionNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr ViewDirectionNodeGlsl::create()
{
    return std::make_shared<ViewDirectionNodeGlsl>();
}

void ViewDirectionNodeGlsl::createVariables(const ShaderNode&, GenContext&, Shader& shader) const
{
    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, "i_position", vs);
    addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, "positionWorld", vs, ps);
    addStageUniform(HW::PRIVATE_UNIFORMS, Type::VECTOR3, "u_viewPosition", ps);
}

void ViewDirectionNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        ShaderPort* position = vertexData["positionWorld"];
        if (!position->isEmitted())
        {
            position->setEmitted();
            shadergen.emitLine(prefix + position->getVariable() + " = hPositionWorld.xyz", stage);
        }
    END_SHADER_STAGE(stage, HW::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        ShaderPort* position = vertexData["positionWorld"];
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = normalize(" + prefix + position->getVariable() + " - u_viewPosition)", stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(stage, HW::PIXEL_STAGE)
}

} // namespace MaterialX
