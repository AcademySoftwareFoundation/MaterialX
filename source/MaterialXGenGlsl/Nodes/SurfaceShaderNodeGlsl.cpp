#include <MaterialXGenGlsl/Nodes/SurfaceShaderNodeGlsl.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

ShaderNodeImplPtr SurfaceShaderNodeGlsl::create()
{
    return std::make_shared<SurfaceShaderNodeGlsl>();
}

const string& SurfaceShaderNodeGlsl::getLanguage() const
{
    return GlslShaderGenerator::LANGUAGE;
}

const string& SurfaceShaderNodeGlsl::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}

void SurfaceShaderNodeGlsl::createVariables(Shader& shader, GenContext&, const ShaderGenerator&, const ShaderNode&) const
{
    // TODO: 
    // The surface shader needs position, view position and light sources. We should solve this by adding some 
    // dependency mechanism so this implementation can be set to depend on the PositionNodeGlsl,  
    // ViewDirectionNodeGlsl and LightNodeGlsl nodes instead? This is where the MaterialX attribute "internalgeomprops" 
    // is needed.
    //
    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(vs, HW::VERTEX_INPUTS, Type::VECTOR3, "i_position");
    addStageConnector(vs, ps, HW::VERTEX_DATA, Type::VECTOR3, "positionWorld");

    addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::VECTOR3, "u_viewPosition");
    addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::INTEGER, "u_numActiveLightSources", Value::createValue<int>(0));
}

void SurfaceShaderNodeGlsl::emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const
{
    BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        ShaderPort* position = vertexData["positionWorld"];
        if (!position->isEmitted())
        {
            position->setEmitted();
            shadergen.emitLine(stage, prefix + position->getVariable() + " = hPositionWorld.xyz");
        }
    END_SHADER_STAGE(shader, HW::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        SourceCodeNode::emitFunctionCall(stage, context, shadergen, node);
    END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
