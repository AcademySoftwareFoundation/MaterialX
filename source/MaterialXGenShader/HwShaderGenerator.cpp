#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>
#include <MaterialXGenShader/Nodes/HwCompoundNode.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Definition.h>

namespace MaterialX
{

namespace HW
{
    const string PIXEL_STAGE      = MAIN_STAGE;
    const string VERTEX_STAGE     = "vertex";
    const string VERTEX_INPUTS    = "VertexInputs";
    const string VERTEX_DATA      = "VertexData";
    const string PRIVATE_UNIFORMS = "PrivateUniforms";
    const string PUBLIC_UNIFORMS  = "PublicUniforms";
    const string LIGHT_DATA       = "LightData";
    const string PIXEL_OUTPUTS    = "PixelOutputs";
    const string TRANSPARENT      = "transparent";
    const string CLOSURE_CONTEXT  = "ccx";
}

const string HwShaderGenerator::LIGHT_DIR = "L";
const string HwShaderGenerator::VIEW_DIR  = "V";
const string HwShaderGenerator::INCIDENT  = "incident";
const string HwShaderGenerator::OUTGOING  = "outgoing";
const string HwShaderGenerator::NORMAL    = "normal";
const string HwShaderGenerator::EVAL      = "eval";

HwShaderGenerator::HwShaderGenerator(SyntaxPtr syntax)
    : ShaderGenerator(syntax)
    , _maxActiveLightSources(3)
    , _reflection(HwClosureContext::REFLECTION)
    , _transmission(HwClosureContext::TRANSMISSION)
    , _indirect(HwClosureContext::INDIRECT)
    , _emission(HwClosureContext::EMISSION)
{
    // Reflection context
    _reflection.addArgument("vec3", INCIDENT);
    _reflection.addArgument("vec3", OUTGOING);
    _reflection.setSuffix("_reflection");

    //Transmission context
    _transmission.addArgument("vec3", OUTGOING);
    _transmission.setSuffix("_transmission");

    // Indirect context
    _indirect.addArgument("vec3", OUTGOING);
    _indirect.setSuffix("_indirect");

    // Emission context
    _emission.addArgument("vec3", NORMAL);
    _emission.addArgument("vec3", EVAL);
}

ShaderPtr HwShaderGenerator::createShader(const string& name, ElementPtr element, GenContext& context)
{
    // Create the root shader graph
    ShaderGraphPtr graph = ShaderGraph::create(nullptr, name, element, *this, context);
    ShaderPtr shader = std::make_shared<Shader>(name, graph);

    // Create vertex stage.
    ShaderStagePtr vs = createStage(*shader, HW::VERTEX_STAGE);
    vs->createInputBlock(HW::VERTEX_INPUTS, "i_vs");
    vs->createUniformBlock(HW::PRIVATE_UNIFORMS, "u_prv");
    vs->createUniformBlock(HW::PUBLIC_UNIFORMS, "u_pub");

    // Create required variables for vertex stage
    VariableBlock& vsInputs = vs->getInputBlock(HW::VERTEX_INPUTS);
    vsInputs.add(Type::VECTOR3, "i_position");
    VariableBlock& vsPrivateUniforms = vs->getUniformBlock(HW::PRIVATE_UNIFORMS);
    vsPrivateUniforms.add(Type::MATRIX44, "u_worldMatrix");
    vsPrivateUniforms.add(Type::MATRIX44, "u_viewProjectionMatrix");

    // Create pixel stage.
    ShaderStagePtr ps = createStage(*shader, HW::PIXEL_STAGE);
    VariableBlockPtr psOutputs = ps->createOutputBlock(HW::PIXEL_OUTPUTS, "o_ps");
    VariableBlockPtr psPrivateUniforms = ps->createUniformBlock(HW::PRIVATE_UNIFORMS, "u_prv");
    VariableBlockPtr psPublicUniforms = ps->createUniformBlock(HW::PUBLIC_UNIFORMS, "u_pub");
    VariableBlockPtr lightData = ps->createUniformBlock(HW::LIGHT_DATA, "u_lightData");
    lightData->add(Type::INTEGER, "type");

    // Add a block for data from vertex to pixel shader.
    addStageConnectorBlock(*vs, *ps, HW::VERTEX_DATA, "vd");

    // Create uniforms for environment lighting
    // Note: Generation of the rotation matrix using floating point math can result
    // in values which when output can't be consumed by a h/w shader, so
    // just setting explicit values here for now since the matrix is simple.
    // In general the values will need to be "sanitized" for hardware.
    const Matrix44 yRotationPI(-1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1, 0,
        0, 0, 0, 1);
    psPrivateUniforms->add(Type::MATRIX44, "u_envMatrix", EMPTY_STRING, Value::createValue<Matrix44>(yRotationPI));
    psPrivateUniforms->add(Type::FILENAME, "u_envIrradiance");
    psPrivateUniforms->add(Type::FILENAME, "u_envRadiance");
    psPrivateUniforms->add(Type::INTEGER, "u_envRadianceMips", EMPTY_STRING, Value::createValue<int>(1));
    psPrivateUniforms->add(Type::INTEGER, "u_envSamples", EMPTY_STRING, Value::createValue<int>(16));

    // Create uniforms for the published graph interface
    for (ShaderGraphInputSocket* inputSocket : graph->getInputSockets())
    {
        // Only for inputs that are connected/used internally,
        // and are editable by users.
        if (inputSocket->connections.size() && graph->isEditable(*inputSocket))
        {
            psPublicUniforms->add(inputSocket->type, inputSocket->variable, EMPTY_STRING, inputSocket->value, inputSocket->path);
        }
    }

    // Add the pixel stage output. This needs to be a color4 for rendering.
    // TODO: Improve this to support multiple outputs and other data types.
    const ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket();
    psOutputs->add(Type::COLOR4, outputSocket->variable);

    // Create shader variables for all nodes that need this.
    for (ShaderNode* node : graph->getNodes())
    {
        node->getImplementation().createVariables(*shader, *node, *this, context);
    }

    // For surface shaders we need light shaders
    if (graph->hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
    {
        // Create shader variables for all bound light shaders
        for (auto it : _boundLightShaders)
        {
            ShaderNode* node = it.second.get();
            node->getImplementation().createVariables(*shader, *node, *this, context);
        }
    }

    //
    // For image textures we need to convert filenames into uniforms (texture samplers).
    // Any unconnected filename input on file texture nodes needs to have a corresponding 
    // uniform.
    //

    // Start with top level graphs.
    std::deque<ShaderGraph*> graphQueue = { graph.get() };
    for (auto it : _boundLightShaders)
    {
        ShaderNode* node = it.second.get();
        ShaderGraph* lightGraph = node->getImplementation().getGraph();
        if (lightGraph)
        {
            graphQueue.push_back(lightGraph);
        }
    }

    while (!graphQueue.empty())
    {
        ShaderGraph* g = graphQueue.back();
        graphQueue.pop_back();

        for (ShaderNode* node : g->getNodes())
        {
            if (node->hasClassification(ShaderNode::Classification::FILETEXTURE))
            {
                for (ShaderInput* input : node->getInputs())
                {
                    if (!input->connection && input->type == Type::FILENAME)
                    {
                        // Create the uniform using the filename type to make this uniform into a texture sampler.
                        psPublicUniforms->add(Type::FILENAME, input->variable, EMPTY_STRING, input->value, input->path);

                        // Assing the uniform name to the input value
                        // so we can reference it duing code generation.
                        input->value = Value::createValue(input->variable);
                    }
                }
            }
            // Push subgraphs on the queue to process these as well.
            ShaderGraph* subgraph = node->getImplementation().getGraph();
            if (subgraph)
            {
                graphQueue.push_back(subgraph);
            }
        }
    }

    if (context.getOptions().hwTransparency)
    {
        // Flag the shader as being transparent.
        shader->setAttribute(HW::TRANSPARENT);
    }

    return shader;
}

void HwShaderGenerator::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, GenContext& context, bool checkScope)
{
    // Omit node if it's only used inside a conditional branch
    if (checkScope && node.referencedConditionally())
    {
        emitComment(stage, "Omitted node '" + node.getName() + "'. Only used in conditional node '" + 
                    node.getScopeInfo().conditionalNode->getName() + "'");
    }
    else
    {
        bool match = true;

        // Check if we have a closure context to modify the function call.
        const HwClosureContext* ccx = context.getUserData<HwClosureContext>(HW::CLOSURE_CONTEXT);

        if (ccx && node.hasClassification(ShaderNode::Classification::CLOSURE))
        {
            match =
                // For reflection and indirect we don't support pure transmissive closures.
                ((ccx->getType() == HwClosureContext::REFLECTION || ccx->getType() == HwClosureContext::INDIRECT) &&
                    node.hasClassification(ShaderNode::Classification::BSDF) &&
                    !node.hasClassification(ShaderNode::Classification::BSDF_T)) ||
                // For transmissive we don't support pure reflective closures.
                (ccx->getType() == HwClosureContext::TRANSMISSION &&
                    node.hasClassification(ShaderNode::Classification::BSDF) &&
                    !node.hasClassification(ShaderNode::Classification::BSDF_R)) ||
                // For emission we only support emission closures.
                (ccx->getType() == HwClosureContext::EMISSION &&
                    node.hasClassification(ShaderNode::Classification::EDF));
        }

        if (match)
        {
            // A match between closure context and node classification was found.
            // So emit the function call in this context.
            node.getImplementation().emitFunctionCall(stage, node, *this, context);
        }
        else
        {
            // Context and node classification doen't match so just
            // emit the output variable set to default value, in case
            // it is referenced by another nodes in this context.
            emitLineBegin(stage);
            emitOutput(stage, context, node.getOutput(), true, true);
            emitLineEnd(stage);
        }
    }
}

void HwShaderGenerator::bindLightShader(const NodeDef& nodeDef, unsigned int lightTypeId, GenContext& context)
{
    if (TypeDesc::get(nodeDef.getType()) != Type::LIGHTSHADER)
    {
        throw ExceptionShaderGenError("Error binding light shader. Given nodedef '" + nodeDef.getName() + "' is not of lightshader type");
    }

    if (getBoundLightShader(lightTypeId))
    {
        throw ExceptionShaderGenError("Error binding light shader. Light type id '" + std::to_string(lightTypeId) +
            "' has already been bound");
    }

    ShaderNodePtr lightShader = ShaderNode::create(nullptr, nodeDef.getNodeString(), nodeDef, *this, context);

    // Prepend the light struct instance name on all input socket variables, 
    // since in generated code these inputs will be members of the light struct.
    ShaderGraph* graph = lightShader->getImplementation().getGraph();
    if (graph)
    {
        for (ShaderGraphInputSocket* inputSockets : graph->getInputSockets())
        {
            inputSockets->variable = "light." + inputSockets->variable;
        }
    }

    _boundLightShaders[lightTypeId] = lightShader;
}

const ShaderNode* HwShaderGenerator::getBoundLightShader(unsigned int lightTypeId) const
{
    auto it = _boundLightShaders.find(lightTypeId);
    return it != _boundLightShaders.end() ? it->second.get() : nullptr;
}

void HwShaderGenerator::getClosureContexts(const ShaderNode& node, vector<const HwClosureContext*>& ccx) const
{
    if (node.hasClassification(ShaderNode::Classification::BSDF))
    {
        if (node.hasClassification(ShaderNode::Classification::BSDF_R))
        {
            // A BSDF for reflection only
            ccx.push_back(&_reflection);
            ccx.push_back(&_indirect);
        }
        else if (node.hasClassification(ShaderNode::Classification::BSDF_T))
        {
            // A BSDF for transmission only
            ccx.push_back(&_transmission);
        }
        else
        {
            // A general BSDF handling both reflection and transmission
            ccx.push_back(&_reflection);
            ccx.push_back(&_transmission);
            ccx.push_back(&_indirect);
        }
    }
    else if (node.hasClassification(ShaderNode::Classification::EDF))
    {
        // An EDF
        ccx.push_back(&_emission);
    }
}

ShaderNodeImplPtr HwShaderGenerator::createSourceCodeImplementation(ImplementationPtr impl)
{
    // The standard source code implementation
    // is the implementation to use by default
    return HwSourceCodeNode::create();
}

ShaderNodeImplPtr HwShaderGenerator::createCompoundImplementation(NodeGraphPtr impl)
{
    // The standard compound implementation
    // is the compound implementation to us by default
    return HwCompoundNode::create();
}

}
