#include <MaterialXGenGlsl/Nodes/LightSamplerNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr LightSamplerNodeGlsl::create()
{
    return std::make_shared<LightSamplerNodeGlsl>();
}

void LightSamplerNodeGlsl::emitFunctionDefinition(ShaderStage& stage, const ShaderNode&, const ShaderGenerator& shadergen_, GenContext& context) const
{
    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(shadergen_);

        // Emit light sampler function with all bound light types
        shadergen.emitLine(stage, "void sampleLightSource(LightData light, vec3 position, out lightshader result)", false);
        shadergen.emitScopeBegin(stage, ShaderStage::Brackets::BRACES);
        shadergen.emitLine(stage, "result.intensity = vec3(0.0)");

        HwLightShadersPtr lightShaders = context.getUserData<HwLightShaders>(HW::USER_DATA_LIGHT_SHADERS);
        if (lightShaders)
        {
            string ifstatement = "if ";
            for (auto it : lightShaders->get())
            {
                shadergen.emitLine(stage, ifstatement + "(light.type == " + std::to_string(it.first) + ")", false);
                shadergen.emitScopeBegin(stage, ShaderStage::Brackets::BRACES);
                shadergen.emitFunctionCall(stage, context, *it.second, false);
                shadergen.emitScopeEnd(stage);
                ifstatement = "else if ";
            }
        }

        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);
    END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
