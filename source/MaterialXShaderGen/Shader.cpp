#include <MaterialXShaderGen/Shader.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Node.h>

#include <MaterialXShaderGen/ShaderGenRegistry.h>
#include <MaterialXShaderGen/Util.h>

#include <sstream>

namespace MaterialX
{

namespace
{
    string getLongName(const ElementPtr& element)
    {
        return element->getParent()->getName() + "_" + element->getName();
    }
}

Shader::Shader(const string& name)
    : _name(name)
    , _activeStage(0)
{
    _stages.resize(numStages());
}

void Shader::initialize(ElementPtr element, const string& language, const string& target)
{
    _activeStage = 0;
    _stages.resize(numStages());

    DocumentPtr doc = element->getDocument();

    // Create a new graph, to hold this node and it's dependencies upstream
    //
    // Disable notifications since this is an internal change.
    // TODO: Do we need a better solution for handling the shader generation graph?
    ScopedDisableNotifications disableNotifications(doc);

    string sgGraphName = "sg_" + _name;
    _nodeGraph = doc->getNodeGraph(sgGraphName);
    if (_nodeGraph)
    {
        doc->removeChild(sgGraphName);
    }
    _nodeGraph = doc->addNodeGraph(sgGraphName);
    _output = _nodeGraph->addOutput("out");

    //
    // Add the element itself to our graph
    // 

    // Keep track of the default geometric nodes we create below.
    unordered_map<string, NodePtr> defaultGeometricNodes;

    ElementPtr root;
    MaterialPtr material;

    if (element->isA<Output>())
    {
        OutputPtr output = element->asA<Output>();
        NodePtr srcNode = output->getConnectedNode();
        if (!srcNode)
        {
            ExceptionShaderGenError("Given output element '" + element->getName() + "' has no node connection");
        }

        NodePtr newNode = _nodeGraph->addNode(srcNode->getCategory(), getLongName(srcNode), srcNode->getType());
        newNode->copyContentFrom(srcNode);

        // Make sure there is a matching node def
        NodeDefPtr nodeDef = newNode->getReferencedNodeDef();
        if (!nodeDef)
        {
            throw ExceptionShaderGenError("No nodedef found for node '" + newNode->getCategory() + "' with type '" + newNode->getType() + "'");
        }

        // Connect any needed default geometric nodes
        addDefaultGeometricNodes(newNode, nodeDef, _nodeGraph);

        _output->setType(newNode->getType());
        _output->setNodeName(newNode->getName());
        _output->setChannels(output->getChannels());

        root = srcNode;
    }
    else if (element->isA<Node>())
    {
        NodePtr srcNode = element->asA<Node>();
        NodePtr newNode = _nodeGraph->addNode(srcNode->getCategory(), getLongName(srcNode), srcNode->getType());
        newNode->copyContentFrom(srcNode);

        // Make sure there is a matching node def
        NodeDefPtr nodeDef = newNode->getReferencedNodeDef();
        if (!nodeDef)
        {
            throw ExceptionShaderGenError("No nodedef found for node '" + newNode->getCategory() + "' with type '" + newNode->getType() + "'");
        }

        // Connect any needed default geometric nodes
        addDefaultGeometricNodes(newNode, nodeDef, _nodeGraph);

        _output->setType(newNode->getType());
        _output->setNodeName(newNode->getName());

        root = srcNode;
    }
    else if (element->isA<ShaderRef>())
    {
        ShaderRefPtr shaderRef = element->asA<ShaderRef>();
        NodeDefPtr nodeDef = shaderRef->getReferencedShaderDef();
        if (!nodeDef)
        {
            throw ExceptionShaderGenError("No nodedef found for shader node '" + shaderRef->getNode() + "'");
        }

        NodePtr newNode = _nodeGraph->addNode(nodeDef->getNode(), getLongName(shaderRef), nodeDef->getType());
        for (BindInputPtr bindInput : shaderRef->getBindInputs())
        {
            InputPtr input = newNode->addInput(bindInput->getName(), bindInput->getType());
            input->setValueString(bindInput->getValueString());
        }

        // Connect any needed default geometric nodes
        addDefaultGeometricNodes(newNode, nodeDef, _nodeGraph);

        _output->setType(newNode->getType());
        _output->setNodeName(newNode->getName());

        root = shaderRef;
        material = shaderRef->getParent()->asA<Material>();
    }

    if (!root)
    {
        throw ExceptionShaderGenError("Element '" + element->getName() + "' is not of supported type for shader generation");
    }

    //
    // Travers upstream to add all dependencies.
    // During this traversal we also add in any needed default geometric nodes.
    // 


    // Keep track of processed nodes to avoid duplication
    // of nodes with multiple downstream connections.
    std::set<NodePtr> processedNodes;

    for (Edge edge : root->traverseGraph(material))
    {
        ElementPtr upstreamElement = edge.getUpstreamElement();

        NodePtr upstreamNode = upstreamElement->isA<Output>() ?
            upstreamElement->asA<Output>()->getConnectedNode() : upstreamElement->asA<Node>();

        if (!upstreamNode)
        {
            // Unconnected
            continue;
        }

        NodePtr newNode;
        if (processedNodes.count(upstreamNode))
        {
            // Already processed so get the corresponding node in the new graph
            newNode = _nodeGraph->getNode(getLongName(upstreamNode));
        }
        else
        {
            // Create this node in the new graph.
            newNode = _nodeGraph->addNode(upstreamNode->getCategory(), getLongName(upstreamNode), upstreamNode->getType());
            newNode->copyContentFrom(upstreamNode);

            // Make sure there is a matching node def
            NodeDefPtr nodeDef = newNode->getReferencedNodeDef();
            if (!nodeDef)
            {
                throw ExceptionShaderGenError("No nodedef found for node '" + newNode->getCategory() + "' with type '" + newNode->getType() + "'");
            }

            // Connect any needed default geometric nodes
            addDefaultGeometricNodes(newNode, nodeDef, _nodeGraph);

            // Mark node as processed.
            processedNodes.insert(upstreamNode);
        }

        // Connect the node to downstream element in the new graph.
        ElementPtr downstreamElement = edge.getDownstreamElement();
        ElementPtr connectingElement = edge.getConnectingElement();
        if (downstreamElement->isA<Output>())
        {
            OutputPtr downstream = _nodeGraph->getOutput(downstreamElement->getName());
            downstream->setConnectedNode(newNode);
        }
        else if (connectingElement)
        {
            NodePtr downstream = _nodeGraph->getNode(getLongName(downstreamElement));
            downstream->setConnectedNode(connectingElement->getName(), newNode);
        }
    }

    // Create a flat version of the graph.
    _nodeGraph->flattenSubgraphs(target);

    // Create a topological ordering of the nodes
    vector<ElementPtr> topologicalOrder = _nodeGraph->topologicalSort();
    std::reverse(topologicalOrder.begin(), topologicalOrder.end());

    // Create an SgNode for each node, holding cached data for shader generation
    for (ElementPtr elem : topologicalOrder)
    {
        if (elem->isA<Node>())
        {
            _nodes.push_back(SgNode(elem->asA<Node>(), language, target));
        }
    }

    // Set the vdirection to use for texture nodes
    // Default is to use direction UP
    const string& vdir = element->getRoot()->getAttribute("vdirection");
    _vdirection = vdir == "down" ? VDirection::DOWN : VDirection::UP;;

#if 0

    for (size_t i = 0; i < _graph->numInputs(); ++i)
    {
        const Input* input = _graph->getInput(i);
        const UString& inputName = input->getName();
        if (usedInputInterface.find(inputName) != usedInputInterface.end())
        {
            _finalShaderInputs.push_back(std::make_pair(inputName.str(), input->asShared<const Input>()));
        }
    }

    if (_graph->numChildren() > 0)
    {
        // Find the topological order of the nodes
        NodeTraversalCallback callback = std::bind(nodeGetterCallback, std::placeholders::_1, std::ref(_topologicalOrder));
        _graph->getOutput()->traverseTopologic(callback, true);
        if (_topologicalOrder.empty())
        {
            throw CodeGenerationError("Code generation: Graph output '" + output->getFullName() + "' holds an empty graph ");
        }

        //
        // Calculate scopes for all nodes, considering branching from conditional nodes
        //

        // Fill the map with empty scope info's
        for (auto node : _topologicalOrder)
        {
            _scopeInfo[node->getName()] = ScopeInfo();
        }

        size_t lastNode = _topologicalOrder.size() - 1;
        const Node* node = _topologicalOrder[lastNode];
        _scopeInfo[node->getName()].type = ScopeInfo::Type::GLOBAL;

        UStringSet nodeUsed;
        nodeUsed.insert(node->getName());

        // Iterate nodes in reversed toplogical order such that every node is visited AFTER each of the nodes that depend on it have been processed first
        for (int nodeIndex = int(lastNode); nodeIndex >= 0; --nodeIndex)
        {
            node = _topologicalOrder[nodeIndex];
            const NodeClass* nodeClass = node->getNodeClass();

            // Once we visit a node the scopeInfo has been determined and it will not be changed
            // By then we have visited all the nodes that depend on it already
            if (!nodeClass || nodeUsed.find(node->getName()) == nodeUsed.end())
            {
                continue;
            }

            const bool isIfElse = nodeClass->isSet(NodeClassFlag::IFELSE_CONDITIONAL);
            const bool isSwitch = nodeClass->isSet(NodeClassFlag::SWITCH_CONDITIONAL);

            const ScopeInfo currentScopeInfo = _scopeInfo.at(node->getName());

            for (size_t plugIndex = 0; plugIndex < node->numInputs(); ++plugIndex)
            {
                const Input* plug = node->getInput(plugIndex);
                const Output* connected = plug->getConnection();
                if (connected && connected->type() != PlugType::INPUT_INTERFACE)
                {
                    const Node* upstreamNode = connected->getNode();

                    // Create scope info for this network brach
                    // If it's a conditonal branch the scope is adjusted
                    ScopeInfo newScopeInfo = currentScopeInfo;
                    if (isIfElse && (plugIndex == 1 || plugIndex == 2))
                    {
                        adjustAtConditionalInput(newScopeInfo, node, int(plugIndex), 0x6);
                    }
                    else if (isSwitch && plugIndex > 0)
                    {
                        const uint32_t fullMask = (1 << node->numInputs()) - 1;
                        adjustAtConditionalInput(newScopeInfo, node, int(plugIndex), fullMask);
                    }

                    // Add the info to the upstream node
                    ScopeInfo& upstreamScopeInfo = _scopeInfo.at(upstreamNode->getName());
                    merge(upstreamScopeInfo, newScopeInfo);

                    nodeUsed.insert(upstreamNode->getName());
                }
            }
        }
    }
#endif
}

void Shader::finalize()
{
    // Release resources
    if (_nodeGraph)
    {
        // Disable notifications since this is an internal change.
        // TODO: Do we need a better solution for handling the shader generation graph?
        ScopedDisableNotifications disableNotifications(_nodeGraph->getDocument());
        _nodeGraph->getDocument()->removeChild(_nodeGraph->getName());
    }
    _nodeGraph.reset();
    _output.reset();
    _nodes.clear();
}

void Shader::beginScope(Brackets brackets)
{
    Stage& s = stage();
        
    switch (brackets) {
    case Brackets::BRACES:
        indent();
        s.code += "{\n";
        break;
    case Brackets::PARENTHESES:
        indent();
        s.code += "(\n";
        break;
    case Brackets::SQUARES:
        indent();
        s.code += "[\n";
        break;
    case Brackets::NONE:
        break;
    }

    ++s.indentations;
    s.scopes.push(brackets);
}

void Shader::endScope(bool semicolon)
{
    Stage& s = stage();

    Brackets brackets = s.scopes.back();
    s.scopes.pop();
    --s.indentations;

    switch (brackets) {
    case Brackets::BRACES:
        indent();
        s.code += "}";
        break;
    case Brackets::PARENTHESES:
        indent();
        s.code += ")";
        break;
    case Brackets::SQUARES:
        indent();
        s.code += "]";
        break;
    case Brackets::NONE:
        break;
    }
    s.code += semicolon ? ";\n" : "\n";
}

void Shader::beginLine()
{
    indent();
}

void Shader::endLine(bool semicolon)
{
    stage().code += semicolon ? ";\n" : "\n";
}

void Shader::newLine()
{
    stage().code += "\n";
}

void Shader::addStr(const string& str)
{
    stage().code += str;
}

void Shader::addLine(const string& str, bool semicolon)
{
    beginLine();
    stage().code += str;
    endLine(semicolon);
}

void Shader::addBlock(const string& str)
{
    // Add each line in the block seperatelly
    // to get correct indentation
    std::stringstream stream(str);
    for (string line; std::getline(stream, line); )
    {
        addLine(line, false);
    }
}

void Shader::addInclude(const string& file)
{
    const string path = ShaderGenRegistry::findSourceCode(file);

    Stage& s = stage();
    if (s.includes.find(path) == s.includes.end())
    {
        string content;
        if (!readFile(path, content))
        {
            throw ExceptionShaderGenError("Could not find include file: '" + file + "'");
        }
        s.includes.insert(path);
        addBlock(content);
    }
}

void Shader::indent()
{
    static const string kIndent = "    ";
    Stage& s = stage();
    for (int i = 0; i < s.indentations; ++i)
    {
        s.code += kIndent;
    }
}

void Shader::addDefaultGeometricNodes(NodePtr node, NodeDefPtr nodeDef, NodeGraphPtr parent)
{
    // Add connections to default geometric nodes
    for (InputPtr inputDef : nodeDef->getInputs())
    {
        const string& defaultgeomprop = inputDef->getAttribute("defaultgeomprop");
        if (!defaultgeomprop.empty())
        {
            InputPtr input = node->getInput(inputDef->getName());
            if (!input)
            {
                input = node->addInput(inputDef->getName(), inputDef->getType());
            }

            if (input->getNodeName().empty())
            {
                const string geomNodeName = defaultgeomprop + "_default";
                NodePtr geomNode = parent->getNode(geomNodeName);
                if (!geomNode)
                {
                    geomNode = _nodeGraph->addNode(defaultgeomprop, geomNodeName, input->getType());
                }
                input->setNodeName(geomNode->getName());
            }
        }
    }
}

}
