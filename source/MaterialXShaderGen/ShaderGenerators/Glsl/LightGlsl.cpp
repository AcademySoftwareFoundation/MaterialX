#include <MaterialXShaderGen/ShaderGenerators/Glsl/LightGlsl.h>

namespace MaterialX
{

namespace
{
    static const string LIGHT_DIRECTION_BLOCK =
        "vec3 L = light.position - position;\n"
        "float distance = length(L);\n"
        "L /= distance;\n"
        "result.direction = L;\n";
}

SgImplementationPtr LightGlsl::create()
{
    return std::make_shared<LightGlsl>();
}

void LightGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    // Create uniform for intensity, exposure and direction
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::LIGHT_DATA_BLOCK, DataType::FLOAT, "intensity",
        EMPTY_STRING, Value::createValue<float>(1.0f));
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::LIGHT_DATA_BLOCK, DataType::FLOAT, "exposure",
        EMPTY_STRING, Value::createValue<float>(0.0f));
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::LIGHT_DATA_BLOCK, DataType::VECTOR3, "direction",
        EMPTY_STRING, Value::createValue<Vector3>(Vector3(0.0f,1.0f,0.0f)));

    // Create uniform for number of active light sources
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, DataType::INTEGER, "u_numActiveLightSources",
        EMPTY_STRING, Value::createValue<int>(0));
}

void LightGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen_, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);
    GlslShaderGenerator& shadergen = static_cast<GlslShaderGenerator&>(shadergen_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    shader.addBlock(LIGHT_DIRECTION_BLOCK);
    shader.newLine();

    string emission;
    shadergen.emitEdfNodes(node, "light.direction", "-L", shader, emission);
    shader.newLine();

    shader.addComment("Apply quadratic falloff and adjust intensity");
    shader.addLine("result.intensity = " + emission + " / (distance * distance)");

    const SgInput* intensity = node.getInput("intensity");
    const SgInput* exposure = node.getInput("exposure");

    shader.beginLine();
    shader.addStr("result.intensity *= ");
    shadergen.emitInput(intensity, shader);
    shader.endLine();

    // Emit exposure adjustment only if it matters
    if (exposure->connection || (exposure->value && exposure->value->asA<float>() != 0.0f))
    {
        shader.beginLine();
        shader.addStr("result.intensity *= pow(2, ");
        shadergen.emitInput(exposure, shader);
        shader.addStr(")");
        shader.endLine();
    }

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
