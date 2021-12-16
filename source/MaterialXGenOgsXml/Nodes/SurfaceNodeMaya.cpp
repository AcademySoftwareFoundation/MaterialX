//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOgsXml/Nodes/SurfaceNodeMaya.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

namespace {
    const string MX_MAYA_EXTERNAL_LIGHTS = "mayaExternalLightFunctions";
}

SurfaceNodeMaya::SurfaceNodeMaya() : SurfaceNodeGlsl()
{
}

ShaderNodeImplPtr SurfaceNodeMaya::create()
{
    return std::make_shared<SurfaceNodeMaya>();
}

void SurfaceNodeMaya::createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const
{
    SurfaceNodeGlsl::createVariables(node, context, shader);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    addStageUniform(HW::PRIVATE_UNIFORMS, Type::INTEGER, MX_MAYA_EXTERNAL_LIGHTS, ps);
}

void SurfaceNodeMaya::emitLightLoop(const ShaderNode& node, GenContext& context, ShaderStage& stage, const string& outColor) const
{
    const GlslShaderGenerator& shadergen = static_cast<const GlslShaderGenerator&>(context.getShaderGenerator());

    const ShaderInput* bsdfInput = node.getInput("bsdf");
    const ShaderNode* bsdf = bsdfInput->getConnectedSibling();

    shadergen.emitComment("Light loop", stage);
    shadergen.emitLine("int numLights = mayaGetNumLights()", stage);
    shadergen.emitLine("irradiance lightShader", stage);
    shadergen.emitLine("for (int activeLightIndex = 0; activeLightIndex < numLights; ++activeLightIndex)", stage, false);

    shadergen.emitScopeBegin(stage);

    shadergen.emitLine("lightShader = mayaGetLightIrradiance(activeLightIndex, P, N, V)", stage);
    shadergen.emitLine("vec3 L = lightShader.Ld", stage);
    shadergen.emitLineBreak(stage);

    shadergen.emitComment("Calculate the BSDF response for this light source", stage);
    context.pushClosureContext(&_callReflection);
    shadergen.emitFunctionCall(*bsdf, context, stage);
    context.popClosureContext();

    shadergen.emitComment("Accumulate the light's contribution", stage);
    shadergen.emitLine(outColor + " += lightShader.specularI * " + bsdf->getOutput()->getVariable() + ".response", stage);

    shadergen.emitScopeEnd(stage);
    shadergen.emitLineBreak(stage);
}

MATERIALX_NAMESPACE_END
