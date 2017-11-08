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

void SgInput::breakConnection(SgOutput* src)
{
    src->connections.erase(this);
    this->connection = nullptr;
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

SgEdgeIterator SgOutput::traverseUpstream()
{
    return SgEdgeIterator(this);
}


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

void SgNode::ScopeInfo::adjustAtConditionalInput(const NodePtr& condNode, int branch, const uint32_t fullMask)
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
        // NOTE: Right now multiple scopes is not really used, it works exactly as GLOBAL_SCOPE
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
    SgNodePtr sgNode = std::make_shared<SgNode>(name);

    // Find the implementation in the document (graph or implementation element)
    vector<ElementPtr> elements = nodeDef.getDocument()->getMatchingImplementations(nodeDef.getName());
    for (ElementPtr element : elements)
    {
        if (element->isA<NodeGraph>())
        {
            NodeGraphPtr implGraph = element->asA<NodeGraph>();
            const string& matchingTarget = implGraph->getTarget();
            if (matchingTarget.empty() || matchingTarget == shadergen.getTarget())
            {
                sgNode->_impl = shadergen.getImplementation(implGraph);
                break;
            }
        }
        else
        {
            ImplementationPtr implElement = element->asA<Implementation>();
            const string& matchingTarget = implElement->getTarget();
            if (implElement->getLanguage() == shadergen.getLanguage() && (matchingTarget.empty() || matchingTarget == shadergen.getTarget()))
            {
                sgNode->_impl = shadergen.getImplementation(implElement);
                break;
            }
        }
    }

    if (!sgNode->_impl)
    {
        throw ExceptionShaderGenError("Could not find a matching implementation for node '" + nodeDef.getNode() +
            "' matching language '" + shadergen.getLanguage() + "' and target '" + shadergen.getTarget() + "'");
    }

    const vector<ValueElementPtr> nodeDefInputs = nodeDef.getChildrenOfType<ValueElement>();
    for (const ValueElementPtr& elem : nodeDefInputs)
    {
        InputPtr in = elem->asA<Input>();
        sgNode->addInput(elem->getName(), elem->getType(), in ? in->getChannels() : EMPTY_STRING, elem->getValue());
    }

    if (nodeInstance)
    {
        // Fill in data from the node instance
        const vector<ValueElementPtr> nodeInstanceInputs = nodeInstance->getChildrenOfType<ValueElement>();
        for (const ValueElementPtr& elem : nodeInstanceInputs)
        {
            SgInputPtr input = sgNode->getInput(elem->getName());

            if (!elem->getValueString().empty())
            {
                input->value = elem->getValue();
            }

            InputPtr in = elem->asA<Input>();
            if (in)
            {
                input->channels = in->getChannels();
            }
        }
    }
    
    // TODO: Support multiple outputs
    sgNode->addOutput("out", nodeDef.getType(), "");

    // Set node classification
    sgNode->_classification = Classification::TEXTURE;
    if (nodeDef.getType() == kSURFACE)
    {
        sgNode->_classification = Classification::SURFACE | Classification::SHADER;
    }
    else if (nodeDef.getType() == kBSDF)
    {
        sgNode->_classification = Classification::BSDF | Classification::CLOSURE;
    }
    else if (nodeDef.getType() == kEDF)
    {
        sgNode->_classification = Classification::EDF | Classification::CLOSURE;
    }
    else if (nodeDef.getType() == kVDF)
    {
        sgNode->_classification = Classification::VDF | Classification::CLOSURE;
    }
    else if (nodeDef.getNode() == "image")
    {
        sgNode->_classification = Classification::TEXTURE | Classification::FILETEXTURE;
    }
    else if (nodeDef.getNode() == "compare" || nodeDef.getNode() == "switch")
    {
        sgNode->_classification = Classification::TEXTURE | Classification::CONDITIONAL;
    }

    return sgNode;
}

SgInputPtr SgNode::getInput(const string& name)
{
    auto it = _inputMap.find(name);
    return it != _inputMap.end() ? it->second : nullptr;
}

SgOutputPtr SgNode::getOutput(const string& name)
{
    auto it = _outputMap.find(name);
    return it != _outputMap.end() ? it->second : nullptr;
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

SgInput* SgNode::addInput(const string& name, const string& type, const string& channels, ValuePtr value)
{
    SgInputPtr input = std::make_shared<SgInput>();
    input->name = name;
    input->type = type;
    input->node = this;
    input->value = value;
    input->connection = nullptr;
    input->channels = channels;
    input->isSocket = false;
    _inputMap[name] = input;
    _inputOrder.push_back(input.get());

    return input.get();
}

SgOutput* SgNode::addOutput(const string& name, const string& type, const string&)
{
    SgOutputPtr output = std::make_shared<SgOutput>();
    output->name = name;
    output->type = type;
    output->node = this;
    output->isSocket = false;
    _outputMap[name] = output;
    _outputOrder.push_back(output.get());

    return output.get();
}

SgNodeGraph::SgNodeGraph(const string& name)
    : SgNode(name)
{
}

SgNodeGraphPtr SgNodeGraph::creator(NodeGraphPtr nodeGraph, ShaderGenerator& shadergen)
{
    SgNodeGraphPtr graph = std::make_shared<SgNodeGraph>(nodeGraph->getName());

    NodeDefPtr graphDef = nodeGraph->getDocument()->getNodeDef(nodeGraph->getNodeDef());
    if (!graphDef)
    {
        throw ExceptionShaderGenError("Can't find nodedef '" + nodeGraph->getNodeDef() + "' referenced by nodegraph '" + nodeGraph->getName() + "'");
    }

    for (const ValueElementPtr& elem : graphDef->getChildrenOfType<ValueElement>())
    {
        InputPtr graphInput = elem->asA<Input>();
        graph->addInput(elem->getName(), elem->getType(),
            graphInput ? graphInput->getChannels() : EMPTY_STRING, 
            elem->getValueString().empty() ? nullptr : elem->getValue());
    }

    for (const OutputPtr& graphOutput : nodeGraph->getOutputs())
    {
        graph->addOutput(graphOutput->getName(), graphOutput->getType(), graphOutput->getChannels());
    }

    // Traverse all outputs and create all dependencies upstream
    for (OutputPtr graphOutput : nodeGraph->getOutputs())
    {
        for (Edge edge : graphOutput->traverseGraph())
        {
            ElementPtr upstreamElement = edge.getUpstreamElement();
            if (upstreamElement->isA<Output>())
            {
                upstreamElement = upstreamElement->asA<Output>()->getConnectedNode();
                if (!upstreamElement)
                {
                    continue;
                }
            }

            // Check if this is a connection to the graph interface
            if (upstreamElement->getParent()->isA<NodeDef>())
            {
                // Find the downstream input this came from and connect to the graph interface
                SgNodePtr downstream = graph->getNode(edge.getDownstreamElement()->getName());
                if (downstream)
                {
                    SgInputPtr downstreamInput = downstream->getInput(edge.getConnectingElement()->getName());
                    SgOutput* inputSocket = graph->getInputSocket(upstreamElement->getName());
                    inputSocket->makeConnection(downstreamInput.get());
                }

                continue;
            }

            NodePtr upstreamNode = upstreamElement->asA<Node>();
            SgNodePtr newNode = graph->getNode(upstreamNode->getName());
            if (!newNode)
            {
                // Create this node in the new graph.
                NodeDefPtr nodeDef = upstreamNode->getReferencedNodeDef();
                if (!nodeDef)
                {
                    throw ExceptionShaderGenError("Could not find a nodedef for node '" + upstreamNode->getName() + "'");
                }
                newNode = SgNode::creator(upstreamNode->getName(), *nodeDef, shadergen, upstreamNode.get());
                graph->_nodeMap[newNode->getName()] = newNode;
            }

            // Make connections
            NodePtr downstreamNode = edge.getDownstreamElement()->asA<Node>();
            if (downstreamNode)
            {
                SgNodePtr downstream = graph->getNode(downstreamNode->getName());
                ElementPtr connectingElement = edge.getConnectingElement();
                if (downstream && connectingElement)
                {
                    SgInputPtr input = downstream->getInput(connectingElement->getName());
                    input->makeConnection(newNode->getOutput());
                }
            }
            else
            {
                // This is a graph interface output
                SgInput* outputSocket = graph->getOutputSocket(edge.getDownstreamElement()->getName());
                outputSocket->makeConnection(newNode->getOutput());
            }
        }
    }

    graph->finalize();

    return graph;
}

SgNodeGraphPtr SgNodeGraph::creator(const string& name, ElementPtr element, ShaderGenerator& shadergen)
{
    SgNodeGraphPtr graph;
    ElementPtr root;
    MaterialPtr material;

    if (element->isA<Output>() && element->getParent()->isA<NodeGraph>())
    {
        NodeGraphPtr nodeGraph = element->getParent()->asA<NodeGraph>();
        OutputPtr nodeGraphOutput = element->asA<Output>();

        NodePtr srcNode = nodeGraphOutput->getConnectedNode();
        if (!srcNode)
        {
            ExceptionShaderGenError("Given output element '" + element->getName() + "' has no node connection");
        }

        NodeDefPtr nodeDef = srcNode->getReferencedNodeDef();
        if (!nodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef for node '" + srcNode->getName() + "'");
        }

        graph = std::make_shared<SgNodeGraph>(name);
        graph->addOutput(nodeGraphOutput->getName(), nodeGraphOutput->getType());

        // Create this node in the new graph and connect it to the graph interface
        SgNodePtr sgNode = SgNode::creator(srcNode->getName(), *nodeDef, shadergen, srcNode.get());
        graph->_nodeMap[sgNode->getName()] = sgNode;
        graph->getOutputSocket(nodeGraphOutput->getName())->makeConnection(sgNode->getOutput());

        // Start traveral from this node
        root = srcNode;
    }
    else if (element->isA<Node>())
    {
        NodePtr srcNode = element->asA<Node>();

        NodeDefPtr nodeDef = srcNode->getReferencedNodeDef();
        if (!nodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef for node '" + srcNode->getName() + "'");
        }

        graph = std::make_shared<SgNodeGraph>(name);
        graph->addOutput("out", element->asA<Node>()->getType());

        // Create this node in the new graph and connect it to the graph interface
        SgNodePtr sgNode = SgNode::creator(srcNode->getName(), *nodeDef, shadergen, srcNode.get());
        graph->_nodeMap[sgNode->getName()] = sgNode;
        graph->getOutputSocket("out")->makeConnection(sgNode->getOutput());

        // Start traveral from this node
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

        graph = std::make_shared<SgNodeGraph>(name);
        graph->addOutput("out", nodeDef->getType());


        //... TODO ...
        // Create inputs from nodedef


        // Create this node in the new graph and connect it to the graph interface
        SgNodePtr sgNode = SgNode::creator(shaderRef->getName(), *nodeDef, shadergen);
        graph->_nodeMap[sgNode->getName()] = sgNode;
        graph->getOutputSocket("out")->makeConnection(sgNode->getOutput());

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
        if (upstreamElement->isA<Output>())
        {
            upstreamElement = upstreamElement->asA<Output>()->getConnectedNode();
            if (!upstreamElement)
            {
                continue;
            }
        }

        // Check if this is a connection to the graph interface
        if (upstreamElement->getParent()->isA<NodeDef>())
        {
            // Find the downstream input this came from and connect to the graph interface
            SgNodePtr downstreamNode = graph->getNode(edge.getDownstreamElement()->getName());
            if (downstreamNode)
            {
                SgInputPtr downstreamInput = downstreamNode->getInput(edge.getConnectingElement()->getName());
                SgOutput* inputSocket = graph->getInputSocket(upstreamElement->getName());
                inputSocket->makeConnection(downstreamInput.get());
            }

            continue;
        }

        NodePtr upstreamNode = upstreamElement->asA<Node>();
        SgNodePtr newNode = graph->getNode(upstreamNode->getName());
        if (!newNode)
        {
            // Create this node in the new graph.
            NodeDefPtr nodeDef = upstreamNode->getReferencedNodeDef();
            if (!nodeDef)
            {
                throw ExceptionShaderGenError("Could not find a nodedef for node '" + upstreamNode->getName() + "'");
            }
            newNode = SgNode::creator(upstreamNode->getName(), *nodeDef, shadergen, upstreamNode.get());
            graph->_nodeMap[newNode->getName()] = newNode;
        }

        // Make connections.
        ElementPtr downstreamNode = edge.getDownstreamElement()->asA<Node>();
        if (downstreamNode)
        {
            SgNodePtr downstream = graph->getNode(downstreamNode->getName());
            ElementPtr connectingElement = edge.getConnectingElement();
            if (downstream && connectingElement)
            {
                SgInputPtr input = downstream->getInput(connectingElement->getName());
                input->makeConnection(newNode->getOutput());
            }
        }
    }

    graph->finalize();

    return graph;
}

SgInput* SgNodeGraph::addInput(const string& name, const string& type, const string& channels, ValuePtr value)
{
    SgOutputPtr inputSocket = std::make_shared<SgOutput>();
    inputSocket->name = name;
    inputSocket->type = type;
    inputSocket->node = this;
    inputSocket->isSocket = true;
    _inputSocketMap[name] = inputSocket;
    _inputSocketOrder.push_back(inputSocket.get());

    return SgNode::addInput(name, type, channels, value);
}

SgOutput* SgNodeGraph::addOutput(const string& name, const string& type, const string& channels)
{
    SgInputPtr outputSocket = std::make_shared<SgInput>();
    outputSocket->name = name;
    outputSocket->type = type;
    outputSocket->node = this;
    outputSocket->connection = nullptr;
    outputSocket->value = nullptr;
    outputSocket->channels = channels;
    outputSocket->isSocket = true;
    _outputSocketMap[name] = outputSocket;
    _outputSocketOrder.push_back(outputSocket.get());

    return SgNode::addOutput(name, type, channels);
}

SgNodePtr SgNodeGraph::getNode(const string& name)
{
    auto it = _nodeMap.find(name);
    return it != _nodeMap.end() ? it->second : nullptr;
}

SgOutput* SgNodeGraph::getInputSocket(const string& name)
{
    auto it = _inputSocketMap.find(name);
    return it != _inputSocketMap.end() ? it->second.get() : nullptr;
}

const SgOutput* SgNodeGraph::getInputSocket(const string& name) const
{
    auto it = _inputSocketMap.find(name);
    return it != _inputSocketMap.end() ? it->second.get() : nullptr;
}

SgInput* SgNodeGraph::getOutputSocket(const string& name)
{
    auto it = _outputSocketMap.find(name);
    return it != _outputSocketMap.end() ? it->second.get() : nullptr;
}

const SgInput* SgNodeGraph::getOutputSocket(const string& name) const
{
    auto it = _outputSocketMap.find(name);
    return it != _outputSocketMap.end() ? it->second.get() : nullptr;
}

void SgNodeGraph::flattenSubgraphs()
{
/*
    std::deque<SgNodePtr> nodeQueue;
    for (auto it : _nodeMap)
    {
        nodeQueue.push_back(it.second);
    }

    while (!nodeQueue.empty())
    {
        SgNodePtr currentNode = nodeQueue.front();
        nodeQueue.pop_front();

        if (currentNode->isNodeGraph())
        {
            SgNodeGraph* graph = static_cast<SgNodeGraph*>(currentNode.get());

            for (auto it : graph->_nodeMap)
            {
                if (it.second->isNodeGraph())
                {
                    nodeQueue.push_back(it.second);
                }
                else
                {
                    _nodeMap[it.first] = it.second;
                }
            }

            // Re-route input connections
            for (SgInput* input : currentNode->getInputs())
            {
                if (input->connection)
                {
                    // Remove direct connection to graph
                    input->connection->breakConnection() ->connections.erase(input);

                    // Add input connections to graph internal nodes
                    auto it = graph->_internalInputs.find(input->name);
                    if (it != graph->_internalInputs.end())
                    {
                        for (auto input2 : it->second)
                        {
                            input2->connection = input->connection;
                            input->connection->connections.insert(input2);
                        }
                    }
                }
            }

            // Re-route output connections
            for (SgOutput* output : currentNode->getOutputs())
            {
                // Add output connections to graph internal nodes
                auto it = graph->_internalOutputs.find(output->name);
                if (it != graph->_internalOutputs.end())
                {
                    for (auto input : output->connections)
                    {
                        input->connection = it->second;
                        it->second->connections.insert(input);
                    }
                }
            }

            // Remove the graph
            _nodeMap.erase(currentNode->getName());
        }
    }
*/
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

    _nodeOrder.resize(_nodeMap.size());
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

void SgNodeGraph::finalize()
{
    topologicalSort();

    // Track closure nodes used by each surface shader.
    for (SgNode* node : _nodeOrder)
    {
        if (node->hasClassification(SgNode::Classification::SHADER))
        {

            /*
            for (Edge edge : node->getNodePtr()->traverseGraph())
            {
                NodePtr upstreamNode = edge.getUpstreamElement()->asA<Node>();
                if (upstreamNode)
                {
                    const SgNode& upstreamSgNode = getNode(upstreamNode);
                    if (upstreamSgNode.hasClassification(SgNode::Classification::CLOSURE))
                    {
                        node->_usedClosures.insert(&upstreamSgNode);
                    }
                }
            }
            */
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
        if (output)
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
            if (output)
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
