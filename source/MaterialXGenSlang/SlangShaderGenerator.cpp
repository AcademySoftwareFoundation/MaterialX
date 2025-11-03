//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenSlang/SlangShaderGenerator.h>

#include <MaterialXGenSlang/SlangSyntax.h>

#include <MaterialXGenShader/Nodes/MaterialNode.h>
#include <MaterialXGenHw/Nodes/HwImageNode.h>
#include <MaterialXGenHw/Nodes/HwGeomColorNode.h>
#include <MaterialXGenHw/Nodes/HwGeomPropValueNode.h>
#include <MaterialXGenHw/Nodes/HwTexCoordNode.h>
#include <MaterialXGenHw/Nodes/HwTransformNode.h>
#include <MaterialXGenHw/Nodes/HwPositionNode.h>
#include <MaterialXGenHw/Nodes/HwNormalNode.h>
#include <MaterialXGenHw/Nodes/HwTangentNode.h>
#include <MaterialXGenHw/Nodes/HwBitangentNode.h>
#include <MaterialXGenHw/Nodes/HwFrameNode.h>
#include <MaterialXGenHw/Nodes/HwTimeNode.h>
#include <MaterialXGenHw/Nodes/HwViewDirectionNode.h>
#include <MaterialXGenHw/Nodes/HwLightCompoundNode.h>
#include <MaterialXGenHw/Nodes/HwLightNode.h>
#include <MaterialXGenHw/Nodes/HwLightSamplerNode.h>
#include <MaterialXGenHw/Nodes/HwLightShaderNode.h>
#include <MaterialXGenHw/Nodes/HwNumLightsNode.h>
#include <MaterialXGenHw/Nodes/HwSurfaceNode.h>

#include <MaterialXGenShader/Util.h>

#include <cassert>

MATERIALX_NAMESPACE_BEGIN

const string SlangShaderGenerator::TARGET = "genslang";
const string SlangShaderGenerator::VERSION = "2025.3";

//
// SlangShaderGenerator methods
//

SlangShaderGenerator::SlangShaderGenerator(TypeSystemPtr typeSystem) :
    HwShaderGenerator(typeSystem, SlangSyntax::create(typeSystem))
{
    //
    // Register all custom node implementation classes
    //

    StringVec elementNames;

    // <!-- <position> -->
    registerImplementation("IM_position_vector3_" + SlangShaderGenerator::TARGET, HwPositionNode::create);
    // <!-- <normal> -->
    registerImplementation("IM_normal_vector3_" + SlangShaderGenerator::TARGET, HwNormalNode::create);
    // <!-- <tangent> -->
    registerImplementation("IM_tangent_vector3_" + SlangShaderGenerator::TARGET, HwTangentNode::create);
    // <!-- <bitangent> -->
    registerImplementation("IM_bitangent_vector3_" + SlangShaderGenerator::TARGET, HwBitangentNode::create);
    // <!-- <texcoord> -->
    registerImplementation("IM_texcoord_vector2_" + SlangShaderGenerator::TARGET, HwTexCoordNode::create);
    registerImplementation("IM_texcoord_vector3_" + SlangShaderGenerator::TARGET, HwTexCoordNode::create);
    // <!-- <geomcolor> -->
    registerImplementation("IM_geomcolor_float_" + SlangShaderGenerator::TARGET, HwGeomColorNode::create);
    registerImplementation("IM_geomcolor_color3_" + SlangShaderGenerator::TARGET, HwGeomColorNode::create);
    registerImplementation("IM_geomcolor_color4_" + SlangShaderGenerator::TARGET, HwGeomColorNode::create);
    // <!-- <geompropvalue> -->
    elementNames = {
        "IM_geompropvalue_integer_" + SlangShaderGenerator::TARGET,
        "IM_geompropvalue_float_" + SlangShaderGenerator::TARGET,
        "IM_geompropvalue_color3_" + SlangShaderGenerator::TARGET,
        "IM_geompropvalue_color4_" + SlangShaderGenerator::TARGET,
        "IM_geompropvalue_vector2_" + SlangShaderGenerator::TARGET,
        "IM_geompropvalue_vector3_" + SlangShaderGenerator::TARGET,
        "IM_geompropvalue_vector4_" + SlangShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, HwGeomPropValueNode::create);
    registerImplementation("IM_geompropvalue_boolean_" + SlangShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);
    registerImplementation("IM_geompropvalue_string_" + SlangShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);
    registerImplementation("IM_geompropvalue_filename_" + SlangShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);

    // <!-- <frame> -->
    registerImplementation("IM_frame_float_" + SlangShaderGenerator::TARGET, HwFrameNode::create);
    // <!-- <time> -->
    registerImplementation("IM_time_float_" + SlangShaderGenerator::TARGET, HwTimeNode::create);
    // <!-- <viewdirection> -->
    registerImplementation("IM_viewdirection_vector3_" + SlangShaderGenerator::TARGET, HwViewDirectionNode::create);

    // <!-- <surface> -->
    registerImplementation("IM_surface_" + SlangShaderGenerator::TARGET, HwSurfaceNode::create);

    // <!-- <light> -->
    registerImplementation("IM_light_" + SlangShaderGenerator::TARGET, HwLightNode::create);

    // <!-- <point_light> -->
    registerImplementation("IM_point_light_" + SlangShaderGenerator::TARGET, HwLightShaderNode::create);
    // <!-- <directional_light> -->
    registerImplementation("IM_directional_light_" + SlangShaderGenerator::TARGET, HwLightShaderNode::create);
    // <!-- <spot_light> -->
    registerImplementation("IM_spot_light_" + SlangShaderGenerator::TARGET, HwLightShaderNode::create);

    // <!-- <ND_transformpoint> ->
    registerImplementation("IM_transformpoint_vector3_" + SlangShaderGenerator::TARGET, HwTransformPointNode::create);

    // <!-- <ND_transformvector> ->
    registerImplementation("IM_transformvector_vector3_" + SlangShaderGenerator::TARGET, HwTransformVectorNode::create);

    // <!-- <ND_transformnormal> ->
    registerImplementation("IM_transformnormal_vector3_" + SlangShaderGenerator::TARGET, HwTransformNormalNode::create);

    // <!-- <image> -->
    elementNames = {
        "IM_image_float_" + SlangShaderGenerator::TARGET,
        "IM_image_color3_" + SlangShaderGenerator::TARGET,
        "IM_image_color4_" + SlangShaderGenerator::TARGET,
        "IM_image_vector2_" + SlangShaderGenerator::TARGET,
        "IM_image_vector3_" + SlangShaderGenerator::TARGET,
        "IM_image_vector4_" + SlangShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, HwImageNode::create);

    // <!-- <surfacematerial> -->
    registerImplementation("IM_surfacematerial_" + SlangShaderGenerator::TARGET, MaterialNode::create);

    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "numActiveLightSources", HwNumLightsNode::create()));
    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "sampleLightSource", HwLightSamplerNode::create()));
}

ShaderPtr SlangShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);

    // Request fixed floating-point notation for consistency across targets.
    ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    /// Sets semantics on the connector data between vertex and pixel stages
    auto setDataSemantics = [](VariableBlock& vertexData)
    {
        for (size_t i = 0; i < vertexData.size(); ++i)
        {
            ShaderPort* port = vertexData[i];
            if (port->getSemantic().empty())
                port->setSemantic(port->getName());
        }
    };

    // Emit code for vertex shader stage
    ShaderStage& vs = shader->getStage(Stage::VERTEX);
    setDataSemantics(vs.getOutputBlock(HW::VERTEX_DATA));
    {
        // All semantics that, in Slang, can have index first check for `name_` and only then for `name`.
        // Because HwShaderGenerator generates index variables as `name_X` and Slang needs `SEMANTICX`.
        // If the variable is not indexed, we also look it up without the `_`
        StringMap indexedSubstitutions;
        StringMap semanticSubstitutions;
        indexedSubstitutions[HW::T_IN_POSITION + "_"] = "POSITION";
        indexedSubstitutions[HW::T_IN_NORMAL + "_"] = "NORMAL";
        indexedSubstitutions[HW::T_IN_BITANGENT + "_"] = "BINORMAL";
        indexedSubstitutions[HW::T_IN_TANGENT + "_"] = "TANGENT";
        indexedSubstitutions[HW::T_IN_TEXCOORD + "_"] = "TEXCOORD";
        indexedSubstitutions[HW::T_IN_COLOR + "_"] = "COLOR";
        semanticSubstitutions[HW::T_IN_POSITION] = "POSITION";
        semanticSubstitutions[HW::T_IN_NORMAL] = "NORMAL";
        semanticSubstitutions[HW::T_IN_BITANGENT] = "BINORMAL";
        semanticSubstitutions[HW::T_IN_TANGENT] = "TANGENT";
        semanticSubstitutions[HW::T_IN_TEXCOORD] = "TEXCOORD";
        semanticSubstitutions[HW::T_IN_COLOR] = "COLOR";

        // This replaces token without assuming that they are just alphanumeric, so we can turn
        // HW::T_IN_TEXCOORD + "_" into `TEXTCOORD` because tokenSubstitution does not work on it.
        auto tokenReplace = [](const string& token, const string& replacement, string& source)
        {
            std::size_t pos = 0;
            while (true)
            {
                pos = source.find(token, pos);
                if (pos == std::string::npos)
                    return;
                source.replace(pos, token.length(), replacement);
            }
        };

        auto replaceAllTokens = [&tokenReplace](const StringMap& substitions, string& source)
        {
            for (auto& it : substitions)
                tokenReplace(it.first, it.second, source);
        };

        // Add semantics to all vertex inputs.
        // We substitute the port name with the list of semantic substitutions.
        // If the has special semantic (e.g., POSITION[n]) then it is generated from the variable name.
        // Otherwise, variable name itself is used as a semantic.
        VariableBlock& inputBlock = vs.getInputBlock(HW::VERTEX_INPUTS);
        for (size_t i = 0; i < inputBlock.size(); ++i)
        {
            ShaderPort* port = inputBlock[i];
            std::string semantic = port->getName();
            replaceAllTokens(indexedSubstitutions, semantic);
            tokenSubstitution(semanticSubstitutions, semantic);
            port->setSemantic(semantic);
        }
    }
    emitVertexStage(shader->getGraph(), context, vs);
    replaceTokens(_tokenSubstitutions, vs);
    SlangSyntaxFromGlsl(vs);

    // Emit code for pixel shader stage
    ShaderStage& ps = shader->getStage(Stage::PIXEL);
    setDataSemantics(ps.getInputBlock(HW::VERTEX_DATA));
    emitPixelStage(shader->getGraph(), context, ps);
    replaceTokens(_tokenSubstitutions, ps);
    SlangSyntaxFromGlsl(ps);

    return shader;
}

void SlangShaderGenerator::emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    emitDirectives(context, stage);
    emitLineBreak(stage);

    // Add all constants
    emitConstants(context, stage);

    // Add all uniforms
    emitUniforms(context, stage, false);

    // Add vertex inputs
    emitInputs(context, stage);

    // Add vertex data outputs block
    emitOutputs(context, stage);

    // Add common math functions
    emitLibraryInclude("stdlib/genslang/lib/mx_math.slang", context, stage);
    emitLineBreak(stage);

    emitFunctionDefinitions(graph, context, stage);

    const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
    const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    const SlangShaderGenerator& shadergen = static_cast<const SlangShaderGenerator&>(context.getShaderGenerator());

    // Add main function
    setFunctionName("vertexMain", stage);
    emitLine("[shader(\"vertex\")]", stage, false);
    emitLine(vertexData.getName() + " vertexMain(" + vertexInputs.getName() + " vi)", stage, false);
    emitFunctionBodyBegin(graph, context, stage);

    // Assign input variables from the `ui` input struct to static variables to mimic the GLSL globals.
    emitComment("Variable input block", stage);
    for (size_t i = 0; i < vertexInputs.size(); ++i)
        emitLine(vertexInputs[i]->getName() + " = vi." + vertexInputs[i]->getName(), stage);

    emitLine("float4 hPositionWorld = mul(float4(" + HW::T_IN_POSITION + ", 1.0), " + HW::T_WORLD_MATRIX + ")", stage);
    emitLine(vertexData.getName() + " " + vertexData.getInstance(), stage);
    emitLine(shadergen.getVertexDataPrefix(vertexData) + "SV_Position = mul(hPositionWorld, " + HW::T_VIEW_PROJECTION_MATRIX + ")", stage);

    // Emit all function calls in order
    for (const ShaderNode* node : graph.getNodes())
    {
        emitFunctionCall(*node, context, stage);
    }

    emitLine("return " + vertexData.getInstance(), stage);
    emitFunctionBodyEnd(graph, context, stage);
}

void SlangShaderGenerator::emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const
{
    int specularMethod = context.getOptions().hwSpecularEnvironmentMethod;
    if (specularMethod == SPECULAR_ENVIRONMENT_FIS)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_environment_fis.glsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_PREFILTER)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_environment_prefilter.glsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_NONE)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_environment_none.glsl", context, stage);
    }
    else
    {
        throw ExceptionShaderGenError("Invalid hardware specular environment method specified: '" + std::to_string(specularMethod) + "'");
    }
    emitLineBreak(stage);
}

void SlangShaderGenerator::emitTransmissionRender(GenContext& context, ShaderStage& stage) const
{
    int transmissionMethod = context.getOptions().hwTransmissionRenderMethod;
    if (transmissionMethod == TRANSMISSION_REFRACTION)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_transmission_refract.glsl", context, stage);
    }
    else if (transmissionMethod == TRANSMISSION_OPACITY)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_transmission_opacity.glsl", context, stage);
    }
    else
    {
        throw ExceptionShaderGenError("Invalid transmission render specified: '" + std::to_string(transmissionMethod) + "'");
    }
    emitLineBreak(stage);
}

void SlangShaderGenerator::emitDirectives(GenContext&, ShaderStage&) const
{
}

void SlangShaderGenerator::emitConstants(GenContext& context, ShaderStage& stage) const
{
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, "static const", Syntax::SEMICOLON, context, stage);
        emitLineBreak(stage);
    }
}

void SlangShaderGenerator::emitUniforms(GenContext& context, ShaderStage& stage, bool emitLighting) const
{
    if (emitLighting)
    {
        const unsigned int maxLights = std::max(1u, context.getOptions().hwMaxActiveLightSources);
        emitLine("#define " + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + " " + std::to_string(maxLights), stage, false);
        emitLineBreak(stage);
        const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
        emitLine("struct " + lightData.getName(), stage, false);
        emitScopeBegin(stage);
        emitVariableDeclarations(lightData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
        emitScopeEnd(stage, true);
        emitLineBreak(stage);
    }

    emitLine("cbuffer " + stage.getName() + "CB", stage, false);
    emitScopeBegin(stage);
    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;

        // Skip light uniforms as they are handled separately
        if (!uniforms.empty() && uniforms.getName() != HW::LIGHT_DATA)
        {
            emitComment("Uniform block: " + uniforms.getName(), stage);
            emitVariableDeclarations(uniforms, _syntax->getUniformQualifier(), Syntax::SEMICOLON, context, stage);
            emitLineBreak(stage);
        }
    }

    if (emitLighting)
    {
        const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
        const string structArraySuffix = "[" + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + "]";
        const string structName = lightData.getInstance();
        emitLine("uniform " + lightData.getName() + " " + structName + structArraySuffix, stage);
    }

    emitScopeEnd(stage);
    emitLineBreak(stage);
}

void SlangShaderGenerator::emitInputs(GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
        if (!vertexInputs.empty())
        {
            emitLine("struct " + vertexInputs.getName(), stage, false);
            emitScopeBegin(stage);
            emitVariableDeclarations(vertexInputs, _syntax->getInputQualifier(), Syntax::SEMICOLON, context, stage, false);
            emitScopeEnd(stage);
            emitLineBreak(stage);

            for (size_t i = 0; i < vertexInputs.size(); ++i)
            {
                ShaderPort port = *vertexInputs[i];
                port.setSemantic("");

                emitLineBegin(stage);
                emitVariableDeclaration(&port, "static", context, stage, false);
                emitString(Syntax::SEMICOLON, stage);
                emitLineEnd(stage, false);
            }
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        emitLine("struct " + vertexData.getName(), stage, false);
        emitScopeBegin(stage);
        emitLine("float4 SV_Position : SV_Position", stage);
        if (!vertexData.empty())
        {
            emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
        }
        emitScopeEnd(stage, false, false);
        emitLineBreak(stage);
        emitLine("static " + vertexData.getName() + " vd", stage);
    }
}

void SlangShaderGenerator::emitOutputs(GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        emitLine("struct " + vertexData.getName(), stage, false);
        emitScopeBegin(stage);
        emitLine("float4 SV_Position : SV_Position", stage);
        if (!vertexData.empty())
        {
            emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
        }
        emitScopeEnd(stage);
        emitLineBreak(stage);
        emitLineBreak(stage);
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        emitComment("Pixel shader outputs", stage);
        const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
        emitVariableDeclarations(outputs, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
        emitLineBreak(stage);
    }
}

string SlangShaderGenerator::getVertexDataPrefix(const VariableBlock& vertexData) const
{
    return vertexData.getInstance() + ".";
}

bool SlangShaderGenerator::requiresLighting(const ShaderGraph& graph) const
{
    const bool isBsdf = graph.hasClassification(ShaderNode::Classification::BSDF);
    const bool isLitSurfaceShader = graph.hasClassification(ShaderNode::Classification::SHADER) &&
                                    graph.hasClassification(ShaderNode::Classification::SURFACE) &&
                                    !graph.hasClassification(ShaderNode::Classification::UNLIT);
    return isBsdf || isLitSurfaceShader;
}

void SlangShaderGenerator::emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    // Add directives
    emitDirectives(context, stage);
    emitLineBreak(stage);

    // Add textures
    emitLibraryInclude("stdlib/genslang/lib/mx_texture.slang", context, stage);

    // Add type definitions
    emitTypeDefinitions(context, stage);

    // Add all constants
    emitConstants(context, stage);

    // Determine whether lighting is required
    bool lighting = requiresLighting(graph);
    bool emitLightUniforms = lighting && context.getOptions().hwMaxActiveLightSources > 0;

    // Add all uniforms
    emitUniforms(context, stage, emitLightUniforms);

    // Add vertex data inputs block
    emitInputs(context, stage);

    // Add common math functions
    emitLibraryInclude("stdlib/genslang/lib/mx_math.slang", context, stage);
    emitLineBreak(stage);

    // Define directional albedo approach
    if (lighting || context.getOptions().hwWriteAlbedoTable || context.getOptions().hwWriteEnvPrefilter)
    {
        emitLine("#define DIRECTIONAL_ALBEDO_METHOD " + std::to_string(int(context.getOptions().hwDirectionalAlbedoMethod)), stage, false);
        emitLineBreak(stage);
    }

    // Define Airy Fresnel iterations
    emitLine("#define AIRY_FRESNEL_ITERATIONS " + std::to_string(context.getOptions().hwAiryFresnelIterations), stage, false);
    emitLineBreak(stage);

    // Add lighting support
    if (lighting)
    {
        emitSpecularEnvironment(context, stage);
        emitTransmissionRender(context, stage);
    }

    // Add shadowing support
    bool shadowing = (lighting && context.getOptions().hwShadowMap) ||
                     context.getOptions().hwWriteDepthMoments;
    if (shadowing)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_shadow.glsl", context, stage);
        emitLibraryInclude("pbrlib/genglsl/lib/mx_shadow_platform.glsl", context, stage);
    }

    // Emit directional albedo table code.
    if (context.getOptions().hwWriteAlbedoTable)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_generate_albedo_table.glsl", context, stage);
        emitLineBreak(stage);
    }

    // Emit environment prefiltering code
    if (context.getOptions().hwWriteEnvPrefilter)
    {
        emitLibraryInclude("pbrlib/genglsl/lib/mx_generate_prefilter_env.glsl", context, stage);
        emitLineBreak(stage);
    }

    // Set the include file to use for uv transformations,
    // depending on the vertical flip flag.
    if (context.getOptions().fileTextureVerticalFlip)
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "mx_transform_uv_vflip.glsl";
    }
    else
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "mx_transform_uv.glsl";
    }

    _tokenSubstitutions[HW::T_TEX_SAMPLER_SIGNATURE] = "SamplerTexture2D tex_sampler";

    emitLightFunctionDefinitions(graph, context, stage);

    // Emit function definitions for all nodes in the graph.
    emitFunctionDefinitions(graph, context, stage);

    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();

    const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);

    // Add main function
    setFunctionName("fragmentMain", stage);
    emitLine("[shader(\"fragment\")]", stage, false);
    emitLine("float4 fragmentMain(" + vertexData.getName() + " _vd) : SV_Target", stage, false);
    emitFunctionBodyBegin(graph, context, stage);

    emitComment("Fragment shader inputs", stage);
    emitLine("vd = _vd", stage);

    // Add the pixel shader output. This needs to be a float4 for rendering
    // and upstream connection will be converted to float4 if needed in emitFinalOutput().
    // It is a variable local to the mainFragment, and returned at the end of it.
    emitOutputs(context, stage);

    if (graph.hasClassification(ShaderNode::Classification::CLOSURE) &&
        !graph.hasClassification(ShaderNode::Classification::SHADER))
    {
        // Handle the case where the graph is a direct closure.
        // We don't support rendering closures without attaching
        // to a surface shader, so just output black.
        emitLine(outputSocket->getVariable() + " = float4(0.0, 0.0, 0.0, 1.0)", stage);
    }
    else if (context.getOptions().hwWriteDepthMoments)
    {
        emitLine(outputSocket->getVariable() + " = float4(mx_compute_depth_moments(), 0.0, 1.0)", stage);
    }
    else if (context.getOptions().hwWriteAlbedoTable)
    {
        emitLine(outputSocket->getVariable() + " = float4(mx_generate_dir_albedo_table(), 1.0)", stage);
    }
    else if (context.getOptions().hwWriteEnvPrefilter)
    {
        emitLine(outputSocket->getVariable() + " = float4(mx_generate_prefilter_env(), 1.0)", stage);
    }
    else
    {
        // Add all function calls.
        //
        // Surface shaders need special handling.
        if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
        {
            // Emit all texturing nodes. These are inputs to any
            // closure/shader nodes and need to be emitted first.
            emitFunctionCalls(graph, context, stage, ShaderNode::Classification::TEXTURE);

            // Emit function calls for "root" closure/shader nodes.
            // These will internally emit function calls for any dependent closure nodes upstream.
            for (ShaderGraphOutputSocket* socket : graph.getOutputSockets())
            {
                if (socket->getConnection())
                {
                    const ShaderNode* upstream = socket->getConnection()->getNode();
                    if (upstream->getParent() == &graph &&
                        (upstream->hasClassification(ShaderNode::Classification::CLOSURE) ||
                         upstream->hasClassification(ShaderNode::Classification::SHADER)))
                    {
                        emitFunctionCall(*upstream, context, stage);
                    }
                }
            }
        }
        else
        {
            // No surface shader graph so just generate all
            // function calls in order.
            emitFunctionCalls(graph, context, stage);
        }

        // Emit final output
        const ShaderOutput* outputConnection = outputSocket->getConnection();
        if (outputConnection)
        {
            if (graph.hasClassification(ShaderNode::Classification::SURFACE))
            {
                string outColor = outputConnection->getVariable() + ".color";
                string outTransparency = outputConnection->getVariable() + ".transparency";
                if (context.getOptions().hwSrgbEncodeOutput)
                {
                    outColor = "mx_srgb_encode(" + outColor + ")";
                }
                if (context.getOptions().hwTransparency)
                {
                    emitLine("float outAlpha = clamp(1.0 - dot(" + outTransparency + ", float3(0.3333)), 0.0, 1.0)", stage);
                    emitLine(outputSocket->getVariable() + " = float4(" + outColor + ", outAlpha)", stage);
                    emitLine("if (outAlpha < " + HW::T_ALPHA_THRESHOLD + ")", stage, false);
                    emitScopeBegin(stage);
                    emitLine("discard", stage);
                    emitScopeEnd(stage);
                }
                else
                {
                    emitLine(outputSocket->getVariable() + " = float4(" + outColor + ", 1.0)", stage);
                }
            }
            else
            {
                string outValue = outputConnection->getVariable();
                if (context.getOptions().hwSrgbEncodeOutput && outputSocket->getType().isFloat3())
                {
                    outValue = "mx_srgb_encode(" + outValue + ")";
                }
                if (!outputSocket->getType().isFloat4())
                {
                    toVec4(outputSocket->getType(), outValue);
                }
                emitLine(outputSocket->getVariable() + " = " + outValue, stage);
            }
        }
        else
        {
            string outputValue = outputSocket->getValue() ? _syntax->getValue(outputSocket->getType(), *outputSocket->getValue()) : _syntax->getDefaultValue(outputSocket->getType());
            if (!outputSocket->getType().isFloat4())
            {
                string finalOutput = outputSocket->getVariable() + "_tmp";
                emitLine(_syntax->getTypeName(outputSocket->getType()) + " " + finalOutput + " = " + outputValue, stage);
                toVec4(outputSocket->getType(), finalOutput);
                emitLine(outputSocket->getVariable() + " = " + finalOutput, stage);
            }
            else
            {
                emitLine(outputSocket->getVariable() + " = " + outputValue, stage);
            }
        }
    }

    emitLine("return " + outputSocket->getVariable(), stage);

    // End main function
    emitFunctionBodyEnd(graph, context, stage);
}

void SlangShaderGenerator::emitLightFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        // Emit Light functions if requested
        if (requiresLighting(graph) && context.getOptions().hwMaxActiveLightSources > 0)
        {
            // For surface shaders we need light shaders
            if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
            {
                // Emit functions for all bound light shaders
                HwLightShadersPtr lightShaders = context.getUserData<HwLightShaders>(HW::USER_DATA_LIGHT_SHADERS);
                if (lightShaders)
                {
                    for (const auto& it : lightShaders->get())
                    {
                        emitFunctionDefinition(*it.second, context, stage);
                    }
                }
                // Emit functions for light sampling
                for (const auto& it : _lightSamplingNodes)
                {
                    emitFunctionDefinition(*it, context, stage);
                }
            }
        }
    }
}

void SlangShaderGenerator::toVec4(TypeDesc type, string& variable)
{
    if (type.isFloat3())
    {
        variable = "float4(" + variable + ", 1.0)";
    }
    else if (type.isFloat2())
    {
        variable = "float4(" + variable + ", 0.0, 1.0)";
    }
    else if (type == Type::FLOAT || type == Type::INTEGER)
    {
        variable = "float4(" + variable + ", " + variable + ", " + variable + ", 1.0)";
    }
    else if (type == Type::BSDF || type == Type::EDF)
    {
        variable = "float4(" + variable + ", 1.0)";
    }
    else
    {
        // Can't understand other types. Just return black.
        variable = "float4(0.0, 0.0, 0.0, 1.0)";
    }
}

void SlangShaderGenerator::SlangSyntaxFromGlsl(ShaderStage& shaderStage) const
{
    std::string sourceCode = shaderStage.getSourceCode();

    // Renames GLSL constructs that are used in shared code to Slang equivalent constructs.
    std::unordered_map<string, string> replaceTokens;
    replaceTokens["sampler2D"] = "SamplerTexture2D";
    replaceTokens["dFdy"] = "ddy";
    replaceTokens["dFdx"] = "ddx";
    replaceTokens["mix"] = "lerp";
    replaceTokens["fract"] = "frac";
    replaceTokens["vec2"] = "float2";
    replaceTokens["vec3"] = "float3";
    replaceTokens["vec4"] = "float4";
    replaceTokens["ivec2"] = "int2";
    replaceTokens["ivec3"] = "int3";
    replaceTokens["ivec4"] = "int4";
    replaceTokens["uvec2"] = "uint2";
    replaceTokens["uvec3"] = "uint3";
    replaceTokens["uvec4"] = "uint4";
    replaceTokens["bvec2"] = "bool2";
    replaceTokens["bvec3"] = "bool3";
    replaceTokens["bvec4"] = "bool4";
    replaceTokens["mat2"] = "float2x2";
    replaceTokens["mat3"] = "float3x3";
    replaceTokens["mat4"] = "float4x4";
    replaceTokens["gl_FragCoord.xy"] = "vd.SV_Position.xy";
    replaceTokens["gl_FragCoord.z"] = "vd.SV_Position.z";
    // Specific to microfacet_diffuse
    replaceTokens["const float FUJII_CONSTANT_1"] = "static const float FUJII_CONSTANT_1";
    replaceTokens["const float FUJII_CONSTANT_2"] = "static const float FUJII_CONSTANT_2";
    // Specific to microfacet_specular
    replaceTokens["const int FRESNEL_MODEL_DIELECTRIC"] = "static const int FRESNEL_MODEL_DIELECTRIC";
    replaceTokens["const int FRESNEL_MODEL_CONDUCTOR"] = "static const int FRESNEL_MODEL_CONDUCTOR";
    replaceTokens["const int FRESNEL_MODEL_SCHLICK"] = "static const int FRESNEL_MODEL_SCHLICK";
    // Specific to blackbody
    replaceTokens["const float3x3 XYZ_to_RGB"] = "static const float3x3 XYZ_to_RGB";

    size_t pos = 0;
    auto isAllowedAfterToken = [](char ch) -> bool
    {
        return std::isspace(ch) || ch == '(' || ch == ')' || ch == ',';
    };

    auto isAllowedBeforeToken = [](char ch) -> bool
    {
        return std::isspace(ch) || ch == '(' || ch == ',' || ch == '-' || ch == '+';
    };

    for (const auto& t : replaceTokens)
    {
        pos = sourceCode.find(t.first);
        while (pos != std::string::npos)
        {
            bool isOutKeyword = isAllowedBeforeToken(sourceCode[pos - 1]);
            size_t beg = pos;
            pos += t.first.length();
            isOutKeyword &= isAllowedAfterToken(sourceCode[pos]);

            if (isOutKeyword)
            {
                sourceCode.replace(beg, t.first.length(), t.second);
                pos = sourceCode.find(t.first, beg + t.second.length());
            }
            else
            {
                pos = sourceCode.find(t.first, beg + t.first.length());
            }
        }
    }

    shaderStage.setSourceCode(sourceCode);
}

void SlangShaderGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                                   GenContext&, ShaderStage& stage,
                                                   bool assignValue) const
{
    // A file texture input needs special handling on Slang
    if (variable->getType() == Type::FILENAME)
    {
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        emitString(str + "SamplerTexture2D " + variable->getVariable(), stage);
    }
    else
    {
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        // Varying parameters of type int must be flat qualified on output from vertex stage and
        // input to pixel stage. The only way to get these is with geompropvalue_integer nodes.
        if (qualifier.empty() && variable->getType() == Type::INTEGER && !assignValue && variable->getName().rfind(HW::T_IN_GEOMPROP, 0) == 0)
        {
            str += SlangSyntax::FLAT_QUALIFIER + " ";
        }
        str += _syntax->getTypeName(variable->getType()) + " " + variable->getVariable();

        // If an array we need an array qualifier (suffix) for the variable name
        if (variable->getType().isArray() && variable->getValue())
        {
            str += _syntax->getArrayVariableSuffix(variable->getType(), *variable->getValue());
        }

        if (!variable->getSemantic().empty())
        {
            str += " : " + variable->getSemantic();
        }

        if (assignValue && qualifier != _syntax->getUniformQualifier())
        {
            const string valueStr = (variable->getValue() ? _syntax->getValue(variable->getType(), *variable->getValue()) : _syntax->getDefaultValue(variable->getType()));
            str += valueStr.empty() ? EMPTY_STRING : " = " + valueStr;
        }

        emitString(str, stage);
    }
}

MATERIALX_NAMESPACE_END
