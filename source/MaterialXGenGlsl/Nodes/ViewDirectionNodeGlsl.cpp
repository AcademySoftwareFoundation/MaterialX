//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/ViewDirectionNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

ShaderNodeImplPtr ViewDirectionNodeGlsl::create()
{
    return std::make_shared<ViewDirectionNodeGlsl>();
}

void ViewDirectionNodeGlsl::createVariables(const ShaderNode&, GenContext&, Shader& shader) const
{
    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, HW::T_IN_POSITION, vs);
    addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, HW::T_POSITION_WORLD, vs, ps);
    addStageUniform(HW::PRIVATE_UNIFORMS, Type::VECTOR3, HW::T_VIEW_POSITION, ps);
}

void ViewDirectionNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        ShaderPort* position = vertexData[HW::T_POSITION_WORLD];
        if (!position->isEmitted())
        {
            position->setEmitted();
            shadergen.emitLine(prefix + position->getVariable() + " = hPositionWorld.xyz", stage);
        }
    END_SHADER_STAGE(stage, Stage::VERTEX)

    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        ShaderPort* position = vertexData[HW::T_POSITION_WORLD];
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = normalize(" + prefix + position->getVariable() + " - " + HW::T_VIEW_POSITION + ")", stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
