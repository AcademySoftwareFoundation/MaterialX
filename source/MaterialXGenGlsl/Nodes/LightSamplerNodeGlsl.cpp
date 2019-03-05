#include <MaterialXGenGlsl/Nodes/LightSamplerNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr LightSamplerNodeGlsl::create()
{
    return std::make_shared<LightSamplerNodeGlsl>();
}

void LightSamplerNodeGlsl::emitFunctionDefinition(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        // Emit light sampler function with all bound light types
        shadergen.emitLine("void sampleLightSource(LightData light, vec3 position, out lightshader result)", stage, false);
        shadergen.emitScopeBegin(stage, ShaderStage::Brackets::BRACES);
        shadergen.emitLine("result.intensity = vec3(0.0)", stage);

        HwLightShadersPtr lightShaders = context.getUserData<HwLightShaders>(HW::USER_DATA_LIGHT_SHADERS);
        if (lightShaders)
        {
            string ifstatement = "if ";
            for (auto it : lightShaders->get())
            {
                shadergen.emitLine(ifstatement + "(light.type == " + std::to_string(it.first) + ")", stage, false);
                shadergen.emitScopeBegin(stage, ShaderStage::Brackets::BRACES);
                shadergen.emitFunctionCall(*it.second, context, stage, false);
                shadergen.emitScopeEnd(stage);
                ifstatement = "else if ";
            }
        }

        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);
    END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
