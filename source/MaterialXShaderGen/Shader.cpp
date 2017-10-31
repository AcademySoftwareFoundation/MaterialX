#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenRegistry.h>
#include <MaterialXShaderGen/Syntax.h>
#include <MaterialXShaderGen/Util.h>
#include <MaterialXShaderGen/NodeImplementations/Compare.h>
#include <MaterialXShaderGen/NodeImplementations/Switch.h>
#include <MaterialXShaderGen/NodeImplementations/Constant.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Value.h>

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
    , _classification(0)
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
        // Remove old graph if it already exists
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

        // Copy the nodedef string from the source graph
        _nodeGraph->setNodeDef(output->getParent()->asA<NodeGraph>()->getNodeDef());

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

        // Copy the nodedef string from the source node
        _nodeGraph->setNodeDef(nodeDef->getName());

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

        // Copy the nodedef string from the source shader
        _nodeGraph->setNodeDef(nodeDef->getName());

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
    // During this traversal we optimize the graph, pruning conditional branches 
    // what will never be taken, and replacing constant nodes with values.
    // We also add in any needed default geometric nodes.
    // 

    // Keep track of processed nodes to avoid duplication
    // of nodes with multiple downstream connections.
    std::set<NodePtr> processedNodes;

    for (Edge edge : root->traverseGraph(material))
    {
        NodePtr upstreamNode = optimize(edge);
        if (!upstreamNode)
        {
            continue;
        }

        ElementPtr downstreamElement = edge.getDownstreamElement();

        NodePtr newNode;
        if (processedNodes.count(upstreamNode))
        {
            // Check if we came to this node from an output. In that case it's the same node
            // we jumped to in the previous iteration, so skip this directly.
            //
            // TODO: This is a hack to avoid a bug triggered when the same processed output
            // is passed again. We need a more robust handling of this!
            //
            if (downstreamElement->isA<Output>())
            {
                continue;
            }

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

        // Connect the node to downstream node in the new graph.
        NodePtr downstream = _nodeGraph->getNode(getLongName(downstreamElement));
        ElementPtr connectingElement = edge.getConnectingElement();
        if (downstream && connectingElement)
        {
            downstream->setConnectedNode(connectingElement->getName(), newNode);
        }
    }

    // Create a flat version of the graph.
    _nodeGraph->flattenSubgraphs(target);

    // Create a topological ordering of the nodes
    vector<ElementPtr> topologicalOrder = _nodeGraph->topologicalSort();

    // Create an SgNode for each node, holding cached data needed for shader generation
    for (ElementPtr elem : topologicalOrder)
    {
        if (elem->isA<Node>())
        {
            NodePtr node = elem->asA<Node>();
            _nodeToSgNodeIndex[node] = _nodes.size();
            _nodes.push_back(SgNode(node, language, target));
        }
    }

    // Set shader classification according to the last node
    const size_t numNode = _nodes.size();
    _classification = numNode > 0 ? _nodes[numNode - 1]._classification : 0;

    // Set the vdirection to use for texture nodes
    // Default is to use direction UP
    const string& vdir = element->getRoot()->getAttribute("vdirection");
    _vdirection = vdir == "down" ? VDirection::DOWN : VDirection::UP;;

    // Create shader uniforms from the graph interface being used
    NodeDefPtr graphNodeDef = doc->getNodeDef(_nodeGraph->getNodeDef());
    if (graphNodeDef)
    {
        for (ParameterPtr param : graphNodeDef->getParameters())
        {
            if (_usedInterface.count(param) > 0)
            {
                addUniform(param->getName(), param);
            }
        }
        for (InputPtr input : graphNodeDef->getInputs())
        {
            if (_usedInterface.count(input) > 0)
            {
                addVarying(input->getName(), input);
            }
        }
    }

    // Create shader uniforms from all public named ports
    for (const SgNode& n : _nodes)
    {
        const Node& node = n.getNode();

        for (ParameterPtr param : node.getParameters())
        {
            const string& publicname = param->getPublicName();
            if (!publicname.empty())
            {
                addUniform(publicname, param);
            }
        }
        for (InputPtr input : node.getInputs())
        {
            // Don't publish connected inputs
            if (input->getNodeName().empty())
            {
                const string& publicname = input->getPublicName();
                if (!publicname.empty())
                {
                    addVarying(publicname, input);
                }
            }
        }
    }

    //
    // Calculate scopes for all nodes, considering branching from conditional nodes
    //

    size_t lastNodeIndex = _nodes.size() - 1;
    SgNode& lastNode = _nodes[lastNodeIndex];
    lastNode.getScopeInfo().type = SgNode::ScopeInfo::Type::GLOBAL;

    StringSet nodeUsed;
    nodeUsed.insert(lastNode.getName());

    // Iterate nodes in reversed toplogical order such that every node is visited AFTER 
    // each of the nodes that depend on it have been processed first.
    for (int nodeIndex = int(lastNodeIndex); nodeIndex >= 0; --nodeIndex)
    {
        SgNode& node = _nodes[nodeIndex];

        // Once we visit a node the scopeInfo has been determined and it will not be changed
        // By then we have visited all the nodes that depend on it already
        if (nodeUsed.count(node.getName()) == 0)
        {
            continue;
        }

        const bool isIfElse = node.getNode().getCategory() == "compare";
        const bool isSwitch = node.getNode().getCategory() == "switch";

        const SgNode::ScopeInfo& currentScopeInfo = node.getScopeInfo();

        const vector<InputPtr> inputs = node.getNode().getInputs();

        for (size_t inputIndex = 0; inputIndex < inputs.size(); ++inputIndex)
        {
            const InputPtr input = inputs[inputIndex];
            const NodePtr upstreamNode = input->getConnectedNode();
            if (upstreamNode)
            {
                // Create scope info for this network brach
                // If it's a conditonal branch the scope is adjusted
                SgNode::ScopeInfo newScopeInfo = currentScopeInfo;
                if (isIfElse && (inputIndex == 1 || inputIndex == 2))
                {
                    newScopeInfo.adjustAtConditionalInput(node.getNodePtr(), int(inputIndex), 0x6);
                }
                else if (isSwitch)
                {
                    const uint32_t fullMask = (1 << inputs.size()) - 1;
                    newScopeInfo.adjustAtConditionalInput(node.getNodePtr(), int(inputIndex), fullMask);
                }

                // Add the info to the upstream node
                SgNode::ScopeInfo& upstreamScopeInfo = getNode(upstreamNode).getScopeInfo();
                upstreamScopeInfo.merge(newScopeInfo);

                nodeUsed.insert(upstreamNode->getName());
            }
        }
    }

    // Track closure nodes used by each surface shader.
    for (SgNode& node : _nodes)
    {
        if (node.hasClassification(SgNode::Classification::SHADER))
        {
            for (Edge edge : node.getNodePtr()->traverseGraph())
            {
                NodePtr upstreamNode = edge.getUpstreamElement()->asA<Node>();
                if (upstreamNode)
                {
                    const SgNode& upstreamSgNode = getNode(upstreamNode);
                    if (upstreamSgNode.hasClassification(SgNode::Classification::CLOSURE))
                    {
                        node._usedClosures.insert(&upstreamSgNode);
                    }
                }
            }
        }
    }
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

void Shader::addComment(const string& str)
{
    beginLine();
    stage().code += "// " + str;
    endLine(false);
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

SgNode& Shader::getNode(const NodePtr& nodePtr)
{
    auto it = _nodeToSgNodeIndex.find(nodePtr);
    if (it == _nodeToSgNodeIndex.end())
    {
        throw ExceptionShaderGenError("No SgNode found for node '" + nodePtr->getName() + "'");
    }
    return _nodes[it->second];
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

void Shader::addUniform(const string& name, ParameterPtr param)
{
    auto it = _uniforms.find(name);
    if (it != _uniforms.end())
    {
        // The uniform name already exists.
        // Make sure it's the same data types.
        if (it->second->getType() != param->getType())
        {
            throw ExceptionShaderGenError("A shader uniform named '" + name + "' already exists but with different type.");
        }
    }
    else
    {
        _uniforms[name] = param;
    }
}

void Shader::addVarying(const string& name, InputPtr input)
{
    auto it = _varyings.find(name);
    if (it != _varyings.end())
    {
        // The uniform name already exists.
        // Make sure it's the same data types.
        if (it->second->getType() != input->getType())
        {
            throw ExceptionShaderGenError("A shader varying named '" + name + "' already exists but with different type.");
        }
    }
    else
    {
        _varyings[name] = input;
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

NodePtr Shader::optimize(const Edge& edge)
{
    // Find the downstream element in the new graph.
    ElementPtr downstreamElement = edge.getDownstreamElement();
    ElementPtr downstreamElementNew = _nodeGraph->getChild(getLongName(downstreamElement));
    if (!downstreamElementNew)
    {
        // Downstream element has been pruned
        // so ignore this edge.
        return nullptr;
    }

    // Find the upstream element. If it's an output move on
    // to the actual node connected to the output.
    ElementPtr upstreamElement = edge.getUpstreamElement();
    if (upstreamElement->isA<Output>())
    {
        upstreamElement = upstreamElement->asA<Output>()->getConnectedNode();
        if (!upstreamElement)
        {
            return nullptr;
        }
    }

    // Check if this is a connection to the graph interface
    if (upstreamElement->getParent()->isA<NodeDef>())
    {
        ValueElementPtr interfacePort = upstreamElement->asA<ValueElement>();
        _usedInterface.insert(interfacePort);
        return nullptr;
    }

    NodePtr node = upstreamElement->asA<Node>();
    if (!node)
    {
        return nullptr;
    }

    ValuePtr value = nullptr;
    string publicname = "";

    if (node->getCategory() == Compare::kNode)
    {
        // Check if we have a constant conditional expression
        const InputPtr intest = node->getInput("intest");
        NodePtr intestNode = intest->getConnectedNode();
        if (!intestNode || intestNode->getCategory() == Constant::kNode)
        {
            const ParameterPtr cutoff = node->getParameter("cutoff");

            const float intestValue = intestNode ?
                intestNode->getParameter("value")->getValue()->asA<float>() :
                intest->getValue()->asA<float>();

            const int branch = (intestValue <= cutoff->getValue()->asA<float>() ? 1 : 2);
            const InputPtr input = node->getInput(Compare::kInputNames[branch]);

            node = input->getConnectedNode();
            if (!node)
            {
                value = input->getValue();
            }
        }
    }
    else if (node->getCategory() == Switch::kNode)
    {
        // For switch the conditional is always constant
        const ParameterPtr which = node->getParameter("which");
        const float whichValue = which->getValue()->asA<float>();

        const int branch = int(whichValue);
        const InputPtr input = node->getInput(Switch::kInputNames[branch]);

        node = input->getConnectedNode();
        if (!node)
        {
            value = input->getValue();
        }
    }

    if (node && node->getCategory() == Constant::kNode)
    {
        const ParameterPtr param = node->getParameter("value");
        value = param->getValue();
        publicname = param->getPublicName();
    }

    if (value)
    {
        NodePtr downstreamNode = downstreamElementNew->asA<Node>();
        ElementPtr connectingElement = edge.getConnectingElement();
        if (downstreamNode && connectingElement)
        {
            InputPtr input = downstreamNode->getInput(connectingElement->getName());
            if (input)
            {
                input->setConnectedNode(nullptr);
                input->setValueString(value->getValueString());
                input->setPublicName(publicname);
            }
        }

        return nullptr;
    }

    return node;
}

}
