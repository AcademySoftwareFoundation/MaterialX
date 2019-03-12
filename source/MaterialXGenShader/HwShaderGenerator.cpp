//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/HwShaderGenerator.h>

#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>
#include <MaterialXGenShader/Nodes/HwCompoundNode.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Definition.h>

namespace MaterialX
{

namespace Stage
{
    const string VERTEX = "vertex";
}

namespace HW
{
    const string VERTEX_INPUTS    = "VertexInputs";
    const string VERTEX_DATA      = "VertexData";
    const string PRIVATE_UNIFORMS = "PrivateUniforms";
    const string PUBLIC_UNIFORMS  = "PublicUniforms";
    const string LIGHT_DATA       = "LightData";
    const string PIXEL_OUTPUTS    = "PixelOutputs";
    const string NORMAL_DIR       = "N";
    const string LIGHT_DIR        = "L";
    const string VIEW_DIR         = "V";
    const string ATTR_TRANSPARENT = "transparent";
    const string USER_DATA_CLOSURE_CONTEXT = "udcc";
    const string USER_DATA_LIGHT_SHADERS   = "udls";
}

//
// HwShaderGenerator methods
//

HwShaderGenerator::HwShaderGenerator(SyntaxPtr syntax) :
    ShaderGenerator(syntax)
{
    // Create closure contexts for defining closure functions
    //
    // Reflection context
    _defReflection = HwClosureContext::create(HwClosureContext::REFLECTION);
    _defReflection->setSuffix("_reflection");
    _defReflection->addArgument(Type::VECTOR3, HW::LIGHT_DIR);
    _defReflection->addArgument(Type::VECTOR3, HW::VIEW_DIR);
    // Transmission context
    _defTransmission = HwClosureContext::create(HwClosureContext::TRANSMISSION);
    _defTransmission->setSuffix("_transmission");
    _defTransmission->addArgument(Type::VECTOR3, HW::VIEW_DIR);
    // Indirect context
    _defIndirect = HwClosureContext::create(HwClosureContext::INDIRECT);
    _defIndirect->setSuffix("_indirect");
    _defIndirect->addArgument(Type::VECTOR3, HW::VIEW_DIR);
    // Emission context
    _defEmission = HwClosureContext::create(HwClosureContext::EMISSION);
    _defEmission->addArgument(Type::VECTOR3, HW::NORMAL_DIR);
    _defEmission->addArgument(Type::VECTOR3, HW::LIGHT_DIR);
}

ShaderPtr HwShaderGenerator::createShader(const string& name, ElementPtr element, GenContext& context) const
{
    // Create the root shader graph
    ShaderGraphPtr graph = ShaderGraph::create(nullptr, name, element, context);
    ShaderPtr shader = std::make_shared<Shader>(name, graph);

    // Create vertex stage.
    ShaderStagePtr vs = createStage(Stage::VERTEX, *shader);
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
    ShaderStagePtr ps = createStage(Stage::PIXEL, *shader);
    VariableBlockPtr psOutputs = ps->createOutputBlock(HW::PIXEL_OUTPUTS, "o_ps");
    VariableBlockPtr psPrivateUniforms = ps->createUniformBlock(HW::PRIVATE_UNIFORMS, "u_prv");
    VariableBlockPtr psPublicUniforms = ps->createUniformBlock(HW::PUBLIC_UNIFORMS, "u_pub");
    VariableBlockPtr lightData = ps->createUniformBlock(HW::LIGHT_DATA, "u_lightData");
    lightData->add(Type::INTEGER, "type");

    // Add a block for data from vertex to pixel shader.
    addStageConnectorBlock(HW::VERTEX_DATA, "vd", *vs, *ps);

    // Create uniforms for environment lighting
    // Note: Generation of the rotation matrix using floating point math can result
    // in values which when output can't be consumed by a h/w shader, so
    // just setting explicit values here for now since the matrix is simple.
    // In general the values will need to be "sanitized" for hardware.
    const Matrix44 yRotationPI(-1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1, 0,
        0, 0, 0, 1);
    psPrivateUniforms->add(Type::MATRIX44, "u_envMatrix", Value::createValue<Matrix44>(yRotationPI));
    psPrivateUniforms->add(Type::FILENAME, "u_envIrradiance");
    psPrivateUniforms->add(Type::FILENAME, "u_envRadiance");
    psPrivateUniforms->add(Type::INTEGER, "u_envRadianceMips", Value::createValue<int>(1));
    psPrivateUniforms->add(Type::INTEGER, "u_envSamples", Value::createValue<int>(16));

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

    // Create shader variables for all nodes that need this.
    for (ShaderNode* node : graph->getNodes())
    {
        node->getImplementation().createVariables(*node, context, *shader);
    }

    HwLightShadersPtr lightShaders = context.getUserData<HwLightShaders>(HW::USER_DATA_LIGHT_SHADERS);

    // For surface shaders we need light shaders
    if (lightShaders && graph->hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
    {
        // Create shader variables for all bound light shaders
        for (auto it : lightShaders->get())
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
    std::deque<ShaderGraph*> graphQueue = { graph.get() };
    if (lightShaders)
    {
        for (auto it : lightShaders->get())
        {
            ShaderNode* node = it.second.get();
            ShaderGraph* lightGraph = node->getImplementation().getGraph();
            if (lightGraph)
            {
                graphQueue.push_back(lightGraph);
            }
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
                    if (!input->getConnection() && input->getType() == Type::FILENAME)
                    {
                        // Create the uniform using the filename type to make this uniform into a texture sampler.
                        ShaderPort* filename = psPublicUniforms->add(Type::FILENAME, input->getVariable(), input->getValue());
                        filename->setPath(input->getPath());

                        // Assing the uniform name to the input value
                        // so we can reference it during code generation.
                        input->setValue(Value::createValue(input->getVariable()));
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
        shader->setAttribute(HW::ATTR_TRANSPARENT);
    }

    return shader;
}

void HwShaderGenerator::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage, bool checkScope) const
{
    // Omit node if it's only used inside a conditional branch
    if (checkScope && node.referencedConditionally())
    {
        emitComment("Omitted node '" + node.getName() + "'. Only used in conditional node '" + 
                    node.getScopeInfo().conditionalNode->getName() + "'", stage);
    }
    else
    {
        bool match = true;

        // Check if we have a closure context to modify the function call.
        HwClosureContextPtr ccx = context.getUserData<HwClosureContext>(HW::USER_DATA_CLOSURE_CONTEXT);

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
            node.getImplementation().emitFunctionCall(node, context, stage);
        }
        else
        {
            // Context and node classification doen't match so just
            // emit the output variable set to default value, in case
            // it is referenced by another nodes in this context.
            emitLineBegin(stage);
            emitOutput(node.getOutput(), true, true, context, stage);
            emitLineEnd(stage);
        }
    }
}

void HwShaderGenerator::emitTextureNodes(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    // Emit function calls for all texturing nodes
    bool found = false;
    for (const ShaderNode* node : graph.getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::TEXTURE) && !node->referencedConditionally())
        {
            emitFunctionCall(*node, context, stage, false);
            found = true;
        }
    }

    if (found)
    {
        emitLineBreak(stage);
    }
}

void HwShaderGenerator::emitBsdfNodes(const ShaderGraph& graph, const ShaderNode& shaderNode, HwClosureContextPtr ccx,
                                      GenContext& context, ShaderStage& stage, string& bsdf) const
{
    bsdf = _syntax->getTypeSyntax(Type::BSDF).getDefaultValue(false);

    context.pushUserData(HW::USER_DATA_CLOSURE_CONTEXT, ccx);

    // Emit function calls for all BSDF nodes used by this surface shader.
    // The last node will hold the final result.
    const ShaderNode* last = nullptr;
    for (const ShaderNode* node : graph.getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::BSDF) && shaderNode.isUsedClosure(node))
        {
            emitFunctionCall(*node, context, stage, false);
            last = node;
        }
    }
    if (last)
    {
        bsdf = last->getOutput()->getVariable();
    }

    context.popUserData(HW::USER_DATA_CLOSURE_CONTEXT);
}

void HwShaderGenerator::emitEdfNodes(const ShaderGraph& graph, const ShaderNode& shaderNode, HwClosureContextPtr ccx,
                                     GenContext& context, ShaderStage& stage, string& edf) const
{
    edf = _syntax->getTypeSyntax(Type::EDF).getDefaultValue(false);

    context.pushUserData(HW::USER_DATA_CLOSURE_CONTEXT, ccx);

    // Emit function calls for all EDF nodes used by this shader
    // The last node will hold the final result
    const ShaderNode* last = nullptr;
    for (const ShaderNode* node : graph.getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::EDF) && shaderNode.isUsedClosure(node))
        {
            emitFunctionCall(*node, context, stage, false);
            last = node;
        }
    }
    if (last)
    {
        edf = last->getOutput()->getVariable();
    }

    context.popUserData(HW::USER_DATA_CLOSURE_CONTEXT);
}

void HwShaderGenerator::bindLightShader(const NodeDef& nodeDef, unsigned int lightTypeId, GenContext& context)
{
    if (TypeDesc::get(nodeDef.getType()) != Type::LIGHTSHADER)
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
            inputSockets->setVariable("light." + inputSockets->getVariable());
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

void HwShaderGenerator::getNodeClosureContexts(const ShaderNode& node, vector<HwClosureContextPtr>& ccx) const
{
    if (node.hasClassification(ShaderNode::Classification::BSDF))
    {
        if (node.hasClassification(ShaderNode::Classification::BSDF_R))
        {
            // A BSDF for reflection only
            ccx.push_back(_defReflection);
            ccx.push_back(_defIndirect);
        }
        else if (node.hasClassification(ShaderNode::Classification::BSDF_T))
        {
            // A BSDF for transmission only
            ccx.push_back(_defTransmission);
        }
        else
        {
            // A general BSDF handling both reflection and transmission
            ccx.push_back(_defReflection);
            ccx.push_back(_defTransmission);
            ccx.push_back(_defIndirect);
        }
    }
    else if (node.hasClassification(ShaderNode::Classification::EDF))
    {
        // An EDF
        ccx.push_back(_defEmission);
    }
}

ShaderNodeImplPtr HwShaderGenerator::createSourceCodeImplementation(const Implementation&) const
{
    // The standard source code implementation
    // is the implementation to use by default
    return HwSourceCodeNode::create();
}

ShaderNodeImplPtr HwShaderGenerator::createCompoundImplementation(const NodeGraph&) const
{
    // The standard compound implementation
    // is the compound implementation to us by default
    return HwCompoundNode::create();
}

} // namespace MaterialX
