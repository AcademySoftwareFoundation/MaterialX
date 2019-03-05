#include <MaterialXGenGlsl/Nodes/NormalNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr NormalNodeGlsl::create()
{
    return std::make_shared<NormalNodeGlsl>();
}

void NormalNodeGlsl::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, "i_normal", vs);

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : -1;
    if (space == WORLD_SPACE)
    {
        addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix", vs);
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, "normalWorld", vs, ps);
    }
    else if (space == MODEL_SPACE)
    {
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, "normalModel", vs, ps);
    }
    else
    {
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, "normalObject", vs, ps);
    }
}

void NormalNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

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
                shadergen.emitLine(prefix + normal->getVariable() + " = (u_worldInverseTransposeMatrix * vec4(i_normal,0.0)).xyz", stage);
            }
        }
        else if (space == MODEL_SPACE)
        {
            ShaderPort* normal = vertexData["normalModel"];
            if (!normal->isEmitted())
            {
                normal->setEmitted();
                shadergen.emitLine(prefix + normal->getVariable() + " = i_normal", stage);
            }
        }
        else
        {
            ShaderPort* normal = vertexData["normalObject"];
            if (!normal->isEmitted())
            {
                normal->setEmitted();
                shadergen.emitLine(prefix + normal->getVariable() + " = i_normal", stage);
            }
        }
    END_SHADER_STAGE(shader, HW::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        if (space == WORLD_SPACE)
        {
            const ShaderPort* normal = vertexData["normalWorld"];
            shadergen.emitString(" = normalize(" + prefix + normal->getVariable() + ")", stage);
        }
        else if (space == MODEL_SPACE)
        {
            const ShaderPort* normal = vertexData["normalModel"];
            shadergen.emitString(" = normalize(" + prefix + normal->getVariable() + ")", stage);
        }
        else
        {
            const ShaderPort* normal = vertexData["normalObject"];
            shadergen.emitString(" = normalize(" + prefix + normal->getVariable() + ")", stage);
        }
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
