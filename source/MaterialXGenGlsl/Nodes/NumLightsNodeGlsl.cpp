#include <MaterialXGenGlsl/Nodes/NumLightsNodeGlsl.h>

namespace MaterialX
{

ShaderNodeImplPtr NumLightsNodeGlsl::create()
{
    return std::make_shared<NumLightsNodeGlsl>();
}

void NumLightsNodeGlsl::createVariables(Shader& shader, const ShaderNode&, const ShaderGenerator&, GenContext&) const
{
    // Create uniform for number of active light sources
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);
    addStageUniform(ps, HW::PRIVATE_UNIFORMS, Type::INTEGER, "u_numActiveLightSources",
        EMPTY_STRING, Value::createValue<int>(0));
}

void NumLightsNodeGlsl::emitFunctionDefinition(ShaderStage& stage, const ShaderNode&, const ShaderGenerator& shadergen, GenContext&) const
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
