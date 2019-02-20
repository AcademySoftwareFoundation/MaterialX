#include <MaterialXGenGlsl/Nodes/LightNodeGlsl.h>

namespace MaterialX
{

namespace
{
    static const string LIGHT_DIRECTION_CALCULATION =
        "vec3 L = light.position - position;\n"
        "float distance = length(L);\n"
        "L /= distance;\n"
        "result.direction = L;\n";
}

ShaderNodeImplPtr LightNodeGlsl::create()
{
    return std::make_shared<LightNodeGlsl>();
}

void LightNodeGlsl::createVariables(Shader& shader, const ShaderNode&, ShaderGenerator&, GenContext&) const
{
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    // Create uniform for intensity, exposure and direction
    VariableBlock& lightUniforms = ps.getUniformBlock(HW::LIGHT_DATA);
    lightUniforms.add(Type::FLOAT, "intensity", EMPTY_STRING, Value::createValue<float>(1.0f));
    lightUniforms.add(Type::FLOAT, "exposure", EMPTY_STRING, Value::createValue<float>(0.0f));
    lightUniforms.add(Type::VECTOR3, "direction", EMPTY_STRING, Value::createValue<Vector3>(Vector3(0.0f,1.0f,0.0f)));

    // Create uniform for number of active light sources
    addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::INTEGER, "u_numActiveLightSources", 
                    EMPTY_STRING, Value::createValue<int>(0));
}

void LightNodeGlsl::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen_, GenContext& context) const
{
BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
    GlslShaderGenerator& shadergen = static_cast<GlslShaderGenerator&>(shadergen_);
    const ShaderGraph& graph = *node.getParent();

    shadergen.emitBlock(stage, LIGHT_DIRECTION_CALCULATION);
    shadergen.emitLineBreak(stage);

    string emission;
    shadergen.emitEdfNodes(stage, graph, context, node, "light.direction", "-L", emission);
    shadergen.emitLineBreak(stage);

    shadergen.emitComment(stage, "Apply quadratic falloff and adjust intensity");
    shadergen.emitLine(stage, "result.intensity = " + emission + " / (distance * distance)");

    const ShaderInput* intensity = node.getInput("intensity");
    const ShaderInput* exposure = node.getInput("exposure");

    shadergen.emitLineBegin(stage);
    shadergen.emitString(stage, "result.intensity *= ");
    shadergen.emitInput(stage, context, intensity);
    shadergen.emitLineEnd(stage);

    // Emit exposure adjustment only if it matters
    if (exposure->connection || (exposure->value && exposure->value->asA<float>() != 0.0f))
    {
        shadergen.emitLineBegin(stage);
        shadergen.emitString(stage, "result.intensity *= pow(2, ");
        shadergen.emitInput(stage, context, exposure);
        shadergen.emitString(stage, ")");
        shadergen.emitLineEnd(stage);
    }

    END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
