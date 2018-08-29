#include <MaterialXShaderGen/SgNode.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/SgImplementation.h>
#include <MaterialXShaderGen/Util.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>

#include <MaterialXFormat/File.h>

#include <iostream>
#include <sstream>
#include <stack>

namespace MaterialX
{

void SgInput::makeConnection(SgOutput* src)
{
    this->connection = src;
    src->connections.insert(this);
}

void SgInput::breakConnection()
{
    if (this->connection)
    {
        this->connection->connections.erase(this);
        this->connection = nullptr;
    }
}

void SgOutput::makeConnection(SgInput* dst)
{
    dst->connection = this;
    this->connections.insert(dst);
}

void SgOutput::breakConnection(SgInput* dst)
{
    this->connections.erase(dst);
    dst->connection = nullptr;
}

void SgOutput::breakConnection()
{
    for (SgInput* input : this->connections)
    {
        input->connection = nullptr;
    }
    this->connections.clear();
}

SgEdgeIterator SgOutput::traverseUpstream()
{
    return SgEdgeIterator(this);
}

namespace
{
    SgNodePtr createEmptyNode()
    {
        SgNodePtr node = std::make_shared<SgNode>("");
        node->addContextID(ShaderGenerator::NODE_CONTEXT_DEFAULT);
        return node;
    }
}

const SgNodePtr SgNode::NONE = createEmptyNode();

const string SgNode::SXCLASS_ATTRIBUTE = "sxclass";
const string SgNode::CONSTANT = "constant";
const string SgNode::IMAGE = "image";
const string SgNode::COMPARE = "compare";
const string SgNode::SWITCH = "switch";

bool SgNode::referencedConditionally() const
{
    if (_scopeInfo.type == SgNode::ScopeInfo::Type::SINGLE)
    {
        int numBranches = 0;
        uint32_t mask = _scopeInfo.conditionBitmask;
        for (; mask != 0; mask >>= 1)
        {
            if (mask & 1)
            {
                numBranches++;
            }
        }
        return numBranches > 0;
    }
    return false;
}

void SgNode::ScopeInfo::adjustAtConditionalInput(SgNode* condNode, int branch, const uint32_t fullMask)
{
    if (type == ScopeInfo::Type::GLOBAL || (type == ScopeInfo::Type::SINGLE && conditionBitmask == fullConditionMask))
    {
        type = ScopeInfo::Type::SINGLE;
        conditionalNode = condNode;
        conditionBitmask = 1 << branch;
        fullConditionMask = fullMask;
    }
    else if (type == ScopeInfo::Type::SINGLE)
    {
        type = ScopeInfo::Type::MULTIPLE;
        conditionalNode = nullptr;
    }
}

void SgNode::ScopeInfo::merge(const ScopeInfo &fromScope)
{
    if (type == ScopeInfo::Type::UNKNOWN || fromScope.type == ScopeInfo::Type::GLOBAL)
    {
        *this = fromScope;
    }
    else if (type == ScopeInfo::Type::GLOBAL)
    {

    }
    else if (type == ScopeInfo::Type::SINGLE && fromScope.type == ScopeInfo::Type::SINGLE && conditionalNode == fromScope.conditionalNode)
    {
        conditionBitmask |= fromScope.conditionBitmask;

        // This node is needed for all branches so it is no longer conditional
        if (conditionBitmask == fullConditionMask)
        {
            type = ScopeInfo::Type::GLOBAL;
            conditionalNode = nullptr;
        }
    }
    else
    {
        // NOTE: Right now multiple scopes is not really used, it works exactly as ScopeInfo::Type::GLOBAL
        type = ScopeInfo::Type::MULTIPLE;
        conditionalNode = nullptr;
    }
}

SgNode::SgNode(const string& name)
    : _name(name)
    , _classification(0)
    , _impl(nullptr)
{
}

SgNodePtr SgNode::create(const string& name, const NodeDef& nodeDef, ShaderGenerator& shadergen, const Node* nodeInstance)
{
    SgNodePtr newNode = std::make_shared<SgNode>(name);

    // Find the implementation for this nodedef
    InterfaceElementPtr impl = nodeDef.getImplementation(shadergen.getTarget(), shadergen.getLanguage());
    if (impl)
    {
        newNode->_impl = shadergen.getImplementation(impl);
    }
    if (!newNode->_impl)
    {
        throw ExceptionShaderGenError("Could not find a matching implementation for node '" + nodeDef.getNodeString() +
            "' matching language '" + shadergen.getLanguage() + "' and target '" + shadergen.getTarget() + "'");
    }

    // Create interface from nodedef
    const vector<ValueElementPtr> nodeDefInputs = nodeDef.getChildrenOfType<ValueElement>();
    for (const ValueElementPtr& elem : nodeDefInputs)
    {
        if (elem->isA<Output>())
        {
            newNode->addOutput(elem->getName(), elem->getType());
        }
        else
        {
            SgInput* input = newNode->addInput(elem->getName(), elem->getType());
            if (!elem->getValueString().empty())
            {
                input->value = elem->getValue();
            }
        }
    }

    // Add a default output if needed
    if (newNode->numOutputs() == 0)
    {
        newNode->addOutput("out", nodeDef.getType());
    }

    // Assign input values from the node instance
    if (nodeInstance)
    {
        const vector<ValueElementPtr> nodeInstanceInputs = nodeInstance->getChildrenOfType<ValueElement>();
        for (const ValueElementPtr& elem : nodeInstanceInputs)
        {
            if (!elem->getValueString().empty())
            {
                SgInput* input = newNode->getInput(elem->getName());
                if (input)
                {
                    input->value = elem->getValue();
                }
            }
        }
    }

    // Set node classification
    newNode->_classification = Classification::TEXTURE;
    if (nodeDef.getType() == DataType::SURFACE)
    {
        newNode->_classification = Classification::SURFACE | Classification::SHADER;
    }
    else if (nodeDef.getType() == DataType::LIGHT)
    {
        newNode->_classification = Classification::LIGHT | Classification::SHADER;
    }
    else if (nodeDef.getType() == DataType::BSDF)
    {
        newNode->_classification = Classification::BSDF | Classification::CLOSURE;
    }
    else if (nodeDef.getType() == DataType::EDF)
    {
        newNode->_classification = Classification::EDF | Classification::CLOSURE;
    }
    else if (nodeDef.getType() == DataType::VDF)
    {
        newNode->_classification = Classification::VDF | Classification::CLOSURE;
    }
    else if (nodeDef.getNodeString() == CONSTANT)
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONSTANT;
    }
    else if (nodeDef.getNodeString() == IMAGE || nodeDef.getAttribute(SXCLASS_ATTRIBUTE) == IMAGE)
    {
        newNode->_classification = Classification::TEXTURE | Classification::FILETEXTURE;
    }
    else if (nodeDef.getNodeString() == COMPARE)
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONDITIONAL | Classification::IFELSE;
    }
    else if (nodeDef.getNodeString() == SWITCH)
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONDITIONAL | Classification::SWITCH;
    }

    // Let the generator assign in which contexts to use this node
    shadergen.addNodeContextIDs(newNode.get());

    return newNode;
}

SgInput* SgNode::getInput(const string& name)
{
    auto it = _inputMap.find(name);
    return it != _inputMap.end() ? it->second.get() : nullptr;
}

SgOutput* SgNode::getOutput(const string& name)
{
    auto it = _outputMap.find(name);
    return it != _outputMap.end() ? it->second.get() : nullptr;
}

const SgInput* SgNode::getInput(const string& name) const
{
    auto it = _inputMap.find(name);
    return it != _inputMap.end() ? it->second.get() : nullptr;
}

const SgOutput* SgNode::getOutput(const string& name) const
{
    auto it = _outputMap.find(name);
    return it != _outputMap.end() ? it->second.get() : nullptr;
}

SgInput* SgNode::addInput(const string& name, const string& type)
{
    if (getInput(name))
    {
        throw ExceptionShaderGenError("An input named '" + name + "' already exists on node '" + _name + "'");
    }

    SgInputPtr input = std::make_shared<SgInput>();
    input->name = name;
    input->type = type;
    input->node = this;
    input->value = nullptr;
    input->connection = nullptr;
    _inputMap[name] = input;
    _inputOrder.push_back(input.get());

    return input.get();
}

SgOutput* SgNode::addOutput(const string& name, const string& type)
{
    if (getOutput(name))
    {
        throw ExceptionShaderGenError("An output named '" + name + "' already exists on node '" + _name + "'");
    }

    SgOutputPtr output = std::make_shared<SgOutput>();
    output->name = name;
    output->type = type;
    output->node = this;
    _outputMap[name] = output;
    _outputOrder.push_back(output.get());

    return output.get();
}

void SgNode::renameInput(const string& name, const string& newName)
{
    if (name != newName)
    {
        auto it = _inputMap.find(name);
        if (it != _inputMap.end())
        {
            it->second->name = newName;
            _inputMap[newName] = it->second;
            _inputMap.erase(it);
        }
    }
}

void SgNode::renameOutput(const string& name, const string& newName)
{
    if (name != newName)
    {
        auto it = _outputMap.find(name);
        if (it != _outputMap.end())
        {
            it->second->name = newName;
            _outputMap[newName] = it->second;
            _outputMap.erase(it);
        }
    }
}


SgNodeGraph::SgNodeGraph(const string& name, DocumentPtr document)
    : SgNode(name)
    , _document(document)
{
}

void SgNodeGraph::addInputSockets(const InterfaceElement& elem)
{
    for (ValueElementPtr port : elem.getChildrenOfType<ValueElement>())
    {
        if (!port->isA<Output>())
        {
            SgInputSocket* inputSocket = addInputSocket(port->getName(), port->getType());
            if (!port->getValueString().empty())
            {
                inputSocket->value = port->getValue();
            }
        }
    }
}

void SgNodeGraph::addOutputSockets(const InterfaceElement& elem)
{
    for (const OutputPtr& output : elem.getOutputs())
    {
        addOutputSocket(output->getName(), output->getType());
    }
    if (numOutputSockets() == 0)
    {
        addOutputSocket("out", elem.getType());
    }
}

void SgNodeGraph::addUpstreamDependencies(const Element& root, ConstMaterialPtr material, ShaderGenerator& shadergen)
{
    // Keep track of our root node in the graph.
    // This is needed when the graph is a shader graph and we need 
    // to make connections for BindInputs during traversal below.
    SgNode* rootNode = getNode(root.getName());

    std::set<ElementPtr> processedOutputs;
    for (Edge edge : root.traverseGraph(material))
    {
        ElementPtr upstreamElement = edge.getUpstreamElement();
        ElementPtr downstreamElement = edge.getDownstreamElement();

        // Early out if downstream element is an output that 
        // we have already processed. This might happen since
        // we perform jumps over output elements below.
        if (processedOutputs.count(downstreamElement))
        {
            continue;
        }

        // If upstream is an output jump to the actual node connected to the output.
        if (upstreamElement->isA<Output>())
        {
            // Record this output so we don't process it again when it
            // shows up as a downstream element in the next iteration.
            processedOutputs.insert(upstreamElement);

            upstreamElement = upstreamElement->asA<Output>()->getConnectedNode();
            if (!upstreamElement)
            {
                continue;
            }
        }

        // Create the node if it doesn't exists
        NodePtr upstreamNode = upstreamElement->asA<Node>();
        const string& newNodeName = upstreamNode->getName();
        SgNode* newNode = getNode(newNodeName);
        if (!newNode)
        {
            newNode = addNode(*upstreamNode, shadergen);
        }

        //
        // Make connections
        //

        // First check if this was a bind input connection
        // In this case we must have a root node as well
        ElementPtr connectingElement = edge.getConnectingElement();
        if (rootNode && connectingElement && connectingElement->isA<BindInput>())
        {
            // Connect to the corresponding input on the root node
            SgInput* input = rootNode->getInput(connectingElement->getName());
            if (input)
            {
                input->breakConnection();
                input->makeConnection(newNode->getOutput());
            }
        }
        else
        {
            // Check if it was a node downstream
            NodePtr downstreamNode = downstreamElement->asA<Node>();
            if (downstreamNode)
            {
                // We have a node downstream
                SgNode* downstream = getNode(downstreamNode->getName());
                if (downstream && connectingElement)
                {
                    SgInput* input = downstream->getInput(connectingElement->getName());
                    if (!input)
                    {
                        throw ExceptionShaderGenError("Could not find an input named '" + connectingElement->getName() +
                            "' on downstream node '" + downstream->getName() + "'");
                    }
                    input->makeConnection(newNode->getOutput());
                }
            }
            else
            {
                // Not a node, then it must be an output
                SgOutputSocket* outputSocket = getOutputSocket(downstreamElement->getName());
                if (outputSocket)
                {
                    outputSocket->makeConnection(newNode->getOutput());
                }
            }
        }
    }
}

void SgNodeGraph::addDefaultGeomNode(SgInput* input, const string& geomNode, ShaderGenerator& shadergen)
{
    const string geomNodeName = "default_" + geomNode;
    SgNode* node = getNode(geomNodeName);

    if (!node)
    {
        string geomNodeDefName = "ND_" + geomNode + "_" + input->type;
        NodeDefPtr geomNodeDef = _document->getNodeDef(geomNodeDefName);
        if (!geomNodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef named '" + geomNodeDefName +
                "' for defaultgeomprop on input '" + input->node->getName() + "." + input->name + "'");
        }

        SgNodePtr geomNodePtr = SgNode::create(geomNodeName, *geomNodeDef, shadergen);
        _nodeMap[geomNodeName] = geomNodePtr;
        _nodeOrder.push_back(geomNodePtr.get());

        node = geomNodePtr.get();
    }

    input->makeConnection(node->getOutput());
}

void SgNodeGraph::addColorTransformNode(SgOutput* output, const string& colorTransform, ShaderGenerator& shadergen)
{
    const string nodeDefName = "ND_" + colorTransform + "_" + output->type;
    NodeDefPtr nodeDef = _document->getNodeDef(nodeDefName);
    if (!nodeDef)
    {
        // Color transformations are by design not defined for all data types, only for color types.
        // So if a nodedef for the given output type is not found we just ignore this transform.
        return;
    }

    const string nodeName = output->node->getName() + "_" + colorTransform;
    SgNodePtr nodePtr = SgNode::create(nodeName, *nodeDef, shadergen);
    _nodeMap[nodeName] = nodePtr;
    _nodeOrder.push_back(nodePtr.get());

    SgNode* node = nodePtr.get();
    SgOutput* nodeOutput = node->getOutput(0);

    // Connect the node to the downstream inputs
    // Iterate a copy of the connection set since the original
    // set will change when breaking the old connections
    SgInputSet downstreamConnections = output->connections;
    for (SgInput* downstreamInput : downstreamConnections)
    {
        downstreamInput->breakConnection();
        downstreamInput->makeConnection(nodeOutput);
    }

    // Connect the node to the upstream output
    SgInput* nodeInput = node->getInput(0);
    nodeInput->makeConnection(output);
}

SgNodeGraphPtr SgNodeGraph::create(NodeGraphPtr nodeGraph, ShaderGenerator& shadergen)
{
    NodeDefPtr nodeDef = nodeGraph->getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Can't find nodedef '" + nodeGraph->getNodeDefString() + "' referenced by nodegraph '" + nodeGraph->getName() + "'");
    }

    SgNodeGraphPtr graph = std::make_shared<SgNodeGraph>(nodeGraph->getName(), nodeGraph->getDocument());

    // Create input sockets from the nodedef
    graph->addInputSockets(*nodeDef);

    // Create output sockets from the nodegraph
    graph->addOutputSockets(*nodeGraph);

    // Traverse all outputs and create all upstream dependencies
    for (OutputPtr graphOutput : nodeGraph->getOutputs())
    {
        graph->addUpstreamDependencies(*graphOutput, nullptr, shadergen);
    }

    // Set classification according to last node
    // TODO: What if the graph has multiple outputs?
    {
        SgOutputSocket* outputSocket = graph->getOutputSocket();
        graph->_classification = outputSocket->connection ? outputSocket->connection->node->_classification : 0;
    }

    graph->finalize(shadergen);

    return graph;
}

SgNodeGraphPtr SgNodeGraph::create(const string& name, ElementPtr element, ShaderGenerator& shadergen)
{
    SgNodeGraphPtr graph;
    ElementPtr root;
    MaterialPtr material;

    if (element->isA<Output>())
    {
        OutputPtr output = element->asA<Output>();
        ElementPtr parent = output->getParent();
        InterfaceElementPtr interface = parent->asA<InterfaceElement>();

        if (parent->isA<NodeGraph>())
        {
            NodeDefPtr nodeDef = parent->asA<NodeGraph>()->getNodeDef();
            if (nodeDef)
            {
                interface = nodeDef;
            }
        }

        if (!interface)
        {
            parent = output->getConnectedNode();
            interface = parent ? parent->asA<InterfaceElement>() : nullptr;
            if (!interface)
            {
                throw ExceptionShaderGenError("Given output '" + output->getName() + "' has no interface valid for shader generation");
            }
        }

        graph = std::make_shared<SgNodeGraph>(name, element->getDocument());

        // Create input sockets
        graph->addInputSockets(*interface);

        // Create the given output socket
        graph->addOutputSocket(output->getName(), output->getType());

        // Start traversal from this output
        root = output;
    }
    else if (element->isA<ShaderRef>())
    {
        ShaderRefPtr shaderRef = element->asA<ShaderRef>();

        NodeDefPtr nodeDef = shaderRef->getNodeDef();
        if (!nodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef for shader '" + shaderRef->getName() + "'");
        }

        graph = std::make_shared<SgNodeGraph>(name, element->getDocument());

        // Create input sockets
        graph->addInputSockets(*nodeDef);

        // Create output sockets
        graph->addOutputSockets(*nodeDef);

        // Create this shader node in the graph.
        const string& newNodeName = shaderRef->getName();
        SgNodePtr newNode = SgNode::create(newNodeName, *nodeDef, shadergen, nullptr);
        graph->_nodeMap[newNodeName] = newNode;
        graph->_nodeOrder.push_back(newNode.get());

        // Connect it to the graph output
        SgOutputSocket* outputSocket = graph->getOutputSocket();
        outputSocket->makeConnection(newNode->getOutput());

        // Connect node parameters to the graph inputs
        for (ParameterPtr elem : nodeDef->getParameters())
        {
            SgInputSocket* inputSocket = graph->getInputSocket(elem->getName());
            SgInput* input = newNode->getInput(elem->getName());
            if (!inputSocket || !input)
            {
                throw ExceptionShaderGenError("Shader parameter '" + elem->getName() + "' doesn't match an existing input on graph '" + graph->getName() + "'");
            }

            // Copy value from binding
            BindParamPtr bindParam = shaderRef->getBindParam(elem->getName());
            if (bindParam)
            {
                if (!bindParam->getValueString().empty())
                {
                    inputSocket->value = bindParam->getValue();
                }
            }

            // Connect to the graph input
            inputSocket->makeConnection(input);
        }

        // Connect unconnected node inputs to the graph inputs
        for (InputPtr elem : nodeDef->getInputs())
        {
            SgInputSocket* inputSocket = graph->getInputSocket(elem->getName());
            SgInput* input = newNode->getInput(elem->getName());
            if (!inputSocket || !input)
            {
                throw ExceptionShaderGenError("Shader input '" + elem->getName() + "' doesn't match an existing input on graph '" + graph->getName() + "'");
            }

            BindInputPtr bindInput = shaderRef->getBindInput(elem->getName());

            // Copy value from binding
            if (bindInput)
            {
                if (!bindInput->getValueString().empty())
                {
                    inputSocket->value = bindInput->getValue();
                }
            }

            // If we have no connection, connect to the graph input
            const string& explicitConnection = bindInput ? bindInput->getOutputString() : EMPTY_STRING;
            const string& implicitConnection = elem->getAttribute("defaultgeomprop");
            if (explicitConnection.empty() && implicitConnection.empty())
            {
                inputSocket->makeConnection(input);
            }
        }

        // Handle the "defaultgeomprop" directives on the nodedef inputs.
        // Create and connect default geometric nodes on unconnected inputs.
        for (const InputPtr& nodeDefInput : nodeDef->getInputs())
        {
            BindInputPtr bindInput = shaderRef->getBindInput(nodeDefInput->getName());

            const string& connection = bindInput ? bindInput->getOutputString() : EMPTY_STRING;
            const string& defaultGeomNode = connection.empty() ? nodeDefInput->getAttribute("defaultgeomprop") : EMPTY_STRING;

            if (!defaultGeomNode.empty())
            {
                SgInput* input = newNode->getInput(nodeDefInput->getName());
                graph->addDefaultGeomNode(input, defaultGeomNode, shadergen);
            }
        }

        // Start traversal from this shaderref and material
        root = shaderRef;
        material = shaderRef->getParent()->asA<Material>();
    }

    if (!root)
    {
        throw ExceptionShaderGenError("Shader generation from element '" + element->getName() + "' of type '" + element->getCategory() + "' is not supported");
    }

    // Traverse and create all dependencies upstream
    graph->addUpstreamDependencies(*root, material, shadergen);

    // Set classification according to root node
    SgOutputSocket* outputSocket = graph->getOutputSocket();
    graph->_classification = outputSocket->connection ? outputSocket->connection->node->_classification : 0;

    graph->finalize(shadergen);

    return graph;
}

SgNode* SgNodeGraph::addNode(const Node& node, ShaderGenerator& shadergen)
{
    NodeDefPtr nodeDef = node.getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Could not find a nodedef for node '" + node.getName() + "'");
    }
    
    // Create this node in the graph.
    const string& name = node.getName();
    SgNodePtr newNode = SgNode::create(name, *nodeDef, shadergen, &node);
    _nodeMap[name] = newNode;
    _nodeOrder.push_back(newNode.get());

    // Check if any of the node inputs should be connected to the graph interface
    for (ValueElementPtr elem : node.getChildrenOfType<ValueElement>())
    {
        const string& interfaceName = elem->getInterfaceName();
        if (!interfaceName.empty())
        {
            SgInputSocket* inputSocket = getInputSocket(interfaceName);
            if (!inputSocket)
            {
                throw ExceptionShaderGenError("Interface name '" + interfaceName + "' doesn't match an existing input on nodegraph '" + getName() + "'");
            }
            SgInput* input = newNode->getInput(elem->getName());
            if (input)
            {
                input->makeConnection(inputSocket);
            }
        }
    }

    // Handle the "defaultgeomprop" directives on the nodedef inputs.
    // Create and connect default geometric nodes on unconnected inputs.
    for (const InputPtr& nodeDefInput : nodeDef->getInputs())
    {
        SgInput* input = newNode->getInput(nodeDefInput->getName());
        InputPtr nodeInput = node.getInput(nodeDefInput->getName());

        const string& connectedNode = nodeInput ? nodeInput->getNodeName() : EMPTY_STRING;
        const string& defaultGeomNode = connectedNode.empty() && !input->connection ? nodeDefInput->getAttribute("defaultgeomprop") : EMPTY_STRING;

        if (!defaultGeomNode.empty())
        {
            addDefaultGeomNode(input, defaultGeomNode, shadergen);
        }
    }

    // Check if this is a file texture node that requires color transformation.
    if (newNode->hasClassification(SgNode::Classification::FILETEXTURE))
    {
        ParameterPtr file = node.getParameter("file");
        const string& colorSpace = file ? file->getAttribute("colorspace") : EMPTY_STRING;

        // TODO: Handle more color transforms
        if (colorSpace == "sRGB")
        {
            // Store the node and it's color transform so we can create this
            // color transformation later when finalizing the graph.
            _colorTransformMap[newNode.get()] = "srgb_linear";
        }
    }

    return newNode.get();
}

SgInputSocket* SgNodeGraph::addInputSocket(const string& name, const string& type)
{
    return SgNode::addOutput(name, type);
}

SgOutputSocket* SgNodeGraph::addOutputSocket(const string& name, const string& type)
{
    return SgNode::addInput(name, type);
}

void SgNodeGraph::renameInputSocket(const string& name, const string& newName)
{
    return SgNode::renameOutput(name, newName);
}

void SgNodeGraph::renameOutputSocket(const string& name, const string& newName)
{
    return SgNode::renameInput(name, newName);
}

SgNode* SgNodeGraph::getNode(const string& name)
{
    auto it = _nodeMap.find(name);
    return it != _nodeMap.end() ? it->second.get() : nullptr;
}

void SgNodeGraph::finalize(ShaderGenerator& shadergen)
{
    // Optimize the graph, removing redundant paths.
    optimize();

    // Insert color transformation nodes where needed
    for (auto it : _colorTransformMap)
    {
        addColorTransformNode(it.first->getOutput(), it.second, shadergen);
    }
    _colorTransformMap.clear();

    // Sort the nodes in topological order.
    topologicalSort();

    // Calculate scopes for all nodes in the graph
    calculateScopes();

    // Make sure inputs and outputs on the graph have
    // valid and unique names to avoid name collisions
    // during shader generation
    validateNames(shadergen);

    // Track closure nodes used by each surface shader.
    for (SgNode* node : _nodeOrder)
    {
        if (node->hasClassification(SgNode::Classification::SHADER))
        {
            for (SgEdge edge : node->getOutput()->traverseUpstream())
            {
                if (edge.upstream)
                {
                    if (edge.upstream->node->hasClassification(SgNode::Classification::CLOSURE))
                    {
                        node->_usedClosures.insert(edge.upstream->node);
                    }
                }
            }
        }
    }
}

void SgNodeGraph::disconnect(SgNode* node)
{
    for (SgInput* input : node->getInputs())
    {
        input->breakConnection();
    }
    for (SgOutput* output : node->getOutputs())
    {
        output->breakConnection();
    }
}

void SgNodeGraph::optimize()
{
    size_t numEdits = 0;
    for (SgNode* node : getNodes())
    {
        if (node->hasClassification(SgNode::Classification::CONSTANT))
        {
            // Constant nodes can be removed by assigning their value downstream
            // But don't remove it if it's connected upstream, i.e. it's value 
            // input is published.
            SgInput* valueInput = node->getInput(0);
            if (!valueInput->connection)
            {
                bypass(node, 0);
                ++numEdits;
            }
        }
        else if (node->hasClassification(SgNode::Classification::IFELSE))
        {
            // Check if we have a constant conditional expression
            SgInput* intest = node->getInput("intest");
            if (!intest->connection || intest->connection->node->hasClassification(SgNode::Classification::CONSTANT))
            {
                // Find which branch should be taken
                SgInput* cutoff = node->getInput("cutoff");
                ValuePtr value = intest->connection ? intest->connection->node->getInput(0)->value : intest->value;
                const float intestValue = value ? value->asA<float>() : 0.0f;
                const int branch = (intestValue <= cutoff->value->asA<float>() ? 2 : 3);

                // Bypass the conditional using the taken branch
                bypass(node, branch);

                ++numEdits;
            }
        }
        else if (node->hasClassification(SgNode::Classification::SWITCH))
        {
            // Check if we have a constant conditional expression
            SgInput* which = node->getInput("which");
            if (!which->connection || which->connection->node->hasClassification(SgNode::Classification::CONSTANT))
            {
                // Find which branch should be taken
                ValuePtr value = which->connection ? which->connection->node->getInput(0)->value : which->value;
                const int branch = int(value==nullptr ? 0 :
                    (which->type == DataType::BOOLEAN ? value->asA<bool>() :
                    (which->type == DataType::FLOAT ? value->asA<float>() : value->asA<int>())));

                // Bypass the conditional using the taken branch
                bypass(node, branch);

                ++numEdits;
            }
        }
    }

    if (numEdits > 0)
    {
        std::set<SgNode*> usedNodes;

        // Travers the graph to find nodes still in use
        for (SgOutputSocket* outputSocket : getOutputSockets())
        {
            if (outputSocket->connection)
            {
                for (SgEdge edge : outputSocket->connection->traverseUpstream())
                {
                    usedNodes.insert(edge.upstream->node);
                }
            }
        }

        // Remove any unused nodes
        for (SgNode* node : _nodeOrder)
        {
            if (usedNodes.count(node) == 0)
            {
                // Break all connections
                disconnect(node);

                // Erase from temporary records
                _colorTransformMap.erase(node);

                // Erase from storage
                _nodeMap.erase(node->getName());
            }
        }
        _nodeOrder.resize(usedNodes.size());
        _nodeOrder.assign(usedNodes.begin(), usedNodes.end());
    }
}

void SgNodeGraph::bypass(SgNode* node, size_t inputIndex, size_t outputIndex)
{
    SgInput* input = node->getInput(inputIndex);
    SgOutput* output = node->getOutput(outputIndex);

    SgOutput* upstream = input->connection;
    if (upstream)
    {
        // Re-route the upstream output to the downstream inputs.
        // Iterate a copy of the connection set since the
        // original set will change when breaking connections.
        SgInputSet downstreamConnections = output->connections;
        for (SgInput* downstream : downstreamConnections)
        {
            output->breakConnection(downstream);
            downstream->makeConnection(upstream);
        }
    }
    else
    {
        // No node connected upstream to re-route,
        // so push the input's value downstream instead.
        // Iterate a copy of the connection set since the
        // original set will change when breaking connections.
        SgInputSet downstreamConnections = output->connections;
        for (SgInput* downstream : downstreamConnections)
        {
            output->breakConnection(downstream);
            downstream->value = input->value;
        }
    }
}

void SgNodeGraph::topologicalSort()
{
    // Calculate a topological order of the children, using Kahn's algorithm
    // to avoid recursion.
    //
    // Running time: O(numNodes + numEdges).

    // Calculate in-degrees for all nodes, and enqueue those with degree 0.
    std::unordered_map<SgNode*, int> inDegree(_nodeMap.size());
    std::deque<SgNode*> nodeQueue;
    for (auto it : _nodeMap)
    {
        SgNode* node = it.second.get();

        int connectionCount = 0;
        for (const SgInput* input : node->getInputs())
        {
            if (input->connection && input->connection->node != this)
            {
                ++connectionCount;
            }
        }

        inDegree[node] = connectionCount;

        if (connectionCount == 0)
        {
            nodeQueue.push_back(node);
        }
    }

    _nodeOrder.resize(_nodeMap.size(), nullptr);
    size_t count = 0;

    while (!nodeQueue.empty())
    {
        // Pop the queue and add to topological order.
        SgNode* node = nodeQueue.front();
        nodeQueue.pop_front();
        _nodeOrder[count++] = node;

        // Find connected nodes and decrease their in-degree, 
        // adding node to the queue if in-degrees becomes 0.
        for (auto output : node->getOutputs())
        {
            for (auto input : output->connections)
            {
                if (input->node != this)
                {
                    if (--inDegree[input->node] <= 0)
                    {
                        nodeQueue.push_back(input->node);
                    }
                }
            }
        }
    }

    // Check if there was a cycle.
    if (count != _nodeMap.size())
    {
        throw ExceptionFoundCycle("Encountered a cycle in graph: " + getName());
    }
}

void SgNodeGraph::calculateScopes()
{
    //
    // Calculate scopes for all nodes, considering branching from conditional nodes
    //
    // TODO: Refactor the scope handling, using scope id's instead
    //

    if (_nodeOrder.empty())
    {
        return;
    }

    size_t lastNodeIndex = _nodeOrder.size() - 1;
    SgNode* lastNode = _nodeOrder[lastNodeIndex];
    lastNode->getScopeInfo().type = SgNode::ScopeInfo::Type::GLOBAL;

    std::set<SgNode*> nodeUsed;
    nodeUsed.insert(lastNode);

    // Iterate nodes in reversed toplogical order such that every node is visited AFTER 
    // each of the nodes that depend on it have been processed first.
    for (int nodeIndex = int(lastNodeIndex); nodeIndex >= 0; --nodeIndex)
    {
        SgNode* node = _nodeOrder[nodeIndex];

        // Once we visit a node the scopeInfo has been determined and it will not be changed
        // By then we have visited all the nodes that depend on it already
        if (nodeUsed.count(node) == 0)
        {
            continue;
        }

        const bool isIfElse = node->hasClassification(SgNode::Classification::IFELSE);
        const bool isSwitch = node->hasClassification(SgNode::Classification::SWITCH);

        const SgNode::ScopeInfo& currentScopeInfo = node->getScopeInfo();

        for (size_t inputIndex = 0; inputIndex < node->numInputs(); ++inputIndex)
        {
            SgInput* input = node->getInput(inputIndex);

            if (input->connection)
            {
                SgNode* upstreamNode = input->connection->node;

                // Create scope info for this network brach
                // If it's a conditonal branch the scope is adjusted
                SgNode::ScopeInfo newScopeInfo = currentScopeInfo;
                if (isIfElse && (inputIndex == 2 || inputIndex == 3))
                {
                    newScopeInfo.adjustAtConditionalInput(node, int(inputIndex), 0x12);
                }
                else if (isSwitch)
                {
                    const uint32_t fullMask = (1 << node->numInputs()) - 1;
                    newScopeInfo.adjustAtConditionalInput(node, int(inputIndex), fullMask);
                }

                // Add the info to the upstream node
                SgNode::ScopeInfo& upstreamScopeInfo = upstreamNode->getScopeInfo();
                upstreamScopeInfo.merge(newScopeInfo);

                nodeUsed.insert(upstreamNode);
            }
        }
    }
}

void SgNodeGraph::validateNames(ShaderGenerator& shadergen)
{
    // Make sure inputs and outputs have names valid for the 
    // target shading language, and are unique to avoid name 
    // conflicts when emitting variable names for them.

    // Names in use for the graph is recorded in 'uniqueNames'.
    Syntax::UniqueNameMap uniqueNames;
    for (SgInputSocket* inputSocket : getInputSockets())
    {
        string name = inputSocket->name;
        shadergen.getSyntax()->makeUnique(name, uniqueNames);
        renameInputSocket(inputSocket->name, name);
    }
    for (SgOutputSocket* outputSocket : getOutputSockets())
    {
        string name = outputSocket->name;
        shadergen.getSyntax()->makeUnique(outputSocket->name, uniqueNames);
        renameOutputSocket(outputSocket->name, name);
    }
    for (SgNode* node : getNodes())
    {
        for (SgOutput* output : node->getOutputs())
        {
            // Node outputs use long names for better code readability
            string name = output->node->getName() + "_" + output->name;
            shadergen.getSyntax()->makeUnique(name, uniqueNames);
            node->renameOutput(output->name, name);
        }
    }
}


namespace
{
    static const SgEdgeIterator NULL_EDGE_ITERATOR(nullptr);
}

SgEdgeIterator::SgEdgeIterator(SgOutput* output)
    : _upstream(output)
    , _downstream(nullptr)
{
}

SgEdgeIterator& SgEdgeIterator::operator++()
{
    if (_upstream && _upstream->node->numInputs())
    {
        // Traverse to the first upstream edge of this element.
        _stack.push_back(StackFrame(_upstream, 0));

        SgInput* input = _upstream->node->getInput(0);
        SgOutput* output = input->connection;

        if (output && !output->node->isNodeGraph())
        {
            extendPathUpstream(output, input);
            return *this;
        }
    }

    while (true)
    {
        if (_upstream)
        {
            returnPathDownstream(_upstream);
        }

        if (_stack.empty())
        {
            // Traversal is complete.
            *this = SgEdgeIterator::end();
            return *this;
        }

        // Traverse to our siblings.
        StackFrame& parentFrame = _stack.back();
        while (parentFrame.second + 1 < parentFrame.first->node->numInputs())
        {
            SgInput* input = parentFrame.first->node->getInput(++parentFrame.second);
            SgOutput* output = input->connection;

            if (output && !output->node->isNodeGraph())
            {
                extendPathUpstream(output, input);
                return *this;
            }
        }

        // Traverse to our parent's siblings.
        returnPathDownstream(parentFrame.first);
        _stack.pop_back();
    }

    return *this;
}

const SgEdgeIterator& SgEdgeIterator::end()
{
    return NULL_EDGE_ITERATOR;
}

void SgEdgeIterator::extendPathUpstream(SgOutput* upstream, SgInput* downstream)
{
    // Check for cycles.
    if (_path.count(upstream))
    {
        throw ExceptionFoundCycle("Encountered cycle at element: " + upstream->node->getName() + "." + upstream->name);
    }

    // Extend the current path to the new element.
    _path.insert(upstream);
    _upstream = upstream;
    _downstream = downstream;
}

void SgEdgeIterator::returnPathDownstream(SgOutput* upstream)
{
    _path.erase(upstream);
    _upstream = nullptr;
    _downstream = nullptr;
}

} // namespace MaterialX
