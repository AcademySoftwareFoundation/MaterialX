//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/Nodes/WgslLightNodes.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXGenHw/HwLightShaders.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

// ============== WgslLightSamplerNode ==============

namespace
{
const string WGSL_SAMPLE_LIGHTS_FUNC_SIGNATURE = "fn sampleLightSource(light: LightData, position: vec3f, result: ptr<function, lightshader>)";
const string WGSL_NUM_LIGHTS_FUNC_SIGNATURE = "fn numActiveLightSources() -> i32";
}

WgslLightSamplerNode::WgslLightSamplerNode()
{
    _hash = std::hash<string>{}(WGSL_SAMPLE_LIGHTS_FUNC_SIGNATURE);
}

ShaderNodeImplPtr WgslLightSamplerNode::create()
{
    return std::make_shared<WgslLightSamplerNode>();
}

void WgslLightSamplerNode::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const Syntax& syntax = shadergen.getSyntax();
        const string vec3_zero = syntax.getValue(Type::VECTOR3, HW::VEC3_ZERO);

        shadergen.emitLine(WGSL_SAMPLE_LIGHTS_FUNC_SIGNATURE, stage, false);
        shadergen.emitFunctionBodyBegin(node, context, stage);
        shadergen.emitLine("(*result).intensity = " + vec3_zero, stage);
        shadergen.emitLine("(*result).direction = " + vec3_zero, stage);

        HwLightShadersPtr lightShaders = context.getUserData<HwLightShaders>(HW::USER_DATA_LIGHT_SHADERS);
        if (lightShaders)
        {
            string ifstatement = "if ";
            for (const auto& it : lightShaders->get())
            {
                shadergen.emitLine(ifstatement + "(light." + shadergen.getLightDataTypevarString() + " == " + std::to_string(it.first) + ")", stage, false);
                shadergen.emitScopeBegin(stage);
                shadergen.emitFunctionCall(*it.second, context, stage);
                shadergen.emitScopeEnd(stage);
                ifstatement = "else if ";
            }
        }

        shadergen.emitFunctionBodyEnd(node, context, stage);
    }
}

// ============== WgslLightCompoundNode ==============

ShaderNodeImplPtr WgslLightCompoundNode::create()
{
    return std::make_shared<WgslLightCompoundNode>();
}

void WgslLightCompoundNode::emitFunctionDefinition(const ShaderNode& /*node*/, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());
        const Syntax& syntax = shadergen.getSyntax();
        const string vec3_zero = syntax.getValue(Type::VECTOR3, HW::VEC3_ZERO);

        shadergen.emitFunctionDefinitions(*_rootGraph, context, stage);

        shadergen.emitLine("fn " + _functionName + "(light: LightData, position: vec3f, result: ptr<function, lightshader>)", stage, false);

        shadergen.emitFunctionBodyBegin(*_rootGraph, context, stage);

        shadergen.emitFunctionCalls(*_rootGraph, context, stage, ShaderNode::Classification::TEXTURE);

        shadergen.emitLine("var closureData: ClosureData = makeClosureData(CLOSURE_TYPE_EMISSION, " + vec3_zero + ", -L, light.direction, " + vec3_zero + ", 0.0)", stage);
        shadergen.emitFunctionCalls(*_rootGraph, context, stage, ShaderNode::Classification::SHADER | ShaderNode::Classification::LIGHT);

        shadergen.emitFunctionBodyEnd(*_rootGraph, context, stage);
    }
}

void WgslLightCompoundNode::emitFunctionCall(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitLine(_functionName + "(light, position, result)", stage);
    }
}

// ============== WgslNumLightsNode ==============

WgslNumLightsNode::WgslNumLightsNode()
{
    _hash = std::hash<string>{}(WGSL_NUM_LIGHTS_FUNC_SIGNATURE);
}

ShaderNodeImplPtr WgslNumLightsNode::create()
{
    return std::make_shared<WgslNumLightsNode>();
}

void WgslNumLightsNode::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitLine(WGSL_NUM_LIGHTS_FUNC_SIGNATURE, stage, false);
        shadergen.emitFunctionBodyBegin(node, context, stage);
        shadergen.emitLine("return min(" + HW::T_NUM_ACTIVE_LIGHT_SOURCES + ", " + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + ")", stage);
        shadergen.emitFunctionBodyEnd(node, context, stage);
    }
}

MATERIALX_NAMESPACE_END
