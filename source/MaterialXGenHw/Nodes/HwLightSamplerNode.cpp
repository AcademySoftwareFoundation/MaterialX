//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHw/Nodes/HwLightSamplerNode.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenHw/HwLightShaders.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

const string SAMPLE_LIGHTS_FUNC_SIGNATURE = "void sampleLightSource(LightData light, vec3 position, out lightshader result)";

} // anonymous namespace

HwLightSamplerNode::HwLightSamplerNode()
{
    _hash = std::hash<string>{}(SAMPLE_LIGHTS_FUNC_SIGNATURE);
}

ShaderNodeImplPtr HwLightSamplerNode::create()
{
    return std::make_shared<HwLightSamplerNode>();
}

void HwLightSamplerNode::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const Syntax& syntax = shadergen.getSyntax();

        const string& vec3 = syntax.getTypeName(Type::VECTOR3);
        const string& out_lightshader = syntax.getOutputTypeName(Type::LIGHTSHADER);
        const string vec3_zero = syntax.getValue(Type::VECTOR3, HW::VEC3_ZERO);

        // Emit light sampler function with all bound light types
        shadergen.emitLine("void sampleLightSource(LightData light, "+vec3+" position, "+out_lightshader+" result)", stage, false);
        shadergen.emitFunctionBodyBegin(node, context, stage);
        shadergen.emitLine("result.intensity = "+vec3_zero, stage);
        shadergen.emitLine("result.direction = "+vec3_zero, stage);

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

MATERIALX_NAMESPACE_END
