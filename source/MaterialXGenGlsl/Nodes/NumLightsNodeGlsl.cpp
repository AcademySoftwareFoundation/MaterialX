#include <MaterialXGenGlsl/Nodes/NumLightsNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr NumLightsNodeGlsl::create()
{
    return std::make_shared<NumLightsNodeGlsl>();
}

void NumLightsNodeGlsl::createVariables(Shader& shader, GenContext&, const ShaderGenerator&, const ShaderNode&) const
{
    // Create uniform for number of active light sources
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);
    addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::INTEGER, "u_numActiveLightSources", Value::createValue<int>(0));
}

void NumLightsNodeGlsl::emitFunctionDefinition(ShaderStage& stage, GenContext&, const ShaderGenerator& shadergen, const ShaderNode&) const
{
    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
        shadergen.emitLine(stage, "int numActiveLightSources()", false);
        shadergen.emitScopeBegin(stage, ShaderStage::Brackets::BRACES);
        shadergen.emitLine(stage, "return min(u_numActiveLightSources, MAX_LIGHT_SOURCES)");
        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);
    END_SHADER_STAGE(shader, HW::PIXEL_STAGE)
}

} // namespace MaterialX
