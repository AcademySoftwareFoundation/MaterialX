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

SgNode::SgNode()
    : _classification(0)
    , _node(nullptr)
    , _nodeDef(nullptr)
    , _impl(nullptr)
{
}

SgNodePtr SgNode::creator(NodePtr node, ShaderGenerator& shadergen)
{
    NodeDefPtr nodeDef = node->getReferencedNodeDef();
    if (!nodeDef)
    {
        nodeDef = node->getReferencedNodeDef();
        throw ExceptionShaderGenError("Could not find a nodedef for node '" + node->getName() + "'");
    }

    // Find the implementation in the document (graph or implementation element)
    NodeGraphPtr implGraph;
    ImplementationPtr implElement;
    vector<ElementPtr> elements = nodeDef->getDocument()->getMatchingImplementations(nodeDef->getName());
    for (ElementPtr element : elements)
    {
        if (element->isA<NodeGraph>())
        {
            NodeGraphPtr candidate = element->asA<NodeGraph>();
            const string& matchingTarget = candidate->getTarget();
            if (matchingTarget.empty() || matchingTarget == shadergen.getTarget())
            {
                implGraph = candidate;
                break;
            }
        }
        else
        {
            ImplementationPtr candidate = element->asA<Implementation>();
            const string& matchingTarget = candidate->getTarget();
            if (candidate->getLanguage() == shadergen.getLanguage() && (matchingTarget.empty() || matchingTarget == shadergen.getTarget()))
            {
                implElement = candidate;
                break;
            }
        }
    }

    SgNodePtr sgNode;

    if (implGraph)
    {
        OutputPtr output = implGraph->getOutputs()[0];
        sgNode = SgNodeGraph::creator(output, shadergen);
    }
    else if (implElement)
    {
        sgNode = std::make_shared<SgNode>();
        sgNode->_impl = shadergen.getImplementation(*implElement);
    }

    if (sgNode)
    {
        sgNode->_node = node;
        sgNode->_nodeDef = nodeDef;

        const vector<ValueElementPtr> valueElements = sgNode->_nodeDef->getChildrenOfType<ValueElement>();

        for (const ValueElementPtr& elem : valueElements)
        {
            SgInputPtr input = std::make_shared<SgInput>();
            input->name = elem->getName();
            input->value = elem->getValue();
            input->connection = nullptr;
            input->parent = sgNode.get();

            sgNode->_inputMap[input->name] = input;
            sgNode->_inputOrder.push_back(input.get());
        }

        // TODO: Support multiple outputs
        SgOutputPtr output = std::make_shared<SgOutput>();
        output->name = "out";
        output->parent = sgNode.get();
        sgNode->_outputMap[output->name] = output;
        sgNode->_outputOrder.push_back(output.get());

        // Set node classification
        sgNode->_classification = Classification::TEXTURE;
        if (sgNode->_nodeDef->getType() == kSURFACE)
        {
            sgNode->_classification = Classification::SURFACE | Classification::SHADER;
        }
        else if (sgNode->_nodeDef->getType() == kBSDF)
        {
            sgNode->_classification = Classification::BSDF | Classification::CLOSURE;
        }
        else if (sgNode->_nodeDef->getType() == kEDF)
        {
            sgNode->_classification = Classification::EDF | Classification::CLOSURE;
        }
        else if (sgNode->_nodeDef->getType() == kVDF)
        {
            sgNode->_classification = Classification::VDF | Classification::CLOSURE;
        }
    }
    else
    {
        throw ExceptionShaderGenError("Could not find a matching implementation for node '" + nodeDef->getNode() +
            "' matching language '" + shadergen.getLanguage() + "' and target '" + shadergen.getTarget() + "'");
    }

    return sgNode;
}

const ValueElement& SgNode::getPort(const string& name) const
{
    ValueElementPtr port = _node->getChildOfType<ValueElement>(name);
    if (!port)
    {
        port = _nodeDef->getChildOfType<ValueElement>(name);
        if (!port)
        {
            throw ExceptionShaderGenError("Node '" + _node->getName() + "' has no input port named '" + name + "'");
        }
    }
    return *port;
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

SgOutputPtr SgNode::getOutput()
{
    return _outputMap.empty() ? nullptr : _outputMap.begin()->second;
}

SgNodeGraphPtr SgNodeGraph::creator(ElementPtr element, ShaderGenerator& shadergen)
{
    SgNodeGraphPtr graph = std::make_shared<SgNodeGraph>();

    OutputPtr graphOutput = element->asA<Output>();
    if (!graphOutput)
    {
        return graph;
    }



    MaterialPtr material;

    for (Edge edge : graphOutput->traverseGraph(material))
    {
/*
        NodePtr upstreamNode = optimize(edge);
        if (!upstreamNode)
        {
            continue;
        }
*/

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
                    
                const string& interfaceInputName = upstreamElement->getName();
                auto it = graph->_internalInputs.find(interfaceInputName);
                if (it != graph->_internalInputs.end())
                {
                    it->second.insert(downstreamInput.get());
                }
                else
                {
                    graph->_internalInputs[interfaceInputName] = { downstreamInput.get() };
                }
            }

            continue;
        }

        NodePtr upstreamNode = upstreamElement->asA<Node>();
        SgNodePtr newNode = graph->getNode(upstreamNode->getName());
        if (!newNode)
        {
            // Create this node in the new graph.
            newNode = SgNode::creator(upstreamNode, shadergen);
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
                SgOutputPtr output = newNode->getOutput();
                input->connection = output.get();
                output->connections.insert(input.get());
            }
        }
        else
        {
            OutputPtr interfaceOutput = edge.getDownstreamElement()->asA<Output>();
            graph->_internalOutputs[interfaceOutput->getName()] = newNode->getOutput().get();
        }
    }

    return graph;
}

SgNodePtr SgNodeGraph::getNode(const string& name)
{
    auto it = _nodeMap.find(name);
    return it != _nodeMap.end() ? it->second : nullptr;
}

void SgNodeGraph::flattenSubgraphs()
{
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
                    input->connection->connections.erase(input);

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
            connectionCount += int(input->connection != nullptr);
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
                if (--inDegree[input->parent] <= 0)
                {
                    nodeQueue.push_back(input->parent);
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

} // namespace MaterialX
