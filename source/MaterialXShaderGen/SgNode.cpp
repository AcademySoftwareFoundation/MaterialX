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


const SgNode SgNode::NONE("");

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

SgNodePtr SgNode::creator(const string& name, const NodeDef& nodeDef, ShaderGenerator& shadergen, const Node* nodeInstance)
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

    // Add inputs from nodedef
    const vector<ValueElementPtr> nodeDefInputs = nodeDef.getChildrenOfType<ValueElement>();
    for (const ValueElementPtr& elem : nodeDefInputs)
    {
        SgInput* input = newNode->addInput(elem->getName(), elem->getType());
        if (!elem->getValueString().empty())
        {
            input->value = elem->getValue();
        }
    }

    // Assign input values and channel swizzling from the node instance
    if (nodeInstance)
    {
        const vector<ValueElementPtr> nodeInstanceInputs = nodeInstance->getChildrenOfType<ValueElement>();
        for (const ValueElementPtr& elem : nodeInstanceInputs)
        {
            SgInput* input = newNode->getInput(elem->getName());
            if (input)
            {
                if (!elem->getValueString().empty())
                {
                    input->value = elem->getValue();
                }
                InputPtr inputElem = elem->asA<Input>();
                if (inputElem)
                {
                    input->channels = inputElem->getChannels();
                }
            }
        }
    }

    // Add the node output
    // TODO: Support multiple outputs
    newNode->addOutput("out", nodeDef.getType());

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
    else if (nodeDef.getNodeString() == "constant")
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONSTANT;
    }
    else if (nodeDef.getNodeString() == "image")
    {
        newNode->_classification = Classification::TEXTURE | Classification::FILETEXTURE;
    }
    else if (nodeDef.getNodeString() == "compare")
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONDITIONAL | Classification::IFELSE;
    }
    else if (nodeDef.getNodeString() == "switch")
    {
        newNode->_classification = Classification::TEXTURE | Classification::CONDITIONAL | Classification::SWITCH;
    }

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


SgNodeGraph::SgNodeGraph(const string& name)
    : SgNode(name)
{
}

SgNodeGraphPtr SgNodeGraph::creator(NodeGraphPtr nodeGraph, ShaderGenerator& shadergen)
{
    SgNodeGraphPtr graph = std::make_shared<SgNodeGraph>(nodeGraph->getName());

    NodeDefPtr graphDef = nodeGraph->getNodeDef();
    if (!graphDef)
    {
        throw ExceptionShaderGenError("Can't find nodedef '" + nodeGraph->getNodeDefString() + "' referenced by nodegraph '" + nodeGraph->getName() + "'");
    }

    for (const ValueElementPtr& elem : graphDef->getChildrenOfType<ValueElement>())
    {
        SgInputSocket* inputSocket = graph->addInputSocket(elem->getName(), elem->getType());

        if (!elem->getValueString().empty())
        {
            inputSocket->value = elem->getValue();
        }
    }

    for (const OutputPtr& graphOutput : nodeGraph->getOutputs())
    {
        SgOutputSocket* outputSocket = graph->addOutputSocket(graphOutput->getName(), graphOutput->getType());
        outputSocket->channels = graphOutput->getChannels();
    }

    // Traverse all outputs and create all dependencies upstream
    for (OutputPtr graphOutput : nodeGraph->getOutputs())
    {
        for (Edge edge : graphOutput->traverseGraph())
        {
            ElementPtr upstreamElement = edge.getUpstreamElement();

            // If it's an output move on to the actual node connected to the output.
            if (upstreamElement->isA<Output>())
            {
                upstreamElement = upstreamElement->asA<Output>()->getConnectedNode();
                if (!upstreamElement)
                {
                    continue;
                }
            }

            NodePtr upstreamNode = upstreamElement->asA<Node>();
            const string& newNodeName = upstreamNode->getName();
            SgNode* newNode = graph->getNode(newNodeName);
            if (!newNode)
            {
                newNode = graph->addNode(*upstreamNode, shadergen);
            }

            // Make connections
            NodePtr downstreamNode = edge.getDownstreamElement()->asA<Node>();
            if (downstreamNode)
            {
                SgNode* downstream = graph->getNode(downstreamNode->getName());
                ElementPtr connectingElement = edge.getConnectingElement();
                if (downstream && connectingElement)
                {
                    SgInput* input = downstream->getInput(connectingElement->getName());
                    input->makeConnection(newNode->getOutput());
                }
            }
            else
            {
                // This is a graph interface output
                SgOutputSocket* outputSocket = graph->getOutputSocket(edge.getDownstreamElement()->getName());
                outputSocket->makeConnection(newNode->getOutput());
            }
        }
    }

    // Set classification according to last node
    // TODO: What if the graph has multiple outputs?
    graph->_classification = graph->getOutputSocket()->connection->node->_classification;

    graph->finalize();

    return graph;
}

SgNodeGraphPtr SgNodeGraph::creator(const string& name, ElementPtr element, ShaderGenerator& shadergen)
{
    SgNodeGraphPtr graph = std::make_shared<SgNodeGraph>(name);

    ElementPtr root;
    MaterialPtr material;

    if (element->isA<Output>())
    {
        OutputPtr output = element->asA<Output>();

        NodePtr srcNode = output->getParent()->isA<NodeGraph>() ? output->getConnectedNode() : output->getParent()->asA<Node>();
        if (!srcNode)
        {
            throw ExceptionShaderGenError("Given output element '" + output->getName() + "' has no node connection");
        }

        // Create this node in the graph and connect it to an output socket
        SgNode* newNode = graph->addNode(*srcNode, shadergen);
        SgOutputSocket* outputSocket = graph->addOutputSocket(output->getName(), output->getType());
        outputSocket->makeConnection(newNode->getOutput());
        outputSocket->channels = output->getChannels();

        // Start traveral from this node
        root = srcNode;
    }
    else if (element->isA<Node>())
    {
        NodePtr srcNode = element->asA<Node>();

        // Create this node in the graph and connect it to an output socket
        SgNode* newNode = graph->addNode(*srcNode, shadergen);
        SgOutputSocket* outputSocket = graph->addOutputSocket("out", newNode->getOutput()->type);
        outputSocket->makeConnection(newNode->getOutput());

        // Start traveral from this node
        root = srcNode;
    }
    else if (element->isA<ShaderRef>())
    {
        ShaderRefPtr shaderRef = element->asA<ShaderRef>();

        // Create this node in the graph and connect it to an output socket
        SgNode* newNode = graph->addNode(*shaderRef, shadergen);
        SgOutputSocket* outputSocket = graph->addOutputSocket("out", newNode->getOutput()->type);
        outputSocket->makeConnection(newNode->getOutput());

        // Start traveral from this shader ref and material
        root = shaderRef;
        material = shaderRef->getParent()->asA<Material>();
    }

    if (!root)
    {
        throw ExceptionShaderGenError("Shader generation from element '" + element->getName() + "' of type '" + element->getCategory() + "' is not supported");
    }

    // Traverse and create all dependencies upstream
    for (Edge edge : root->traverseGraph(material))
    {
        ElementPtr upstreamElement = edge.getUpstreamElement();

        // If it's an output move on to the actual node connected to the output.
        if (upstreamElement->isA<Output>())
        {
            upstreamElement = upstreamElement->asA<Output>()->getConnectedNode();
            if (!upstreamElement)
            {
                continue;
            }
        }

        // Create the node.
        NodePtr upstreamNode = upstreamElement->asA<Node>();
        const string& newNodeName = upstreamNode->getName();
        SgNode* newNode = graph->getNode(newNodeName);
        if (!newNode)
        {
            newNode = graph->addNode(*upstreamNode, shadergen);
        }

        // Make the connection.
        SgNode* downstream = graph->getNode(edge.getDownstreamElement()->getName());
        ElementPtr connectingElement = edge.getConnectingElement();
        if (downstream && connectingElement)
        {
            SgInput* input = downstream->getInput(connectingElement->getName());
            input->makeConnection(newNode->getOutput());
        }
    }

    // Set classification according to last node
    graph->_classification = graph->getOutputSocket()->connection->node->_classification;

    graph->finalize();

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
    SgNodePtr newNode = SgNode::creator(name, *nodeDef, shadergen, &node);
    _nodeMap[name] = newNode;
    _nodeOrder.push_back(newNode.get());

    // Check if any of the node inputs should be connected to the graph interface
    for (ValueElementPtr inputElem : node.getChildrenOfType<ValueElement>())
    {
        const string& interfaceName = inputElem->getInterfaceName();
        if (!interfaceName.empty())
        {
            SgOutput* inputSocket = getInputSocket(interfaceName);
            if (!inputSocket)
            {
                throw ExceptionShaderGenError("Interface name '" + interfaceName + "' doesn't match an existing input on nodegraph '" + getName() + "'");
            }
            SgInput* input = newNode->getInput(inputElem->getName());
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
            const string geomNodeName = "default_" + defaultGeomNode;
            SgNode* geomNode = getNode(geomNodeName);
            if (!geomNode)
            {
                string geomNodeDefName = "ND_" + defaultGeomNode + "__" + nodeDefInput->getType();
                NodeDefPtr geomNodeDef = node.getDocument()->getNodeDef(geomNodeDefName);
                if (!geomNodeDef)
                {
                    throw ExceptionShaderGenError("Could not find a nodedef named '" + geomNodeDefName +
                        "' for defaultgeomprop on input '" + node.getName() + "." + nodeDefInput->getName() + "'");
                }

                SgNodePtr geomNodePtr = SgNode::creator(geomNodeName, *geomNodeDef, shadergen);
                _nodeMap[geomNodeName] = geomNodePtr;
                _nodeOrder.push_back(geomNodePtr.get());

                geomNode = geomNodePtr.get();
            }

            input->makeConnection(geomNode->getOutput());
        }
    }

    return newNode.get();
}

SgNode* SgNodeGraph::addNode(const ShaderRef& shaderRef, ShaderGenerator& shadergen)
{
    NodeDefPtr nodeDef = shaderRef.getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Could not find a nodedef for shader '" + shaderRef.getName() + "'");
    }

    // Create this shader node in the graph.
    const string& name = shaderRef.getName();
    SgNodePtr newNode = SgNode::creator(name, *nodeDef, shadergen, nullptr);
    _nodeMap[name] = newNode;
    _nodeOrder.push_back(newNode.get());

    // Copy input values from all bindings
    for (BindParamPtr bindParam : shaderRef.getBindParams())
    {
        SgInput* input = newNode->getInput(bindParam->getName());
        if (input)
        {
            if (!bindParam->getValueString().empty())
            {
                input->value = bindParam->getValue();
            }
        }
    }
    for (BindInputPtr bindInput : shaderRef.getBindInputs())
    {
        SgInput* input = newNode->getInput(bindInput->getName());
        if (input)
        {
            if (!bindInput->getValueString().empty())
            {
                input->value = bindInput->getValue();
            }
        }
    }

    // Handle the "defaultgeomprop" directives on the nodedef inputs.
    // Create and connect default geometric nodes on unconnected inputs.
    for (const InputPtr& nodeDefInput : nodeDef->getInputs())
    {
        BindInputPtr bindInput = shaderRef.getBindInput(nodeDefInput->getName());

        const string& connection = bindInput ? bindInput->getNodeGraphString() : EMPTY_STRING;
        const string& defaultGeomNode = connection.empty() ? nodeDefInput->getAttribute("defaultgeomprop") : EMPTY_STRING;

        if (!defaultGeomNode.empty())
        {
            const string geomNodeName = "default_" + defaultGeomNode;
            SgNode* geomNode = getNode(geomNodeName);
            if (!geomNode)
            {
                string geomNodeDefName = "ND_" + defaultGeomNode + "__" + nodeDefInput->getType();
                NodeDefPtr geomNodeDef = shaderRef.getDocument()->getNodeDef(geomNodeDefName);
                if (!geomNodeDef)
                {
                    throw ExceptionShaderGenError("Could not find a nodedef named '" + geomNodeDefName +
                        "' for defaultgeomprop on input '" + shaderRef.getName() + "." + nodeDefInput->getName() + "'");
                }

                SgNodePtr geomNodePtr = SgNode::creator(geomNodeName, *geomNodeDef, shadergen);
                _nodeMap[geomNodeName] = geomNodePtr;
                _nodeOrder.push_back(geomNodePtr.get());

                geomNode = geomNodePtr.get();
            }

            SgInput* input = newNode->getInput(nodeDefInput->getName());
            input->makeConnection(geomNode->getOutput());
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

void SgNodeGraph::finalize()
{
    optimize();
    topologicalSort();
    calculateScopes();

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
                const float whichValue = value ? value->asA<float>() : 0.0f;
                const int branch = int(whichValue);

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
                disconnect(node);
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
