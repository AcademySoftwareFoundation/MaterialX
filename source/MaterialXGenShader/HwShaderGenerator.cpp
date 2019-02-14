#include <MaterialXGenShader/HwShaderGenerator.h>

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
    const string LIGHT_UNIFORMS   = "LightUniforms";
    const string PIXEL_OUTPUTS    = "PixelOutputs";
    const string TRANSPARENCY     = "transparency";
}

HwShaderGenerator::HwShaderGenerator(SyntaxPtr syntax)
    : ShaderGenerator(syntax)
    , _maxActiveLightSources(3)
{
}

ShaderPtr HwShaderGenerator::create(const string& name, ElementPtr element, const GenOptions& options)
{
    ShaderGraphPtr graph = ShaderGraph::create(name, element, *this, options);
    ShaderPtr shader = std::make_shared<Shader>(name, graph);

    // Store transparency state in an attribute
    shader->setAttribute(HW::TRANSPARENCY, Value::createValue<bool>(options.hwTransparency));

    // Create vertex stage.
    ShaderStagePtr vs = createStage(*shader, HW::VERTEX_STAGE);
    vs->createInputBlock(HW::VERTEX_INPUTS, "i_app");
    vs->createUniformBlock(HW::PRIVATE_UNIFORMS, "u_prv");
    vs->createUniformBlock(HW::PUBLIC_UNIFORMS, "u_pub");

    // Create pixel stage.
    ShaderStagePtr ps = createStage(*shader, HW::PIXEL_STAGE);
    addStageConnectorBlock(*vs, *ps, HW::VERTEX_DATA, "vd");
    VariableBlockPtr psOutputs = ps->createOutputBlock(HW::PIXEL_OUTPUTS, "o_ps");
    VariableBlockPtr psPrivateUniforms = ps->createUniformBlock(HW::PRIVATE_UNIFORMS, "u_prv");
    VariableBlockPtr psPublicUniforms = ps->createUniformBlock(HW::PUBLIC_UNIFORMS, "u_pub");
    VariableBlockPtr lightData = ps->createUniformBlock(HW::LIGHT_UNIFORMS, "u_ld");
    lightData->add(Type::INTEGER, "type");

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
    psPrivateUniforms->add(Type::FILENAME, "u_envSpecular");
    psPrivateUniforms->add(Type::FILENAME, "u_envIrradiance");

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

    // Create outputs from the graph interface
    for (ShaderGraphOutputSocket* outputSocket : graph->getOutputSockets())
    {
        psOutputs->add(outputSocket->type, outputSocket->name);
    }

    // For both stages, create shader variables for all nodes 
    // that need this (geometric nodes / input streams).
    for (auto stage : { vs, ps })
    {
        for (ShaderNode* node : graph->getNodes())
        {
            ShaderNodeImpl* impl = node->getImplementation();
            impl->createVariables(*stage, *node);
        }
    }

    // For surface shaders we need light shaders
    if (graph->hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
    {
        // For both stages, create shader variables for all bound light shaders
        for (auto stage : { vs, ps })
        {
            for (auto it : _boundLightShaders)
            {
                it.second->createVariables(*stage, *ShaderNode::NONE);
            }
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
        ShaderGraph* lightGraph = it.second->getGraph();
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
            ShaderGraph* sg = node->getImplementation()->getGraph();
            if (sg)
            {
                graphQueue.push_back(sg);
            }
        }
    }

    return shader;
}

void HwShaderGenerator::bindLightShader(const NodeDef& nodeDef, size_t lightTypeId, const GenOptions& options)
{
    if (TypeDesc::get(nodeDef.getType()) != Type::LIGHTSHADER)
    {
        throw ExceptionShaderGenError("Error binding light shader. Given nodedef '" + nodeDef.getName() + "' is not of lightshader type");
    }

    if (getBoundLightShader(lightTypeId))
    {
        throw ExceptionShaderGenError("Error binding light shader. Light type id '" + std::to_string(lightTypeId) + "' has already been bound");
    }

    ShaderNodeImplPtr sgimpl;

    // Find the implementation for this nodedef
    InterfaceElementPtr impl = nodeDef.getImplementation(getTarget(), getLanguage());
    if (impl)
    {
        sgimpl = getImplementation(impl, options);
    }
    if (!sgimpl)
    {
        throw ExceptionShaderGenError("Could not find a matching implementation for node '" + nodeDef.getNodeString() +
            "' matching language '" + getLanguage() + "' and target '" + getTarget() + "'");
    }

    // Prepend the light struct instance name on all input socket variables, 
    // since in generated code these inputs will be members of the light struct.
    ShaderGraph* graph = sgimpl->getGraph();
    if (graph)
    {
        for (ShaderGraphInputSocket* inputSockets : graph->getInputSockets())
        {
            inputSockets->variable = "light." + inputSockets->variable;
        }
    }

    _boundLightShaders[lightTypeId] = sgimpl;
}

ShaderNodeImpl* HwShaderGenerator::getBoundLightShader(size_t lightTypeId)
{
    auto it = _boundLightShaders.find(lightTypeId);
    return it != _boundLightShaders.end() ? it->second.get() : nullptr;
}

}
