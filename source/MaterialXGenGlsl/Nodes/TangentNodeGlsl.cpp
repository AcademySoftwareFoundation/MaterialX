#include <MaterialXGenGlsl/Nodes/TangentNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr TangentNodeGlsl::create()
{
    return std::make_shared<TangentNodeGlsl>();
}

void TangentNodeGlsl::createVariables(Shader& shader, const ShaderNode& node, const ShaderGenerator&, GenContext&) const
{
    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(vs, HW::VERTEX_INPUTS, Type::VECTOR3, "i_tangent");

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : -1;
    if (space == WORLD_SPACE)
    {
        addStageUniform(vs, HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix");
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

void TangentNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const
{
    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : -1;

    BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        if (space == WORLD_SPACE)
        {
            ShaderPort* tangent = vertexData["tangentWorld"];
            if (!tangent->isEmitted())
            {
                tangent->setEmitted();
                shadergen.emitLine(stage, prefix + tangent->getVariable() + " = (u_worldInverseTransposeMatrix * vec4(i_tangent,0.0)).xyz");
            }
        }
        else if (space == MODEL_SPACE)
        {
            ShaderPort* tangent = vertexData["tangentModel"];
            if (!tangent->isEmitted())
            {
                tangent->setEmitted();
                shadergen.emitLine(stage, prefix + tangent->getVariable() + " = i_tangent");
            }
        }
        else
        {
            ShaderPort* tangent = vertexData["tangentObject"];
            if (!tangent->isEmitted())
            {
                tangent->setEmitted();
                shadergen.emitLine(stage, prefix + tangent->getVariable() + " = i_tangent");
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
            const ShaderPort* tangent = vertexData["tangentWorld"];
            shadergen.emitString(stage, " = normalize(" + prefix + tangent->getVariable() + ")");
        }
        else if (space == MODEL_SPACE)
        {
            const ShaderPort* tangent = vertexData["tangentModel"];
            shadergen.emitString(stage, " = normalize(" + prefix + tangent->getVariable() + ")");
        }
        else
        {
            const ShaderPort* tangent = vertexData["tangentObject"];
            shadergen.emitString(stage, " = normalize(" + prefix + tangent->getVariable() + ")");
        }
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
