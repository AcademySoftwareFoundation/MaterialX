//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/Nodes/WgslSurfaceNode.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr WgslSurfaceNode::create()
{
    return std::make_shared<WgslSurfaceNode>();
}

void WgslSurfaceNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());
    const Syntax& syntax = shadergen.getSyntax();

    const string& vec3 = syntax.getTypeName(Type::VECTOR3);
    const string vec3_zero = syntax.getValue(Type::VECTOR3, HW::VEC3_ZERO);
    const string vec3_one = syntax.getValue(Type::VECTOR3, HW::VEC3_ONE);

    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* position = vertexData[HW::T_POSITION_WORLD];
        if (!position->isEmitted())
        {
            position->setEmitted();
            shadergen.emitLine(prefix + position->getVariable() + " = hPositionWorld.xyz", stage);
        }
        ShaderPort* normal = vertexData[HW::T_NORMAL_WORLD];
        if (!normal->isEmitted())
        {
            normal->setEmitted();
            shadergen.emitLine(prefix + normal->getVariable() + " = normalize((" + HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX + " * " + syntax.getTypeName(Type::VECTOR4) + "(" + HW::T_IN_NORMAL + ", 0.0)).xyz)", stage);
        }
        if (context.getOptions().hwAmbientOcclusion)
        {
            ShaderPort* texcoord = vertexData[HW::T_TEXCOORD + "_0"];
            if (!texcoord->isEmitted())
            {
                texcoord->setEmitted();
                shadergen.emitLine(prefix + texcoord->getVariable() + " = " + HW::T_IN_TEXCOORD + "_0", stage);
            }
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);

        const ShaderOutput* output = node.getOutput();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(output, true, true, context, stage);
        shadergen.emitLineEnd(stage);

        shadergen.emitScopeBegin(stage);

        shadergen.emitLine("var N: " + vec3 + " = normalize(" + prefix + HW::T_NORMAL_WORLD + ")", stage);
        shadergen.emitLine("var V: " + vec3 + " = normalize(" + HW::T_VIEW_POSITION + " - " + prefix + HW::T_POSITION_WORLD + ")", stage);
        shadergen.emitLine("var P: " + vec3 + " = " + prefix + HW::T_POSITION_WORLD, stage);
        shadergen.emitLine("var L: " + vec3 + " = " + vec3_zero, stage);
        shadergen.emitLine("var occlusion: f32 = 1.0", stage);
        shadergen.emitLineBreak(stage);

        const string outColor = output->getVariable() + ".color";
        const string outTransparency = output->getVariable() + ".transparency";

        const ShaderInput* bsdfInput = node.getInput("bsdf");
        if (const ShaderNode* bsdf = bsdfInput->getConnectedSibling())
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitString("var surfaceOpacity: f32 = ", stage);
            shadergen.emitInput(node.getInput("opacity"), context, stage);
            shadergen.emitLineEnd(stage);
            shadergen.emitLineBreak(stage);

            emitLightLoop(node, context, stage, outColor);

            shadergen.emitComment("Ambient occlusion", stage);
            shadergen.emitLine("occlusion = 1.0", stage);
            shadergen.emitLineBreak(stage);

            shadergen.emitComment("Add environment contribution", stage);
            shadergen.emitScopeBegin(stage);

            if (bsdf->hasClassification(ShaderNode::Classification::BSDF_R))
            {
                shadergen.emitLine("var closureData: ClosureData = makeClosureData(CLOSURE_TYPE_INDIRECT, L, V, N, P, occlusion)", stage);
                shadergen.emitFunctionCall(*bsdf, context, stage);
            }
            else
            {
                shadergen.emitLineBegin(stage);
                shadergen.emitOutput(bsdf->getOutput(), true, true, context, stage);
                shadergen.emitLineEnd(stage);
            }

            shadergen.emitLineBreak(stage);
            shadergen.emitLine(outColor + " += occlusion * " + bsdf->getOutput()->getVariable() + ".response", stage);
            shadergen.emitScopeEnd(stage);
            shadergen.emitLineBreak(stage);
        }

        const ShaderInput* edfInput = node.getInput("edf");
        if (const ShaderNode* edf = edfInput->getConnectedSibling())
        {
            shadergen.emitComment("Add surface emission", stage);
            shadergen.emitScopeBegin(stage);

            if (edf->hasClassification(ShaderNode::Classification::EDF))
            {
                shadergen.emitLine("var closureData: ClosureData = makeClosureData(CLOSURE_TYPE_EMISSION, L, V, N, P, occlusion)", stage);
                shadergen.emitFunctionCall(*edf, context, stage);
            }
            else
            {
                shadergen.emitLineBegin(stage);
                shadergen.emitOutput(edf->getOutput(), true, true, context, stage);
                shadergen.emitLineEnd(stage);
            }

            shadergen.emitLine(outColor + " += " + edf->getOutput()->getVariable(), stage);
            shadergen.emitScopeEnd(stage);
            shadergen.emitLineBreak(stage);
        }

        if (const ShaderNode* bsdf = bsdfInput->getConnectedSibling())
        {
            shadergen.emitComment("Calculate the BSDF transmission for viewing direction", stage);
            if (bsdf->hasClassification(ShaderNode::Classification::BSDF_T) || bsdf->hasClassification(ShaderNode::Classification::VDF))
            {
                shadergen.emitLine("var closureData: ClosureData = makeClosureData(CLOSURE_TYPE_TRANSMISSION, L, V, N, P, occlusion)", stage);
                shadergen.emitFunctionCall(*bsdf, context, stage);
            }
            else
            {
                shadergen.emitLineBegin(stage);
                shadergen.emitOutput(bsdf->getOutput(), true, true, context, stage);
                shadergen.emitLineEnd(stage);
            }

            if (context.getOptions().hwTransmissionRenderMethod == TRANSMISSION_REFRACTION)
            {
                shadergen.emitLine(outColor + " += " + bsdf->getOutput()->getVariable() + ".response", stage);
            }
            else
            {
                shadergen.emitLine(outTransparency + " += " + bsdf->getOutput()->getVariable() + ".response", stage);
            }

            shadergen.emitLineBreak(stage);
            shadergen.emitComment("Compute and apply surface opacity", stage);
            shadergen.emitScopeBegin(stage);
            shadergen.emitLine(outColor + " *= surfaceOpacity", stage);
            shadergen.emitLine(outTransparency + " = mix(" + vec3_one + ", " + outTransparency + ", surfaceOpacity)", stage);
            shadergen.emitScopeEnd(stage);
        }

        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);
    }
}

void WgslSurfaceNode::emitLightLoop(const ShaderNode& node, GenContext& context, ShaderStage& stage, const string& outColor) const
{
    if (context.getOptions().hwMaxActiveLightSources <= 0)
        return;

    const HwShaderGenerator& shadergen = static_cast<const HwShaderGenerator&>(context.getShaderGenerator());
    const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    const string prefix = shadergen.getVertexDataPrefix(vertexData);

    const ShaderInput* bsdfInput = node.getInput("bsdf");
    const ShaderNode* bsdf = bsdfInput->getConnectedSibling();

    shadergen.emitComment("Light loop", stage);
    shadergen.emitLine("var numLights: i32 = numActiveLightSources()", stage);
    shadergen.emitLine("var lightShader: lightshader", stage);
    shadergen.emitLine("for (var activeLightIndex: i32 = 0; activeLightIndex < numLights; activeLightIndex += 1)", stage, false);

    shadergen.emitScopeBegin(stage);

    shadergen.emitLine("sampleLightSource(" + HW::T_LIGHT_DATA_INSTANCE + "[activeLightIndex], " + prefix + HW::T_POSITION_WORLD + ", &lightShader)", stage);
    shadergen.emitLine("L = lightShader.direction", stage);
    shadergen.emitLineBreak(stage);

    shadergen.emitComment("Calculate the BSDF response for this light source", stage);
    if (bsdf->hasClassification(ShaderNode::Classification::BSDF_R))
    {
        shadergen.emitLine("var closureData: ClosureData = makeClosureData(CLOSURE_TYPE_REFLECTION, L, V, N, P, occlusion)", stage);
        shadergen.emitFunctionCall(*bsdf, context, stage);
    }
    else
    {
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(bsdf->getOutput(), true, true, context, stage);
        shadergen.emitLineEnd(stage);
    }

    shadergen.emitLineBreak(stage);
    shadergen.emitComment("Accumulate the light's contribution", stage);
    shadergen.emitLine(outColor + " += lightShader.intensity * " + bsdf->getOutput()->getVariable() + ".response", stage);

    shadergen.emitScopeEnd(stage);
    shadergen.emitLineBreak(stage);
}

MATERIALX_NAMESPACE_END
