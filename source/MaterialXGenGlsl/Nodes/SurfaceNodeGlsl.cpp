#include <MaterialXGenGlsl/Nodes/SurfaceNodeGlsl.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

namespace
{
    static const string LIGHT_LOOP_BEGIN =
        "vec3 N = normalize(vd.normalWorld);\n"
        "vec3 V = normalize(u_viewPosition - vd.positionWorld);\n"
        "int numLights = numActiveLightSources();\n"
        "lightshader lightShader;\n"
        "for (int activeLightIndex = 0; activeLightIndex < numLights; ++activeLightIndex)\n";

    static const string LIGHT_CONTRIBUTION =
        "sampleLightSource(u_lightData[activeLightIndex], vd.positionWorld, lightShader);\n"
        "vec3 L = lightShader.direction;\n";
}

SurfaceNodeGlsl::SurfaceNodeGlsl()
{
    // Create closure contexts for calling closure functions.
    //
    // Reflection context
    _callReflection = HwClosureContext::create(HwClosureContext::REFLECTION);
    _callReflection->setSuffix("_reflection");
    _callReflection->addArgument(Type::VECTOR3, HW::LIGHT_DIR);
    _callReflection->addArgument(Type::VECTOR3, HW::VIEW_DIR);
    // Transmission context
    _callTransmission = HwClosureContext::create(HwClosureContext::TRANSMISSION);
    _callTransmission->setSuffix("_transmission");
    _callTransmission->addArgument(Type::VECTOR3, HW::VIEW_DIR);
    // Indirect context
    _callIndirect = HwClosureContext::create(HwClosureContext::INDIRECT);
    _callIndirect->setSuffix("_indirect");
    _callIndirect->addArgument(Type::VECTOR3, HW::VIEW_DIR);
    // Emission context
    _callEmission = HwClosureContext::create(HwClosureContext::EMISSION);
    _callEmission->addArgument(Type::VECTOR3, HW::NORMAL_DIR);
    _callEmission->addArgument(Type::VECTOR3, HW::VIEW_DIR);
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
    ShaderStage& vs = shader.getStage(HW::VERTEX_STAGE);
    ShaderStage& ps = shader.getStage(HW::PIXEL_STAGE);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, "i_position", vs);
    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, "i_normal", vs);
    addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldInverseTransposeMatrix", vs);

    addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, "positionWorld", vs, ps);
    addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, "normalWorld", vs, ps);

    addStageUniform(HW::PRIVATE_UNIFORMS, Type::VECTOR3, "u_viewPosition", ps);
    ShaderPort* numActiveLights = addStageUniform(HW::PRIVATE_UNIFORMS, Type::INTEGER, "u_numActiveLightSources", ps);
    numActiveLights->setValue(Value::createValue<int>(0));
}

void SurfaceNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const GlslShaderGenerator& shadergen = static_cast<const GlslShaderGenerator&>(context.getShaderGenerator());

    const ShaderGraph& graph = *node.getParent();

    BEGIN_SHADER_STAGE(stage, HW::VERTEX_STAGE)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        ShaderPort* position = vertexData["positionWorld"];
        if (!position->isEmitted())
        {
            position->setEmitted();
            shadergen.emitLine(prefix + position->getVariable() + " = hPositionWorld.xyz", stage);
        }
        ShaderPort* normal = vertexData["normalWorld"];
        if (!normal->isEmitted())
        {
            normal->setEmitted();
            shadergen.emitLine(prefix + normal->getVariable() + " = normalize((u_worldInverseTransposeMatrix * vec4(i_normal, 0)).xyz)", stage);
        }
    END_SHADER_STAGE(stage, HW::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(stage, HW::PIXEL_STAGE)
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
        shadergen.emitBlock(LIGHT_LOOP_BEGIN, context, stage);
        shadergen.emitScopeBegin(stage);

        shadergen.emitBlock(LIGHT_CONTRIBUTION, context, stage);
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

    END_SHADER_STAGE(stage, HW::PIXEL_STAGE)
}

} // namespace MaterialX
