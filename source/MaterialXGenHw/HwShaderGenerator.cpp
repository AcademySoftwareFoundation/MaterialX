//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHw/HwShaderGenerator.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenHw/HwLightShaders.h>
#include <MaterialXGenHw/Nodes/HwLightCompoundNode.h>
#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>

MATERIALX_NAMESPACE_BEGIN

//
// HwShaderGenerator methods
//

HwShaderGenerator::HwShaderGenerator(TypeSystemPtr typeSystem, SyntaxPtr syntax) :
    ShaderGenerator(typeSystem, syntax)
{
    // Assign default identifiers names for all tokens.
    // Derived generators can override these names.
    _tokenSubstitutions[HW::T_IN_POSITION] = HW::IN_POSITION;
    _tokenSubstitutions[HW::T_IN_NORMAL] = HW::IN_NORMAL;
    _tokenSubstitutions[HW::T_IN_TANGENT] = HW::IN_TANGENT;
    _tokenSubstitutions[HW::T_IN_BITANGENT] = HW::IN_BITANGENT;
    _tokenSubstitutions[HW::T_IN_TEXCOORD] = HW::IN_TEXCOORD;
    _tokenSubstitutions[HW::T_IN_GEOMPROP] = HW::IN_GEOMPROP;
    _tokenSubstitutions[HW::T_IN_COLOR] = HW::IN_COLOR;
    _tokenSubstitutions[HW::T_POSITION_WORLD] = HW::POSITION_WORLD;
    _tokenSubstitutions[HW::T_NORMAL_WORLD] = HW::NORMAL_WORLD;
    _tokenSubstitutions[HW::T_TANGENT_WORLD] = HW::TANGENT_WORLD;
    _tokenSubstitutions[HW::T_BITANGENT_WORLD] = HW::BITANGENT_WORLD;
    _tokenSubstitutions[HW::T_POSITION_OBJECT] = HW::POSITION_OBJECT;
    _tokenSubstitutions[HW::T_NORMAL_OBJECT] = HW::NORMAL_OBJECT;
    _tokenSubstitutions[HW::T_TANGENT_OBJECT] = HW::TANGENT_OBJECT;
    _tokenSubstitutions[HW::T_BITANGENT_OBJECT] = HW::BITANGENT_OBJECT;
    _tokenSubstitutions[HW::T_TEXCOORD] = HW::TEXCOORD;
    _tokenSubstitutions[HW::T_COLOR] = HW::COLOR;
    _tokenSubstitutions[HW::T_WORLD_MATRIX] = HW::WORLD_MATRIX;
    _tokenSubstitutions[HW::T_WORLD_INVERSE_MATRIX] = HW::WORLD_INVERSE_MATRIX;
    _tokenSubstitutions[HW::T_WORLD_TRANSPOSE_MATRIX] = HW::WORLD_TRANSPOSE_MATRIX;
    _tokenSubstitutions[HW::T_WORLD_INVERSE_TRANSPOSE_MATRIX] = HW::WORLD_INVERSE_TRANSPOSE_MATRIX;
    _tokenSubstitutions[HW::T_VIEW_MATRIX] = HW::VIEW_MATRIX;
    _tokenSubstitutions[HW::T_VIEW_INVERSE_MATRIX] = HW::VIEW_INVERSE_MATRIX;
    _tokenSubstitutions[HW::T_VIEW_TRANSPOSE_MATRIX] = HW::VIEW_TRANSPOSE_MATRIX;
    _tokenSubstitutions[HW::T_VIEW_INVERSE_TRANSPOSE_MATRIX] = HW::VIEW_INVERSE_TRANSPOSE_MATRIX;
    _tokenSubstitutions[HW::T_PROJ_MATRIX] = HW::PROJ_MATRIX;
    _tokenSubstitutions[HW::T_PROJ_INVERSE_MATRIX] = HW::PROJ_INVERSE_MATRIX;
    _tokenSubstitutions[HW::T_PROJ_TRANSPOSE_MATRIX] = HW::PROJ_TRANSPOSE_MATRIX;
    _tokenSubstitutions[HW::T_PROJ_INVERSE_TRANSPOSE_MATRIX] = HW::PROJ_INVERSE_TRANSPOSE_MATRIX;
    _tokenSubstitutions[HW::T_WORLD_VIEW_MATRIX] = HW::WORLD_VIEW_MATRIX;
    _tokenSubstitutions[HW::T_VIEW_PROJECTION_MATRIX] = HW::VIEW_PROJECTION_MATRIX;
    _tokenSubstitutions[HW::T_WORLD_VIEW_PROJECTION_MATRIX] = HW::WORLD_VIEW_PROJECTION_MATRIX;
    _tokenSubstitutions[HW::T_VIEW_POSITION] = HW::VIEW_POSITION;
    _tokenSubstitutions[HW::T_VIEW_DIRECTION] = HW::VIEW_DIRECTION;
    _tokenSubstitutions[HW::T_FRAME] = HW::FRAME;
    _tokenSubstitutions[HW::T_TIME] = HW::TIME;
    _tokenSubstitutions[HW::T_GEOMPROP] = HW::GEOMPROP;
    _tokenSubstitutions[HW::T_ALPHA_THRESHOLD] = HW::ALPHA_THRESHOLD;
    _tokenSubstitutions[HW::T_NUM_ACTIVE_LIGHT_SOURCES] = HW::NUM_ACTIVE_LIGHT_SOURCES;
    _tokenSubstitutions[HW::T_ENV_MATRIX] = HW::ENV_MATRIX;
    _tokenSubstitutions[HW::T_ENV_RADIANCE] = HW::ENV_RADIANCE;
    _tokenSubstitutions[HW::T_ENV_RADIANCE_SAMPLER2D] = HW::ENV_RADIANCE_SAMPLER2D;
    _tokenSubstitutions[HW::T_ENV_RADIANCE_MIPS] = HW::ENV_RADIANCE_MIPS;
    _tokenSubstitutions[HW::T_ENV_RADIANCE_SAMPLES] = HW::ENV_RADIANCE_SAMPLES;
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE] = HW::ENV_IRRADIANCE;
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE_SAMPLER2D] = HW::ENV_IRRADIANCE_SAMPLER2D;
    _tokenSubstitutions[HW::T_ENV_LIGHT_INTENSITY] = HW::ENV_LIGHT_INTENSITY;
    _tokenSubstitutions[HW::T_REFRACTION_TWO_SIDED] = HW::REFRACTION_TWO_SIDED;
    _tokenSubstitutions[HW::T_ALBEDO_TABLE] = HW::ALBEDO_TABLE;
    _tokenSubstitutions[HW::T_ALBEDO_TABLE_SIZE] = HW::ALBEDO_TABLE_SIZE;
    _tokenSubstitutions[HW::T_SHADOW_MAP] = HW::SHADOW_MAP;
    _tokenSubstitutions[HW::T_SHADOW_MATRIX] = HW::SHADOW_MATRIX;
    _tokenSubstitutions[HW::T_AMB_OCC_MAP] = HW::AMB_OCC_MAP;
    _tokenSubstitutions[HW::T_AMB_OCC_GAIN] = HW::AMB_OCC_GAIN;
    _tokenSubstitutions[HW::T_VERTEX_DATA_INSTANCE] = HW::VERTEX_DATA_INSTANCE;
    _tokenSubstitutions[HW::T_LIGHT_DATA_INSTANCE] = HW::LIGHT_DATA_INSTANCE;
    _tokenSubstitutions[HW::T_ENV_PREFILTER_MIP] = HW::ENV_PREFILTER_MIP;
    _tokenSubstitutions[HW::T_TEX_SAMPLER_SAMPLER2D] = HW::TEX_SAMPLER_SAMPLER2D;
    _tokenSubstitutions[HW::T_TEX_SAMPLER_SIGNATURE] = HW::TEX_SAMPLER_SIGNATURE;
    _tokenSubstitutions[HW::T_CLOSURE_DATA_CONSTRUCTOR] = HW::CLOSURE_DATA_CONSTRUCTOR;
}

ShaderPtr HwShaderGenerator::createShader(const string& name, ElementPtr element, GenContext& context) const
{
    // Create the root shader graph
    ShaderGraphPtr graph = ShaderGraph::create(nullptr, name, element, context);
    ShaderPtr shader = std::make_shared<Shader>(name, graph);

    // Check if there are inputs with default geomprops assigned. In order to bind the
    // corresponding data to these inputs we insert geomprop nodes in the graph.
    bool geomNodeAdded = false;
    for (ShaderGraphInputSocket* socket : graph->getInputSockets())
    {
        if (!socket->getGeomProp().empty())
        {
            ConstDocumentPtr doc = element->getDocument();
            GeomPropDefPtr geomprop = doc->getGeomPropDef(socket->getGeomProp());
            if (geomprop)
            {
                // A default geomprop was assigned to this graph input.
                // For all internal connections to this input, break the connection
                // and assign a geomprop node that generates this data.
                // Note: If a geomprop node exists already it is reused,
                // so only a single node per geometry type is created.
                ShaderInputVec connections = socket->getConnections();
                for (auto connection : connections)
                {
                    connection->breakConnection();
                    graph->addDefaultGeomNode(connection, *geomprop, context);
                    geomNodeAdded = true;
                }
            }
        }
    }
    // If nodes were added we need to re-sort the nodes in topological order.
    if (geomNodeAdded)
    {
        graph->topologicalSort();
    }

    // Create vertex stage.
    ShaderStagePtr vs = createStage(Stage::VERTEX, *shader);
    vs->createInputBlock(HW::VERTEX_INPUTS, "i_vs");

    // Each Stage must have three types of uniform blocks:
    // Private, Public and Sampler blocks
    // Public uniforms are inputs that should be published in a user interface for user interaction,
    // while private uniforms are internal variables needed by the system which should not be exposed in UI.
    // So when creating these uniforms for a shader node, if the variable is user-facing it should go into the public block,
    // and otherwise the private block.
    // All texture based objects should be added to Sampler block

    vs->createUniformBlock(HW::PRIVATE_UNIFORMS, "u_prv");
    vs->createUniformBlock(HW::PUBLIC_UNIFORMS, "u_pub");

    // Create required variables for vertex stage
    VariableBlock& vsInputs = vs->getInputBlock(HW::VERTEX_INPUTS);
    vsInputs.add(Type::VECTOR3, HW::T_IN_POSITION);
    VariableBlock& vsPrivateUniforms = vs->getUniformBlock(HW::PRIVATE_UNIFORMS);
    vsPrivateUniforms.add(Type::MATRIX44, HW::T_WORLD_MATRIX);
    vsPrivateUniforms.add(Type::MATRIX44, HW::T_VIEW_PROJECTION_MATRIX);

    // Create pixel stage.
    ShaderStagePtr ps = createStage(Stage::PIXEL, *shader);
    VariableBlockPtr psOutputs = ps->createOutputBlock(HW::PIXEL_OUTPUTS, "o_ps");

    // Create required Uniform blocks and any additional blocks if needed.
    VariableBlockPtr psPrivateUniforms = ps->createUniformBlock(HW::PRIVATE_UNIFORMS, "u_prv");
    VariableBlockPtr psPublicUniforms = ps->createUniformBlock(HW::PUBLIC_UNIFORMS, "u_pub");
    VariableBlockPtr lightData = ps->createUniformBlock(HW::LIGHT_DATA, HW::T_LIGHT_DATA_INSTANCE);
    lightData->add(Type::INTEGER, getLightDataTypevarString());

    // Add a block for data from vertex to pixel shader.
    addStageConnectorBlock(HW::VERTEX_DATA, HW::T_VERTEX_DATA_INSTANCE, *vs, *ps);

    // Add uniforms for transparent rendering.
    if (context.getOptions().hwTransparency)
    {
        psPrivateUniforms->add(Type::FLOAT, HW::T_ALPHA_THRESHOLD, Value::createValue(0.001f));
    }

    // Add uniforms for shadow map rendering.
    if (context.getOptions().hwShadowMap)
    {
        psPrivateUniforms->add(Type::FILENAME, HW::T_SHADOW_MAP);
        psPrivateUniforms->add(Type::MATRIX44, HW::T_SHADOW_MATRIX, Value::createValue(Matrix44::IDENTITY));
    }

    // Add inputs and uniforms for ambient occlusion.
    if (context.getOptions().hwAmbientOcclusion)
    {
        addStageInput(HW::VERTEX_INPUTS, Type::VECTOR2, HW::T_IN_TEXCOORD + "_0", *vs);
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR2, HW::T_TEXCOORD + "_0", *vs, *ps);
        psPrivateUniforms->add(Type::FILENAME, HW::T_AMB_OCC_MAP);
        psPrivateUniforms->add(Type::FLOAT, HW::T_AMB_OCC_GAIN, Value::createValue(1.0f));
    }

    // Add uniforms for environment lighting.
    if (requiresLighting(*graph) && context.getOptions().hwSpecularEnvironmentMethod != SPECULAR_ENVIRONMENT_NONE)
    {
        const Matrix44 yRotationPI = Matrix44::createScale(Vector3(-1, 1, -1));
        psPrivateUniforms->add(Type::MATRIX44, HW::T_ENV_MATRIX, Value::createValue(yRotationPI));
        psPrivateUniforms->add(Type::FILENAME, HW::ENV_RADIANCE); // Use direct value instead of HW::T_ENV_RADIANCE
        psPrivateUniforms->add(Type::FLOAT, HW::T_ENV_LIGHT_INTENSITY, Value::createValue(1.0f));
        psPrivateUniforms->add(Type::INTEGER, HW::T_ENV_RADIANCE_MIPS, Value::createValue<int>(1));
        psPrivateUniforms->add(Type::INTEGER, HW::T_ENV_RADIANCE_SAMPLES, Value::createValue<int>(16));
        psPrivateUniforms->add(Type::FILENAME, HW::ENV_IRRADIANCE); // Use direct value instead of HW::T_ENV_IRRADIANCE
        psPrivateUniforms->add(Type::BOOLEAN, HW::T_REFRACTION_TWO_SIDED);
    }

    // Add uniforms for the directional albedo table.
    if (context.getOptions().hwDirectionalAlbedoMethod == DIRECTIONAL_ALBEDO_TABLE ||
        context.getOptions().hwWriteAlbedoTable)
    {
        psPrivateUniforms->add(Type::FILENAME, HW::T_ALBEDO_TABLE);
        psPrivateUniforms->add(Type::INTEGER, HW::T_ALBEDO_TABLE_SIZE, Value::createValue<int>(64));
    }

    // Add uniforms for environment prefiltering.
    if (context.getOptions().hwWriteEnvPrefilter)
    {
        psPrivateUniforms->add(Type::FILENAME, HW::ENV_RADIANCE); // Use direct value instead of HW::T_ENV_RADIANCE
        psPrivateUniforms->add(Type::FLOAT, HW::T_ENV_LIGHT_INTENSITY, Value::createValue(1.0f));
        psPrivateUniforms->add(Type::INTEGER, HW::T_ENV_PREFILTER_MIP, Value::createValue<int>(1));
        const Matrix44 yRotationPI = Matrix44::createScale(Vector3(-1, 1, -1));
        psPrivateUniforms->add(Type::MATRIX44, HW::T_ENV_MATRIX, Value::createValue(yRotationPI));
        psPrivateUniforms->add(Type::INTEGER, HW::T_ENV_RADIANCE_MIPS, Value::createValue<int>(1));
    }

    // Create uniforms for the published graph interface
    for (ShaderGraphInputSocket* inputSocket : graph->getInputSockets())
    {
        // Only for inputs that are connected/used internally,
        // and are editable by users.
        if (!inputSocket->getConnections().empty() && graph->isEditable(*inputSocket))
        {
            psPublicUniforms->add(inputSocket->getSelf());
        }
    }

    // Add the pixel stage output. This needs to be a color4 for rendering,
    // so copy name and variable from the graph output but set type to color4.
    // TODO: Improve this to support multiple outputs and other data types.
    ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket();
    ShaderPort* output = psOutputs->add(Type::COLOR4, outputSocket->getName());
    output->setVariable(outputSocket->getVariable());
    output->setPath(outputSocket->getPath());

    // Create shader variables for all nodes that need this.
    createVariables(graph, context, *shader);

    HwLightShadersPtr lightShaders = context.getUserData<HwLightShaders>(HW::USER_DATA_LIGHT_SHADERS);

    // For surface shaders we need light shaders
    if (lightShaders && graph->hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
    {
        // Create shader variables for all bound light shaders
        for (const auto& it : lightShaders->get())
        {
            ShaderNode* node = it.second.get();
            node->getImplementation().createVariables(*node, context, *shader);
        }
    }

    //
    // For image textures we need to convert filenames into uniforms (texture samplers).
    // Any unconnected filename input on file texture nodes needs to have a corresponding
    // uniform.
    //

    // Start with top level graphs.
    vector<ShaderGraph*> graphStack = { graph.get() };
    if (lightShaders)
    {
        for (const auto& it : lightShaders->get())
        {
            ShaderNode* node = it.second.get();
            ShaderGraph* lightGraph = node->getImplementation().getGraph();
            if (lightGraph)
            {
                graphStack.push_back(lightGraph);
            }
        }
    }

    while (!graphStack.empty())
    {
        ShaderGraph* g = graphStack.back();
        graphStack.pop_back();

        for (ShaderNode* node : g->getNodes())
        {
            if (node->hasClassification(ShaderNode::Classification::FILETEXTURE))
            {
                for (ShaderInput* input : node->getInputs())
                {
                    if (!input->getConnection() && input->getType() == Type::FILENAME)
                    {
                        // Create the uniform using the filename type to make this uniform into a texture sampler.
                        ShaderPort* filename = psPublicUniforms->add(Type::FILENAME, input->getVariable(), input->getValue());
                        filename->setPath(input->getPath());

                        // Assign the uniform name to the input value
                        // so we can reference it during code generation.
                        input->setValue(Value::createValue(input->getVariable()));
                    }
                }
            }
            // Push subgraphs on the stack to process these as well.
            ShaderGraph* subgraph = node->getImplementation().getGraph();
            if (subgraph)
            {
                graphStack.push_back(subgraph);
            }
        }
    }

    if (context.getOptions().hwTransparency)
    {
        // Flag the shader as being transparent.
        shader->setAttribute(HW::ATTR_TRANSPARENT);
    }

    return shader;
}

bool HwShaderGenerator::requiresLighting(const ShaderGraph& graph) const
{
    const bool isBsdf = graph.hasClassification(ShaderNode::Classification::BSDF);
    const bool isLitSurfaceShader = graph.hasClassification(ShaderNode::Classification::SHADER) &&
                                    graph.hasClassification(ShaderNode::Classification::SURFACE) &&
                                    !graph.hasClassification(ShaderNode::Classification::UNLIT);
    return isBsdf || isLitSurfaceShader;
}

void HwShaderGenerator::bindLightShader(const NodeDef& nodeDef, unsigned int lightTypeId, GenContext& context)
{
    if (context.getTypeDesc(nodeDef.getType()) != Type::LIGHTSHADER)
    {
        throw ExceptionShaderGenError("Error binding light shader. Given nodedef '" + nodeDef.getName() + "' is not of lightshader type");
    }

    HwLightShadersPtr lightShaders = context.getUserData<HwLightShaders>(HW::USER_DATA_LIGHT_SHADERS);
    if (!lightShaders)
    {
        lightShaders = HwLightShaders::create();
        context.pushUserData(HW::USER_DATA_LIGHT_SHADERS, lightShaders);
    }

    if (lightShaders->get(lightTypeId))
    {
        throw ExceptionShaderGenError("Error binding light shader. Light type id '" + std::to_string(lightTypeId) +
                                      "' has already been bound");
    }

    ShaderNodePtr shader = ShaderNode::create(nullptr, nodeDef.getNodeString(), nodeDef, context);

    // Check if this is a graph implementation.
    // If so prepend the light struct instance name on all input socket variables,
    // since in generated code these inputs will be members of the light struct.
    ShaderGraph* graph = shader->getImplementation().getGraph();
    if (graph)
    {
        for (ShaderGraphInputSocket* inputSockets : graph->getInputSockets())
        {
            inputSockets->setVariable("light." + inputSockets->getName());
        }
    }

    lightShaders->bind(lightTypeId, shader);
}

void HwShaderGenerator::unbindLightShader(unsigned int lightTypeId, GenContext& context)
{
    HwLightShadersPtr lightShaders = context.getUserData<HwLightShaders>(HW::USER_DATA_LIGHT_SHADERS);
    if (lightShaders)
    {
        lightShaders->unbind(lightTypeId);
    }
}

void HwShaderGenerator::unbindLightShaders(GenContext& context)
{
    HwLightShadersPtr lightShaders = context.getUserData<HwLightShaders>(HW::USER_DATA_LIGHT_SHADERS);
    if (lightShaders)
    {
        lightShaders->clear();
    }
}

bool HwShaderGenerator::nodeNeedsClosureData(const ShaderNode& node) const
{
    return (node.hasClassification(ShaderNode::Classification::BSDF) || node.hasClassification(ShaderNode::Classification::EDF) || node.hasClassification(ShaderNode::Classification::VDF));
}

void HwShaderGenerator::addStageLightingUniforms(GenContext& context, ShaderStage& stage) const
{
    // Create uniform for number of active light sources
    if (context.getOptions().hwMaxActiveLightSources > 0)
    {
        ShaderPort* numActiveLights = addStageUniform(HW::PRIVATE_UNIFORMS, Type::INTEGER, HW::T_NUM_ACTIVE_LIGHT_SOURCES, stage);
        numActiveLights->setValue(Value::createValue<int>(0));
    }
}
ShaderNodeImplPtr HwShaderGenerator::createShaderNodeImplForNodeGraph(const NodeGraph& nodegraph) const
{
    vector<OutputPtr> outputs = nodegraph.getActiveOutputs();
    if (outputs.empty())
    {
        throw ExceptionShaderGenError("NodeGraph '" + nodegraph.getName() + "' has no outputs defined");
    }

    const TypeDesc outputType = _typeSystem->getType(outputs[0]->getType());

    // Use a compound implementation.
    if (outputType == Type::LIGHTSHADER)
    {
        return HwLightCompoundNode::create();
    }
    return CompoundNode::create();
}

void HwShaderGenerator::emitClosureDataArg(const ShaderNode& node, GenContext& /*context*/, ShaderStage& stage) const
{
    if (nodeNeedsClosureData(node))
    {
        emitString(HW::CLOSURE_DATA_ARG + ", ", stage);
    }
}

void HwShaderGenerator::emitClosureDataParameter(const ShaderNode& node, GenContext& /*context*/, ShaderStage& stage) const
{
    if (nodeNeedsClosureData(node))
    {
        emitString(HW::CLOSURE_DATA_TYPE + " " + HW::CLOSURE_DATA_ARG + ", ", stage);
    }
}

void HwShaderGenerator::toVec4(TypeDesc type, string& variable) const
{
    const string& vec4 = _syntax->getTypeName(Type::VECTOR4);

    if (type.isFloat3())
    {
        variable = vec4+"(" + variable + ", 1.0)";
    }
    else if (type.isFloat2())
    {
        variable = vec4+"(" + variable + ", 0.0, 1.0)";
    }
    else if (type == Type::FLOAT || type == Type::INTEGER || type == Type::BOOLEAN)
    {
        variable = vec4+"(" + variable + ", " + variable + ", " + variable + ", 1.0)";
    }
    else if (type == Type::BSDF || type == Type::EDF)
    {
        variable = vec4+"(" + variable + ", 1.0)";
    }
    else
    {
        // Can't understand other types. Just return black.
        variable = vec4+"(0.0, 0.0, 0.0, 1.0)";
    }
}

MATERIALX_NAMESPACE_END
