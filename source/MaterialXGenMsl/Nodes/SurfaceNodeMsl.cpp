//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMsl/Nodes/SurfaceNodeMsl.h>
#include <MaterialXGenMsl/MslShaderGenerator.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

SurfaceNodeMsl::SurfaceNodeMsl()
{}

ShaderNodeImplPtr SurfaceNodeMsl::create()
{
    return std::make_shared<SurfaceNodeMsl>();
}

void SurfaceNodeMsl::createVariables(const ShaderNode&, GenContext& context, Shader& shader) const
{
    // TODO:
    // The surface shader needs position, normal, view position and light sources. We should solve this by adding some
    // dependency mechanism so this implementation can be set to depend on the HwPositionNode, HwNormalNode
    // HwViewDirectionNode and LightNodeMsl nodes instead? This is where the MaterialX attribute "internalgeomprops"
    // is needed.
    //
    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, HW::T_IN_POSITION, vs);
    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, HW::T_IN_NORMAL, vs);
    addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX, vs);

    addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, HW::T_POSITION_WORLD, vs, ps);
    addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, HW::T_NORMAL_WORLD, vs, ps);

    addStageUniform(HW::PRIVATE_UNIFORMS, Type::VECTOR3, HW::T_VIEW_POSITION, ps);

    const MslShaderGenerator& shadergen = static_cast<const MslShaderGenerator&>(context.getShaderGenerator());
    shadergen.addStageLightingUniforms(context, ps);
}

void SurfaceNodeMsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const MslShaderGenerator& shadergen = static_cast<const MslShaderGenerator&>(context.getShaderGenerator());

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
            shadergen.emitLine(prefix + normal->getVariable() + " = normalize((" + HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX + " * float4(" + HW::T_IN_NORMAL + ", 0)).xyz)", stage);
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

        // Declare the output variable
        const ShaderOutput* output = node.getOutput();
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(output, true, true, context, stage);
        shadergen.emitLineEnd(stage);

        shadergen.emitScopeBegin(stage);

        shadergen.emitLine("float3 N = normalize(" + prefix + HW::T_NORMAL_WORLD + ")", stage);
        shadergen.emitLine("float3 V = normalize(" + HW::T_VIEW_POSITION + " - " + prefix + HW::T_POSITION_WORLD + ")", stage);
        shadergen.emitLine("float3 P = " + prefix + HW::T_POSITION_WORLD, stage);
        shadergen.emitLine("float3 L = float3(0,0,0);", stage);
        shadergen.emitLine("float occlusion = 1.0", stage);
        shadergen.emitLineBreak(stage);

        const string outColor = output->getVariable() + ".color";
        const string outTransparency = output->getVariable() + ".transparency";

        const ShaderInput* bsdfInput = node.getInput("bsdf");
        if (const ShaderNode* bsdf = bsdfInput->getConnectedSibling())
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitString("float surfaceOpacity = ", stage);
            shadergen.emitInput(node.getInput("opacity"), context, stage);
            shadergen.emitLineEnd(stage);
            shadergen.emitLineBreak(stage);

            //
            // Handle direct lighting
            //
            shadergen.emitComment("Shadow occlusion", stage);
            if (context.getOptions().hwShadowMap)
            {
                shadergen.emitLine("float3 shadowCoord = (" + HW::T_SHADOW_MATRIX + " * float4(" + prefix + HW::T_POSITION_WORLD + ", 1.0)).xyz", stage);
                shadergen.emitLine("shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5", stage);

                shadergen.emitLine("float2 shadowMoments = texture(" + HW::T_SHADOW_MAP + ", shadowCoord.xy).xy", stage);
                shadergen.emitLine("occlusion = mx_variance_shadow_occlusion(shadowMoments, shadowCoord.z)", stage);
            }
            shadergen.emitLineBreak(stage);

            emitLightLoop(node, context, stage, outColor);

            //
            // Handle indirect lighting.
            //
            shadergen.emitComment("Ambient occlusion", stage);
            if (context.getOptions().hwAmbientOcclusion)
            {
                ShaderPort* texcoord = vertexData[HW::T_TEXCOORD + "_0"];
                shadergen.emitLine("float2 ambOccUv = mx_transform_uv(" + prefix + texcoord->getVariable() + ", float2(1.0), float2(0.0))", stage);
                shadergen.emitLine("occlusion = mix(1.0, texture(" + HW::T_AMB_OCC_MAP + ", ambOccUv).x, " + HW::T_AMB_OCC_GAIN + ")", stage);
            }
            else
            {
                shadergen.emitLine("occlusion = 1.0", stage);
            }
            shadergen.emitLineBreak(stage);

            shadergen.emitComment("Add environment contribution", stage);
            shadergen.emitScopeBegin(stage);

            if (bsdf->hasClassification(ShaderNode::Classification::BSDF_R)) {
                shadergen.emitLine("ClosureData closureData = {CLOSURE_TYPE_INDIRECT, L, V, N, P, occlusion}", stage);
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

        //
        // Handle surface emission.
        //
        const ShaderInput* edfInput = node.getInput("edf");
        if (const ShaderNode* edf = edfInput->getConnectedSibling())
        {
            shadergen.emitComment("Add surface emission", stage);
            shadergen.emitScopeBegin(stage);

            if (edf->hasClassification(ShaderNode::Classification::EDF)) {
                shadergen.emitLine("ClosureData closureData = {CLOSURE_TYPE_EMISSION, L, V, N, P, occlusion}", stage);
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

        //
        // Handle surface transmission and opacity.
        //
        if (const ShaderNode* bsdf = bsdfInput->getConnectedSibling())
        {
            shadergen.emitComment("Calculate the BSDF transmission for viewing direction", stage);
            if (bsdf->hasClassification(ShaderNode::Classification::BSDF_T)) {
                shadergen.emitLine("ClosureData closureData = {CLOSURE_TYPE_TRANSMISSION, L, V, N, P, occlusion}", stage);
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
            shadergen.emitLine(outTransparency + " = mix(float3(1.0), " + outTransparency + ", surfaceOpacity)", stage);
            shadergen.emitScopeEnd(stage);
        }

        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);
    }
}

void SurfaceNodeMsl::emitLightLoop(const ShaderNode& node, GenContext& context, ShaderStage& stage, const string& outColor) const
{
    //
    // Generate Light loop if requested
    //
    if (context.getOptions().hwMaxActiveLightSources > 0)
    {
        const MslShaderGenerator& shadergen = static_cast<const MslShaderGenerator&>(context.getShaderGenerator());
        const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);

        const ShaderInput* bsdfInput = node.getInput("bsdf");
        const ShaderNode* bsdf = bsdfInput->getConnectedSibling();

        shadergen.emitComment("Light loop", stage);
        shadergen.emitLine("int numLights = numActiveLightSources()", stage);
        shadergen.emitLine("lightshader lightShader", stage);
        shadergen.emitLine("for (int activeLightIndex = 0; activeLightIndex < numLights; ++activeLightIndex)", stage, false);

        shadergen.emitScopeBegin(stage);

        shadergen.emitLine("sampleLightSource(" + HW::T_LIGHT_DATA_INSTANCE + "[activeLightIndex], " + prefix + HW::T_POSITION_WORLD + ", lightShader)", stage);
        shadergen.emitLine("L = lightShader.direction", stage);
        shadergen.emitLineBreak(stage);

        shadergen.emitComment("Calculate the BSDF response for this light source", stage);
        if (bsdf->hasClassification(ShaderNode::Classification::BSDF_R)) {
            shadergen.emitLine("ClosureData closureData = {CLOSURE_TYPE_REFLECTION, L, V, N, P, occlusion}", stage);
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
}

MATERIALX_NAMESPACE_END
