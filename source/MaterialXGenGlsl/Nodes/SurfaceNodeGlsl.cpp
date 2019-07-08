//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/SurfaceNodeGlsl.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

SurfaceNodeGlsl::SurfaceNodeGlsl()
{
    // Create closure contexts for calling closure functions.
    //
    // Reflection context
    _callReflection = HwClosureContext::create(HwClosureContext::REFLECTION);
    _callReflection->setSuffix("_reflection");
    _callReflection->addArgument(Type::VECTOR3, HW::DIR_L);
    _callReflection->addArgument(Type::VECTOR3, HW::DIR_V);
    // Transmission context
    _callTransmission = HwClosureContext::create(HwClosureContext::TRANSMISSION);
    _callTransmission->setSuffix("_transmission");
    _callTransmission->addArgument(Type::VECTOR3, HW::DIR_V);
    // Indirect context
    _callIndirect = HwClosureContext::create(HwClosureContext::INDIRECT);
    _callIndirect->setSuffix("_indirect");
    _callIndirect->addArgument(Type::VECTOR3, HW::DIR_V);
    // Emission context
    _callEmission = HwClosureContext::create(HwClosureContext::EMISSION);
    _callEmission->addArgument(Type::VECTOR3, HW::DIR_N);
    _callEmission->addArgument(Type::VECTOR3, HW::DIR_V);
}

ShaderNodeImplPtr SurfaceNodeGlsl::create()
{
    return std::make_shared<SurfaceNodeGlsl>();
}

void SurfaceNodeGlsl::createVariables(const ShaderNode&, GenContext&, Shader& shader) const
{
    // TODO: 
    // The surface shader needs position, normal, view position and light sources. We should solve this by adding some 
    // dependency mechanism so this implementation can be set to depend on the PositionNodeGlsl, NormalNodeGlsl
    // ViewDirectionNodeGlsl and LightNodeGlsl nodes instead? This is where the MaterialX attribute "internalgeomprops" 
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
    ShaderPort* numActiveLights = addStageUniform(HW::PRIVATE_UNIFORMS, Type::INTEGER, HW::T_NUM_ACTIVE_LIGHT_SOURCES, ps);
    numActiveLights->setValue(Value::createValue<int>(0));
}

void SurfaceNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const GlslShaderGenerator& shadergen = static_cast<const GlslShaderGenerator&>(context.getShaderGenerator());

    const ShaderGraph& graph = *node.getParent();

    BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
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
            shadergen.emitLine(prefix + normal->getVariable() + " = normalize((" + HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX + " * vec4(" + HW::T_IN_NORMAL + ", 0)).xyz)", stage);
        }
    END_SHADER_STAGE(stage, Stage::VERTEX)

    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        // Declare the output variable
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, true, context, stage);
        shadergen.emitLineEnd(stage);

        shadergen.emitScopeBegin(stage);

        if (context.getOptions().hwTransparency)
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitString("float surfaceOpacity = ", stage);
            shadergen.emitInput(node.getInput("opacity"), context, stage);
            shadergen.emitLineEnd(stage);
            // Early out for 100% cutout transparency
            shadergen.emitLine("if (surfaceOpacity < 0.001)", stage, false);
            shadergen.emitScopeBegin(stage);
            shadergen.emitLine("discard", stage);
            shadergen.emitScopeEnd(stage);
            shadergen.emitLineBreak(stage);
        }

        const ShaderOutput* output = node.getOutput();
        const string outColor = output->getVariable() + ".color";
        const string outTransparency = output->getVariable() + ".transparency";

        //
        // Handle direct lighting
        //

        shadergen.emitComment("Light loop", stage);
        shadergen.emitLine("vec3 N = normalize(" + HW::T_VERTEX_DATA_INSTANCE + "." + HW::T_NORMAL_WORLD + ")", stage);
        shadergen.emitLine("vec3 V = normalize(" + HW::T_VIEW_POSITION + " - " + HW::T_VERTEX_DATA_INSTANCE + "." + HW::T_POSITION_WORLD + ")", stage);
        shadergen.emitLine("int numLights = numActiveLightSources()", stage);
        shadergen.emitLine("lightshader lightShader", stage);
        shadergen.emitLine("for (int activeLightIndex = 0; activeLightIndex < numLights; ++activeLightIndex)", stage, false);

        shadergen.emitScopeBegin(stage);

        shadergen.emitLine("sampleLightSource(" + HW::T_LIGHT_DATA_INSTANCE + "[activeLightIndex], " + HW::T_VERTEX_DATA_INSTANCE + "." + HW::T_POSITION_WORLD + ", lightShader)", stage);
        shadergen.emitLine("vec3 L = lightShader.direction", stage);
        shadergen.emitLineBreak(stage);

        shadergen.emitComment("Calculate the BSDF response for this light source", stage);
        string bsdf;
        shadergen.emitBsdfNodes(graph, node, _callReflection, context, stage, bsdf);
        shadergen.emitLineBreak(stage);

        shadergen.emitComment("Accumulate the light's contribution", stage);
        shadergen.emitLine(outColor + " += lightShader.intensity * " + bsdf, stage);

        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);

        //
        // Handle indirect lighting.
        //

        shadergen.emitComment("Add surface emission", stage);
        shadergen.emitScopeBegin(stage);
        string emission;
        shadergen.emitEdfNodes(graph, node, _callEmission, context, stage, emission);
        shadergen.emitLine(outColor + " += " + emission, stage);
        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);

        shadergen.emitComment("Add indirect contribution", stage);
        shadergen.emitScopeBegin(stage);
        shadergen.emitBsdfNodes(graph, node, _callIndirect, context, stage, bsdf);
        shadergen.emitLineBreak(stage);
        shadergen.emitLine(outColor + " += " + bsdf, stage);
        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);

        // Handle surface transparency
        //
        if (context.getOptions().hwTransparency)
        {
            shadergen.emitComment("Calculate the BSDF transmission for viewing direction", stage);
            shadergen.emitScopeBegin(stage);
            shadergen.emitBsdfNodes(graph, node, _callTransmission, context, stage, bsdf);
            shadergen.emitLine(outTransparency + " = " + bsdf, stage);
            shadergen.emitScopeEnd(stage);
            shadergen.emitLineBreak(stage);

            shadergen.emitComment("Mix in opacity which affect the total result", stage);
            shadergen.emitLine(outColor + " *= surfaceOpacity", stage);
            shadergen.emitLine(outTransparency + " = mix(vec3(1.0), " + outTransparency + ", surfaceOpacity)", stage);
        }
        else
        {
            shadergen.emitLine(outTransparency + " = vec3(0.0)", stage);
        }

        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);

    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
