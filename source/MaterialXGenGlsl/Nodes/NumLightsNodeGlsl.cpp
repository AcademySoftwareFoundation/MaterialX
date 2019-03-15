//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/NumLightsNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

namespace
{
    const string NUM_LIGHTS_FUNC_SIGNATURE = "int numActiveLightSources()";
}

NumLightsNodeGlsl::NumLightsNodeGlsl()
{
    _hash = std::hash<string>{}(NUM_LIGHTS_FUNC_SIGNATURE);
}

ShaderNodeImplPtr NumLightsNodeGlsl::create()
{
    return std::make_shared<NumLightsNodeGlsl>();
}

void NumLightsNodeGlsl::createVariables(const ShaderNode&, GenContext&, Shader& shader) const
{
    // Create uniform for number of active light sources
    ShaderStage& ps = shader.getStage(Stage::PIXEL);
    ShaderPort* numActiveLights = addStageUniform(HW::PRIVATE_UNIFORMS, Type::INTEGER, "u_numActiveLightSources", ps);
    numActiveLights->setValue(Value::createValue<int>(0));
}

void NumLightsNodeGlsl::emitFunctionDefinition(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitLine(NUM_LIGHTS_FUNC_SIGNATURE, stage, false);
        shadergen.emitScopeBegin(stage);
        shadergen.emitLine("return min(u_numActiveLightSources, MAX_LIGHT_SOURCES)", stage);
        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

} // namespace MaterialX
