//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenOsl/OslNetworkShaderGenerator.h>
#include <MaterialXGenOsl/OslNetworkSyntax.h>
#include <MaterialXGenOsl/Nodes/OsoNode.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/ShaderStage.h>

MATERIALX_NAMESPACE_BEGIN

const string OslNetworkShaderGenerator::TARGET = "genoslnetwork";

//
// OslNetworkShaderGenerator methods
//

OslNetworkShaderGenerator::OslNetworkShaderGenerator(TypeSystemPtr typeSystem) :
    OslShaderGenerator(typeSystem)
{
    _syntax = OslNetworkSyntax::create(typeSystem);
}

ShaderNodeImplPtr OslNetworkShaderGenerator::createShaderNodeImplForImplementation(const Implementation& /* implElement */) const
{
    return OsoNode::create();
}

static string paramString(const string& paramType, const string& paramName, const string& paramValue)
{
    return "param " + paramType + " " + paramName + " " + paramValue + " ;";
}

static string connectString(const string& fromNode, const string& fromName, const string& toNode, const string& toName)
{
    return "connect " + fromNode + "." + fromName + " " + toNode + "." + toName + " ;";
}

ShaderPtr OslNetworkShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);
    ShaderGraph& graph = shader->getGraph();
    ShaderStage& stage = shader->getStage(Stage::PIXEL);

    if (context.getOptions().oslConnectCiWrapper)
    {
        addSetCiTerminalNode(graph, element->getDocument(), context);
    }

    ConstDocumentPtr document = element->getDocument();

    string lastNodeName;
    ShaderOutput* lastOutput = nullptr;
    std::vector<string> connections;

    std::set<std::string> osoPaths;

    // Walk the node graph, emitting shaders and param declarations.
    for (auto&& node : graph.getNodes())
    {
        const string& nodeName = node->getName();

        for (auto&& input : node->getInputs())
        {
            string inputName = input->getName();
            _syntax->makeValidName(inputName);

            const ShaderOutput* connection = input->getConnection();
            if (!connection || connection->getNode() == &graph)
            {
                if (!input->hasAuthoredValue())
                    continue;

                if (input->getName() == "backsurfaceshader" || input->getName() == "displacementshader")
                    continue; // FIXME: these aren't getting pruned by hasAuthoredValue

                string value = _syntax->getValue(input);
                if (value == "null_closure()")
                    continue;

                // TODO: Figure out how to avoid special-casing struct-types in the generator, perhaps in the syntax?
                auto inputType = input->getType();
                if (inputType == Type::VECTOR2)
                {
                    auto parts = splitString(value, " ");
                    emitLine(paramString(_syntax->getTypeName(Type::FLOAT), inputName + ".x", parts[0]), stage, false);
                    emitLine(paramString(_syntax->getTypeName(Type::FLOAT), inputName + ".y", parts[1]), stage, false);
                }
                else if (inputType == Type::VECTOR4)
                {
                    auto parts = splitString(value, " ");
                    emitLine(paramString(_syntax->getTypeName(Type::FLOAT), inputName + ".x", parts[0]), stage, false);
                    emitLine(paramString(_syntax->getTypeName(Type::FLOAT), inputName + ".y", parts[1]), stage, false);
                    emitLine(paramString(_syntax->getTypeName(Type::FLOAT), inputName + ".z", parts[2]), stage, false);
                    emitLine(paramString(_syntax->getTypeName(Type::FLOAT), inputName + ".w", parts[3]), stage, false);
                }
                else if (inputType == Type::COLOR4)
                {
                    auto parts = splitString(value, " ");
                    emitLine(paramString(_syntax->getTypeName(Type::COLOR3), inputName + ".rgb", parts[0] + " " + parts[1] + " " + parts[2]), stage, false);
                    emitLine(paramString(_syntax->getTypeName(Type::FLOAT), inputName + ".a", parts[3]), stage, false);
                }
                else
                {
                    emitLine(paramString(_syntax->getTypeName(input->getType()), inputName, value), stage, false);
                }
            }
            else
            {
                string connName = connection->getName();
                _syntax->makeValidName(connName);

                string connect = connectString(connection->getNode()->getName(), connName, nodeName, inputName);
                // Save connect emits for the end, because they can't come
                // before both connected shaders have been declared.
                connections.push_back(connect);
            }
        }

        // Keep track of the root output, so we can connect it to our setCi node
        lastOutput = node->getOutput(0);

        const ShaderNodeImpl& impl = node->getImplementation();
        const OsoNode& osoNodeImpl = dynamic_cast<const OsoNode&>(impl);

        const string osoPath = osoNodeImpl.getOsoPath();
        osoPaths.insert(osoPath);

        emitLine("shader " + osoNodeImpl.getOsoName() + " " + nodeName + " ;", stage, false);
        lastNodeName = nodeName;
    }

    if (!lastOutput)
    {
        printf("Invalid shader\n");
        return nullptr;
    }

    for (auto&& connect : connections)
    {
        emitLine(connect, stage, false);
    }

    // From our set of required oso paths, build the path string that oslc will need.
    string osoPathStr;
    string separator = "";
    for (const auto& osoPath : osoPaths)
    {
        auto fullOsoPath = context.resolveSourceFile(osoPath, "");
        auto fullOsoPathStr = fullOsoPath.asString();

        osoPathStr += separator + fullOsoPathStr;
        separator = ",";
    }

    shader->setAttribute("osoPath", Value::createValue<string>(osoPathStr));

    return shader;
}

ShaderPtr OslNetworkShaderGenerator::createShader(const string& name, ElementPtr element, GenContext& context) const
{
    // Create the root shader graph
    ShaderGraphPtr graph = ShaderGraph::create(nullptr, name, element, context);
    ShaderPtr shader = std::make_shared<Shader>(name, graph);

    // Create our stage.
    ShaderStagePtr stage = createStage(Stage::PIXEL, *shader);
    stage->createUniformBlock(OSLNetwork::UNIFORMS);
    stage->createInputBlock(OSLNetwork::INPUTS);
    stage->createOutputBlock(OSLNetwork::OUTPUTS);

    // Create shader variables for all nodes that need this.
    createVariables(graph, context, *shader);

    // Create uniforms for the published graph interface.
    VariableBlock& uniforms = stage->getUniformBlock(OSLNetwork::UNIFORMS);
    for (ShaderGraphInputSocket* inputSocket : graph->getInputSockets())
    {
        // Only for inputs that are connected/used internally,
        // and are editable by users.
        if (inputSocket->getConnections().size() && graph->isEditable(*inputSocket))
        {
            uniforms.add(inputSocket->getSelf());
        }
    }

    // Create outputs from the graph interface.
    VariableBlock& outputs = stage->getOutputBlock(OSLNetwork::OUTPUTS);
    for (ShaderGraphOutputSocket* outputSocket : graph->getOutputSockets())
    {
        outputs.add(outputSocket->getSelf());
    }

    return shader;
}

namespace OSLNetwork
{

// Identifiers for OSL variable blocks
const string UNIFORMS = "u";
const string INPUTS = "i";
const string OUTPUTS = "o";

} // namespace OSLNetwork

MATERIALX_NAMESPACE_END
