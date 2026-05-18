//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenGlsl/GlslSyntax.h>
#include <MaterialXGenGlsl/Nodes/DisplacementNodeGlsl.h>

#include <MaterialXGenHw/HwLightShaders.h>
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

#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Nodes/MaterialNode.h>

#include <MaterialXTrace/Tracing.h>

#include <functional>
#include <set>

MATERIALX_NAMESPACE_BEGIN

const string GlslShaderGenerator::TARGET = "genglsl";
const string GlslShaderGenerator::VERSION = "400";

//
// GlslShaderGenerator methods
//

GlslShaderGenerator::GlslShaderGenerator(TypeSystemPtr typeSystem) :
    HwShaderGenerator(typeSystem, GlslSyntax::create(typeSystem))
{
    //
    // Register all custom node implementation classes
    //

    StringVec elementNames;

    // <!-- <position> -->
    registerImplementation("IM_position_vector3_" + GlslShaderGenerator::TARGET, HwPositionNode::create);
    // <!-- <normal> -->
    registerImplementation("IM_normal_vector3_" + GlslShaderGenerator::TARGET, HwNormalNode::create);
    // <!-- <tangent> -->
    registerImplementation("IM_tangent_vector3_" + GlslShaderGenerator::TARGET, HwTangentNode::create);
    // <!-- <bitangent> -->
    registerImplementation("IM_bitangent_vector3_" + GlslShaderGenerator::TARGET, HwBitangentNode::create);
    // <!-- <texcoord> -->
    registerImplementation("IM_texcoord_vector2_" + GlslShaderGenerator::TARGET, HwTexCoordNode::create);
    registerImplementation("IM_texcoord_vector3_" + GlslShaderGenerator::TARGET, HwTexCoordNode::create);
    // <!-- <geomcolor> -->
    registerImplementation("IM_geomcolor_float_" + GlslShaderGenerator::TARGET, HwGeomColorNode::create);
    registerImplementation("IM_geomcolor_color3_" + GlslShaderGenerator::TARGET, HwGeomColorNode::create);
    registerImplementation("IM_geomcolor_color4_" + GlslShaderGenerator::TARGET, HwGeomColorNode::create);
    // <!-- <geompropvalue> -->
    elementNames = {
        "IM_geompropvalue_integer_" + GlslShaderGenerator::TARGET,
        "IM_geompropvalue_float_" + GlslShaderGenerator::TARGET,
        "IM_geompropvalue_color3_" + GlslShaderGenerator::TARGET,
        "IM_geompropvalue_color4_" + GlslShaderGenerator::TARGET,
        "IM_geompropvalue_vector2_" + GlslShaderGenerator::TARGET,
        "IM_geompropvalue_vector3_" + GlslShaderGenerator::TARGET,
        "IM_geompropvalue_vector4_" + GlslShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, HwGeomPropValueNode::create);
    registerImplementation("IM_geompropvalue_boolean_" + GlslShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);
    registerImplementation("IM_geompropvalue_string_" + GlslShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);
    registerImplementation("IM_geompropvalue_filename_" + GlslShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);

    // <!-- <frame> -->
    registerImplementation("IM_frame_float_" + GlslShaderGenerator::TARGET, HwFrameNode::create);
    // <!-- <time> -->
    registerImplementation("IM_time_float_" + GlslShaderGenerator::TARGET, HwTimeNode::create);
    // <!-- <viewdirection> -->
    registerImplementation("IM_viewdirection_vector3_" + GlslShaderGenerator::TARGET, HwViewDirectionNode::create);

    // <!-- <surface> -->
    registerImplementation("IM_surface_" + GlslShaderGenerator::TARGET, HwSurfaceNode::create);

    // <!-- <displacement> -->
    registerImplementation("IM_displacement_float_" + GlslShaderGenerator::TARGET, DisplacementNodeGlsl::create);
    registerImplementation("IM_displacement_vector3_" + GlslShaderGenerator::TARGET, DisplacementNodeGlsl::create);

    // <!-- <light> -->
    registerImplementation("IM_light_" + GlslShaderGenerator::TARGET, HwLightNode::create);

    // <!-- <point_light> -->
    registerImplementation("IM_point_light_" + GlslShaderGenerator::TARGET, HwLightShaderNode::create);
    // <!-- <directional_light> -->
    registerImplementation("IM_directional_light_" + GlslShaderGenerator::TARGET, HwLightShaderNode::create);
    // <!-- <spot_light> -->
    registerImplementation("IM_spot_light_" + GlslShaderGenerator::TARGET, HwLightShaderNode::create);

    // <!-- <ND_transformpoint> ->
    registerImplementation("IM_transformpoint_vector3_" + GlslShaderGenerator::TARGET, HwTransformPointNode::create);

    // <!-- <ND_transformvector> ->
    registerImplementation("IM_transformvector_vector3_" + GlslShaderGenerator::TARGET, HwTransformVectorNode::create);

    // <!-- <ND_transformnormal> ->
    registerImplementation("IM_transformnormal_vector3_" + GlslShaderGenerator::TARGET, HwTransformNormalNode::create);

    // <!-- <image> -->
    elementNames = {
        "IM_image_float_" + GlslShaderGenerator::TARGET,
        "IM_image_color3_" + GlslShaderGenerator::TARGET,
        "IM_image_color4_" + GlslShaderGenerator::TARGET,
        "IM_image_vector2_" + GlslShaderGenerator::TARGET,
        "IM_image_vector3_" + GlslShaderGenerator::TARGET,
        "IM_image_vector4_" + GlslShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, HwImageNode::create);

    // <!-- <surfacematerial> -->
    registerImplementation("IM_surfacematerial_" + GlslShaderGenerator::TARGET, MaterialNode::create);

    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "numActiveLightSources", HwNumLightsNode::create()));
    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "sampleLightSource", HwLightSamplerNode::create()));
}

ShaderPtr GlslShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    MX_TRACE_FUNCTION(Tracing::Category::ShaderGen);
    MX_TRACE_SCOPE(Tracing::Category::ShaderGen, name.c_str());

    ShaderPtr shader = createShader(name, element, context);

    // Request fixed floating-point notation for consistency across targets.
    ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    // Make sure we initialize/reset the binding context before generation.
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->initialize();
    }

    // Set the include file for uv transformations early, before the vertex
    // stage, so displacement nodes using texture sampling can resolve it.
    if (context.getOptions().fileTextureVerticalFlip)
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "mx_transform_uv_vflip.glsl";
    }
    else
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "mx_transform_uv.glsl";
    }

    // Emit code for vertex shader stage
    ShaderStage& vs = shader->getStage(Stage::VERTEX);
    emitVertexStage(shader->getGraph(), context, vs);
    replaceTokens(_tokenSubstitutions, vs);

    // Emit code for pixel shader stage
    ShaderStage& ps = shader->getStage(Stage::PIXEL);
    emitPixelStage(shader->getGraph(), context, ps);
    replaceTokens(_tokenSubstitutions, ps);

    return shader;
}

void GlslShaderGenerator::emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);

    emitDirectives(context, stage);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->emitDirectives(context, stage);
    }
    emitLineBreak(stage);

    // Add type definitions (needed for displacementshader struct in vertex stage)
    emitTypeDefinitions(context, stage);

    // Add all constants
    emitConstants(context, stage);

    // Add all uniforms
    emitUniforms(context, stage);

    // Add vertex inputs
    emitInputs(context, stage);

    // Add vertex data outputs block
    emitOutputs(context, stage);

    // Add common math functions
    emitLibraryInclude("stdlib/genglsl/lib/mx_math.glsl", context, stage);
    emitLineBreak(stage);

    // Check for displacement nodes and collect dependency chain early —
    // needed for uniform filtering and function definitions.
    const ShaderNode* displacementNode = nullptr;
    const ShaderOutput* displacementOutput = nullptr;  // specific output for multioutput nodes
    std::set<const ShaderNode*> dispDeps;
    for (const ShaderNode* node : graph.getNodes())
    {
        if (node->getOutput()->getType() == Type::DISPLACEMENTSHADER)
        {
            // Skip default displacement nodes from surfacematerial's unconnected
            // inputs. Check that the displacement input connects to a real
            // computation node, not just a graph input socket.
            bool isDefault = true;
            const ShaderInput* dispIn = node->getInput("displacement");
            if (dispIn && dispIn->getConnection())
            {
                const ShaderNode* upstream = dispIn->getConnection()->getNode();
                // Real nodes have inputs with connections; graph input socket nodes don't
                if (upstream)
                {
                    for (ShaderInput* upIn : upstream->getInputs())
                    {
                        if (upIn->getConnection()) { isDefault = false; break; }
                    }
                }
            }
            if (!isDefault)
            {
                displacementNode = node;
                displacementOutput = node->getOutput();
                break;
            }
        }
        // Also check MaterialNode's displacementshader input — the displacement
        // may be connected through a nodedef compound node rather than being
        // a direct top-level node in the graph.
        if (node->getOutput()->getType() == Type::MATERIAL)
        {
            const ShaderInput* dispInput = node->getInput(ShaderNode::DISPLACEMENTSHADER);
            if (dispInput && dispInput->getConnection())
            {
                const ShaderNode* candidate = dispInput->getConnection()->getNode();
                // Skip self-connections and graph-node connections
                // (default unconnected displacement loops back via graph input sockets)
                if (!candidate || candidate == node || candidate == &graph) continue;
                if (candidate)
                {
                    // Check that the displacement node has real upstream computation.
                    // Default displacement nodes (from unconnected surfacematerial inputs)
                    // only connect to the graph's own input sockets.
                    bool hasRealInput = false;

                    // For compound/multioutput nodes (nodedef instances), displacement
                    // is inside the compound graph — just check if the node has
                    // a displacement output, which means it was authored.
                    if (candidate->numOutputs() > 1)
                    {
                        // Compound node: check if it has a displacement-related output
                        for (size_t oi = 0; oi < candidate->numOutputs(); ++oi)
                        {
                            if (candidate->getOutput(oi)->getType() == Type::DISPLACEMENTSHADER)
                            {
                                hasRealInput = true;
                                break;
                            }
                        }
                    }
                    else
                    {
                        // Default displacement nodes auto-created by the graph
                        // builder have names starting with their type name
                        // (e.g. "displacementshader1"). Authored nodes have
                        // user-defined names (e.g. "disp1", "needle_displacement").
                        const string& name = candidate->getName();
                        if (name.find("displacementshader") != 0)
                        {
                            hasRealInput = true;
                        }
                    }
                    if (hasRealInput)
                    {
                        displacementOutput = dispInput->getConnection();
                        displacementNode = candidate;
                        break;
                    }
                }
            }
        }
    }
    if (displacementNode)
    {
        std::function<void(const ShaderNode*)> collectDeps = [&](const ShaderNode* n) {
            if (dispDeps.count(n)) return;
            dispDeps.insert(n);
            for (ShaderInput* input : n->getInputs())
            {
                const ShaderNode* upstream = input->getConnectedSibling();
                if (upstream) collectDeps(upstream);
            }
        };
        collectDeps(displacementNode);

        // Emit public uniforms in the vertex shader so displacement
        // dependencies can access them. We emit all editable graph input
        // sockets as direct uniform declarations rather than adding to the
        // vertex stage's uniform block, because the uniform block approach
        // causes binding conflicts in GlslProgram::updateUniformsList().
        // Shared GLSL uniforms between vertex and pixel stages are fine —
        // they reference the same uniform location.
        // ESSL 300 (WebGL 2) does not support uniform initializers.
        const bool assignValues = (getVersion().find("es") == string::npos);
        emitComment("Public uniforms (shared with pixel stage)", stage);
        for (ShaderGraphInputSocket* inputSocket : graph.getInputSockets())
        {
            if (inputSocket->getConnections().empty() || !graph.isEditable(*inputSocket))
                continue;

            const TypeDesc& type = inputSocket->getType();

            // Skip types that can't be GLSL uniforms
            if (type.isClosure() || type.isStruct())
                continue;

            const string& qualifier = _syntax->getUniformQualifier();
            const string typeName = _syntax->getTypeName(type);

            // Texture/filename types become sampler2D uniforms — emit
            // without an initializer since their "value" is a file path.
            if (type.getSemantic() == TypeDesc::SEMANTIC_FILENAME)
            {
                emitLine(qualifier + " " + typeName + " " + inputSocket->getVariable(), stage);
                continue;
            }

            if (assignValues)
            {
                string valueStr;
                if (inputSocket->getValue())
                {
                    valueStr = _syntax->getValue(type, *inputSocket->getValue());
                }
                else
                {
                    valueStr = _syntax->getDefaultValue(type);
                }
                emitLine(qualifier + " " + typeName + " " + inputSocket->getVariable() +
                         (valueStr.empty() ? "" : " = " + valueStr), stage);
            }
            else
            {
                emitLine(qualifier + " " + typeName + " " + inputSocket->getVariable(), stage);
            }
        }
        emitLineBreak(stage);

        context.setEmitVertexDisplacement(true);

    }

    emitFunctionDefinitions(graph, context, stage);

    // Add main function
    setFunctionName("main", stage);
    emitLine("void main()", stage, false);
    emitFunctionBodyBegin(graph, context, stage);

    if (displacementNode)
    {
        // Enable vertex displacement flag so SourceCodeNode allows
        // emission in the vertex stage for displacement dependencies.
        context.setEmitVertexDisplacement(true);

        // Emit displacement dependency nodes in topological order.
        for (const ShaderNode* node : graph.getNodes())
        {
            if (dispDeps.count(node))
            {
                emitFunctionCall(*node, context, stage);
            }
        }

        context.setEmitVertexDisplacement(false);

        // Apply displacement along the vertex normal.
        // Float displacement stores the value in offset.z (via vec3(0,0,disp)).
        // Vector3 displacement uses the full offset directly.
        // Use the specific displacement output (important for multioutput compound nodes).
        const string& dispVar = displacementOutput->getVariable();
        emitComment("Apply vertex displacement along normal", stage);
        // Use the magnitude of the offset for normal-direction displacement.
        // For float displacement: offset = (0,0,d) → length = |d|, sign from d.
        // For vector3 displacement: offset = (dx,dy,dz) → applied as-is.
        const ShaderInput* dispInput = displacementNode->getInput("displacement");
        if (dispInput && dispInput->getType() == Type::FLOAT)
        {
            emitLine("vec3 displacedPosition = " + HW::T_IN_POSITION + " + " +
                     HW::T_IN_NORMAL + " * " + dispVar + ".offset.z * " + dispVar + ".scale", stage);
        }
        else
        {
            emitLine("vec3 displacedPosition = " + HW::T_IN_POSITION + " + " +
                     dispVar + ".offset * " + dispVar + ".scale", stage);
        }
        emitLine("vec4 hPositionWorld = " + HW::T_WORLD_MATRIX + " * vec4(displacedPosition, 1.0)", stage);
        emitLine("gl_Position = " + HW::T_VIEW_PROJECTION_MATRIX + " * hPositionWorld", stage);

        // Emit remaining nodes for vertex data connectors.
        // Displacement dep nodes were already emitted above. For compound
        // nodes that are in dispDeps, force re-emit their vertex data
        // connectors (normalWorld, positionWorld, etc.) since their first
        // emission only handled displacement-related internal nodes.
        for (const ShaderNode* node : graph.getNodes())
        {
            if (dispDeps.count(node))
            {
                // Force vertex data emission for compound nodes
                node->getImplementation().emitFunctionCall(*node, context, stage);
            }
            else
            {
                emitFunctionCall(*node, context, stage);
            }
        }
    }
    else
    {
        // Standard vertex position transformation.
        emitLine("vec4 hPositionWorld = " + HW::T_WORLD_MATRIX + " * vec4(" + HW::T_IN_POSITION + ", 1.0)", stage);
        emitLine("gl_Position = " + HW::T_VIEW_PROJECTION_MATRIX + " * hPositionWorld", stage);

        // Emit all function calls in order.
        for (const ShaderNode* node : graph.getNodes())
        {
            emitFunctionCall(*node, context, stage);
        }
    }

    context.setEmitVertexDisplacement(false);
    emitFunctionBodyEnd(graph, context, stage);
}

void GlslShaderGenerator::emitCommonMathLibrary(GenContext& context, ShaderStage& stage) const
{
    emitLibraryInclude("stdlib/genglsl/lib/mx_math.glsl", context, stage);
}

void GlslShaderGenerator::emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const
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

void GlslShaderGenerator::emitTransmissionRender(GenContext& context, ShaderStage& stage) const
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

void GlslShaderGenerator::emitDirectives(GenContext&, ShaderStage& stage) const
{
    emitLine("#version " + getVersion(), stage, false);
    emitLineBreak(stage);
}

void GlslShaderGenerator::emitConstants(GenContext& context, ShaderStage& stage) const
{
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), Syntax::SEMICOLON, context, stage);
        emitLineBreak(stage);
    }
}

void GlslShaderGenerator::emitUniforms(GenContext& context, ShaderStage& stage) const
{
    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;

        // Skip light uniforms as they are handled separately
        if (!uniforms.empty() && uniforms.getName() != HW::LIGHT_DATA)
        {
            emitComment("Uniform block: " + uniforms.getName(), stage);
            HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
            if (resourceBindingCtx)
            {
                resourceBindingCtx->emitResourceBindings(context, uniforms, stage);
            }
            else
            {
                emitVariableDeclarations(uniforms, _syntax->getUniformQualifier(), Syntax::SEMICOLON, context, stage);
                emitLineBreak(stage);
            }
        }
    }
}

void GlslShaderGenerator::emitLightData(GenContext& context, ShaderStage& stage) const
{
    const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
    const string structArraySuffix = "[" + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + "]";
    const string structName = lightData.getInstance();
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->emitStructuredResourceBindings(
            context, lightData, stage, structName, structArraySuffix);
    }
    else
    {
        emitLine("struct " + lightData.getName(), stage, false);
        emitScopeBegin(stage);
        emitVariableDeclarations(lightData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
        emitScopeEnd(stage, true);
        emitLineBreak(stage);
        emitLine("uniform " + lightData.getName() + " " + structName + structArraySuffix, stage);
    }
    emitLineBreak(stage);
}

void GlslShaderGenerator::emitInputs(GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
        if (!vertexInputs.empty())
        {
            emitComment("Inputs block: " + vertexInputs.getName(), stage);
            emitVariableDeclarations(vertexInputs, _syntax->getInputQualifier(), Syntax::SEMICOLON, context, stage, false);
            emitLineBreak(stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        if (!vertexData.empty())
        {
            emitLine("in " + vertexData.getName(), stage, false);
            emitScopeBegin(stage);
            emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
            emitScopeEnd(stage, false, false);
            emitString(" " + vertexData.getInstance() + Syntax::SEMICOLON, stage);
            emitLineBreak(stage);
            emitLineBreak(stage);
        }
    }
}

void GlslShaderGenerator::emitOutputs(GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        if (!vertexData.empty())
        {
            emitLine("out " + vertexData.getName(), stage, false);
            emitScopeBegin(stage);
            emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
            emitScopeEnd(stage, false, false);
            emitString(" " + vertexData.getInstance() + Syntax::SEMICOLON, stage);
            emitLineBreak(stage);
            emitLineBreak(stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        emitComment("Pixel shader outputs", stage);
        const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
        emitVariableDeclarations(outputs, _syntax->getOutputQualifier(), Syntax::SEMICOLON, context, stage, false);
        emitLineBreak(stage);
    }
}

HwResourceBindingContextPtr GlslShaderGenerator::getResourceBindingContext(GenContext& context) const
{
    return context.getUserData<HwResourceBindingContext>(HW::USER_DATA_BINDING_CONTEXT);
}

string GlslShaderGenerator::getVertexDataPrefix(const VariableBlock& vertexData) const
{
    return vertexData.getInstance() + ".";
}

void GlslShaderGenerator::emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);

    // Add directives
    emitDirectives(context, stage);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->emitDirectives(context, stage);
    }
    emitLineBreak(stage);

    // Add type definitions
    emitTypeDefinitions(context, stage);

    // Add all constants
    emitConstants(context, stage);

    // Add all uniforms
    emitUniforms(context, stage);

    // Add vertex data inputs block
    emitInputs(context, stage);

    // Add the pixel shader output. This needs to be a vec4 for rendering
    // and upstream connection will be converted to vec4 if needed in emitFinalOutput()
    emitOutputs(context, stage);

    // Add common math functions
    emitCommonMathLibrary(context, stage);
    emitLineBreak(stage);

    // Determine whether lighting is required
    bool lighting = requiresLighting(graph);

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
        if (context.getOptions().hwMaxActiveLightSources > 0)
        {
            const unsigned int maxLights = std::max(1u, context.getOptions().hwMaxActiveLightSources);
            emitLine("#define " + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + " " + std::to_string(maxLights), stage, false);
        }
        emitSpecularEnvironment(context, stage);
        emitTransmissionRender(context, stage);

        if (context.getOptions().hwMaxActiveLightSources > 0)
        {
            emitLightData(context, stage);
        }
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

    emitLightFunctionDefinitions(graph, context, stage);

    // Emit function definitions for all nodes in the graph.
    emitFunctionDefinitions(graph, context, stage);

    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();

    // Add main function
    setFunctionName("main", stage);
    emitLine("void main()", stage, false);
    emitFunctionBodyBegin(graph, context, stage);

    if (graph.hasClassification(ShaderNode::Classification::CLOSURE) &&
        !graph.hasClassification(ShaderNode::Classification::SHADER))
    {
        // Handle the case where the graph is a direct closure.
        // We don't support rendering closures without attaching
        // to a surface shader, so just output black.
        emitLine(outputSocket->getVariable() + " = vec4(0.0, 0.0, 0.0, 1.0)", stage);
    }
    else if (context.getOptions().hwWriteDepthMoments)
    {
        emitLine(outputSocket->getVariable() + " = vec4(mx_compute_depth_moments(), 0.0, 1.0)", stage);
    }
    else if (context.getOptions().hwWriteAlbedoTable)
    {
        emitLine(outputSocket->getVariable() + " = vec4(mx_generate_dir_albedo_table(), 1.0)", stage);
    }
    else if (context.getOptions().hwWriteEnvPrefilter)
    {
        emitLine(outputSocket->getVariable() + " = vec4(mx_generate_prefilter_env(), 1.0)", stage);
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
                    emitLine("float outAlpha = clamp(1.0 - dot(" + outTransparency + ", vec3(0.3333)), 0.0, 1.0)", stage);
                    emitLine(outputSocket->getVariable() + " = vec4(" + outColor + ", outAlpha)", stage);
                    emitLine("if (outAlpha < " + HW::T_ALPHA_THRESHOLD + ")", stage, false);
                    emitScopeBegin(stage);
                    emitLine("discard", stage);
                    emitScopeEnd(stage);
                }
                else
                {
                    emitLine(outputSocket->getVariable() + " = vec4(" + outColor + ", 1.0)", stage);
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
            string outputValue = outputSocket->getValue() ?
                                 _syntax->getValue(outputSocket->getType(), *outputSocket->getValue()) :
                                 _syntax->getDefaultValue(outputSocket->getType());
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

    // End main function
    emitFunctionBodyEnd(graph, context, stage);
}

void GlslShaderGenerator::emitLightFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
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

void GlslShaderGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                                  GenContext&, ShaderStage& stage,
                                                  bool assignValue) const
{
    // A file texture input needs special handling on GLSL
    if (variable->getType() == Type::FILENAME)
    {
        // Samplers must always be uniforms
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        emitString(str + "sampler2D " + variable->getVariable(), stage);
    }
    else
    {
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        // Varying parameters of type int must be flat qualified on output from vertex stage and
        // input to pixel stage. The only way to get these is with geompropvalue_integer nodes.
        if (qualifier.empty() && variable->getType() == Type::INTEGER && !assignValue && variable->getName().rfind(HW::T_IN_GEOMPROP, 0) == 0)
        {
            str += GlslSyntax::FLAT_QUALIFIER + " ";
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

        if (assignValue)
        {
            const string valueStr = (variable->getValue() ?
                                    _syntax->getValue(variable->getType(), *variable->getValue(), true) :
                                    _syntax->getDefaultValue(variable->getType(), true));
            str += valueStr.empty() ? EMPTY_STRING : " = " + valueStr;
        }

        emitString(str, stage);
    }
}

MATERIALX_NAMESPACE_END
