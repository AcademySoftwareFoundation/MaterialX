//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Node.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Material.h>

#include <deque>

namespace MaterialX
{

//
// Node methods
//

InputPtr Node::setConnectedNode(const string& inputName, NodePtr node)
{
    InputPtr input = getInput(inputName);
    if (!input)
    {
        input = addInput(inputName);
        if (node)
        {
            input->setType(node->getType());
        }
    }
    if (node)
    {
        input->setConnectedNode(node);
    }
    return input;
}

NodePtr Node::getConnectedNode(const string& inputName) const
{
    InputPtr input = getInput(inputName);
    if (!input)
    {
        return NodePtr();
    }
    return input->getConnectedNode();    
}

InputPtr Node::setConnectedNodeName(const string& inputName, const string& nodeName)
{
    InputPtr input = getInput(inputName);
    if (!input)
    {
        input = addInput(inputName);
    }
    input->setNodeName(nodeName);
    return input;
}

string Node::getConnectedNodeName(const string& inputName) const
{
    InputPtr input = getInput(inputName);
    if (!input)
    {
        return EMPTY_STRING;
    }
    return input->getNodeName();
}

NodeDefPtr Node::getNodeDef(const string& target) const
{
    if (hasNodeDefString())
    {
        return resolveRootNameReference<NodeDef>(getNodeDefString());
    }
    vector<NodeDefPtr> nodeDefs = getDocument()->getMatchingNodeDefs(getQualifiedName(getCategory()));
    vector<NodeDefPtr> secondary = getDocument()->getMatchingNodeDefs(getCategory());
    nodeDefs.insert(nodeDefs.end(), secondary.begin(), secondary.end());
    for (NodeDefPtr nodeDef : nodeDefs)
    {
        if (targetStringsMatch(nodeDef->getTarget(), target) &&
            nodeDef->isVersionCompatible(getSelf()) &&
            isTypeCompatible(nodeDef))
        {
            return nodeDef;
        }
    }
    return NodeDefPtr();
}

Edge Node::getUpstreamEdge(ConstMaterialPtr material, size_t index) const
{
    if (index < getUpstreamEdgeCount())
    {
        InputPtr input = getInputs()[index];
        ElementPtr upstreamNode = input->getConnectedNode();
        if (upstreamNode)
        {
            return Edge(getSelfNonConst(), input, upstreamNode);
        }
    }

    return NULL_EDGE;
}

OutputPtr Node::getNodeDefOutput(const Edge& edge)
{
    const ElementPtr connectingElement = edge.getConnectingElement();
    const PortElementPtr input = connectingElement ? connectingElement->asA<PortElement>() : nullptr;
    if (input)
    {
        NodeDefPtr nodeDef = getNodeDef();
        if (nodeDef)
        {
            return nodeDef->getActiveOutput(input->getOutputString());
        }
    }
    return OutputPtr();
}

vector<PortElementPtr> Node::getDownstreamPorts() const
{
    vector<PortElementPtr> downstreamPorts;
    for (PortElementPtr port : getDocument()->getMatchingPorts(getName()))
    {
        if (port->getConnectedNode() == getSelf())
        {
            downstreamPorts.push_back(port);
        }
    }
    return downstreamPorts;
}

bool Node::validate(string* message) const
{
    bool res = true;
    validateRequire(hasType(), res, message, "Missing type");
    return InterfaceElement::validate(message) && res;
}

//
// GraphElement methods
//

void GraphElement::flattenSubgraphs(const string& target)
{
    vector<NodePtr> processNodeVec = getNodes();
    while (!processNodeVec.empty())
    {
        // Precompute graph implementations and downstream ports for this node vector.
        using PortElementVec = vector<PortElementPtr>;
        std::unordered_map<NodePtr, NodeGraphPtr> graphImplMap;
        std::unordered_map<NodePtr, PortElementVec> downstreamPortMap;
        for (NodePtr cacheNode : processNodeVec)
        {
            InterfaceElementPtr implement = cacheNode->getImplementation(target);
            if (!implement || !implement->isA<NodeGraph>())
            {
                continue;
            }
            NodeGraphPtr subNodeGraph = implement->asA<NodeGraph>();
            graphImplMap[cacheNode] = subNodeGraph;
            downstreamPortMap[cacheNode] = cacheNode->getDownstreamPorts();
            for (NodePtr subNode : subNodeGraph->getNodes())
            {
                downstreamPortMap[subNode] = subNode->getDownstreamPorts();
            }
        }
        processNodeVec.clear();

        // Iterate through nodes with graph implementations.
        for (auto pair : graphImplMap)
        {
            NodePtr processNode = pair.first;
            NodeGraphPtr sourceSubGraph = pair.second;
            std::unordered_map<NodePtr, NodePtr> subNodeMap;

            // Create a new instance of each original subnode.
            for (NodePtr sourceSubNode : sourceSubGraph->getNodes())
            {
                string destName = createValidChildName(sourceSubGraph->getName() + "_" + sourceSubNode->getName());
                NodePtr destSubNode = addNode(sourceSubNode->getCategory(), destName);
                destSubNode->copyContentFrom(sourceSubNode);
                setChildIndex(destSubNode->getName(), getChildIndex(processNode->getName()));

                // Transfer interface properties from the reference node to the new subnode.
                for (ValueElementPtr destValue : destSubNode->getChildrenOfType<ValueElement>())
                {
                    if (!destValue->hasInterfaceName())
                    {
                        continue;
                    }

                    ValueElementPtr refValue = processNode->getChildOfType<ValueElement>(destValue->getInterfaceName());
                    if (refValue)
                    {
                        if (refValue->hasValueString())
                        {
                            destValue->setValueString(refValue->getValueString());
                        }
                        if (destValue->isA<Input>() && refValue->isA<Input>())
                        {
                            InputPtr refInput = refValue->asA<Input>();
                            InputPtr newInput = destValue->asA<Input>();
                            if (refInput->hasNodeName())
                            {
                                newInput->setNodeName(refInput->getNodeName());
                            }
                            if (refInput->hasOutputString())
                            {
                                newInput->setOutputString(refInput->getOutputString());
                            }
                        }
                    }
                    destValue->removeAttribute(ValueElement::INTERFACE_NAME_ATTRIBUTE);
                }

                // Store the mapping between subgraphs.
                subNodeMap[sourceSubNode] = destSubNode;

                // Add the subnode to the queue, allowing processing of nested subgraphs.
                processNodeVec.push_back(destSubNode);
            }

            // Transfer internal connections between subgraphs.
            for (auto subNodePair : subNodeMap)
            {
                NodePtr sourceSubNode = subNodePair.first;
                NodePtr destSubNode = subNodePair.second;
                for (PortElementPtr sourcePort : downstreamPortMap[sourceSubNode])
                {
                    if (sourcePort->isA<Input>())
                    {
                        auto it = subNodeMap.find(sourcePort->getParent()->asA<Node>());
                        if (it != subNodeMap.end())
                        {
                            it->second->setConnectedNode(sourcePort->getName(), destSubNode);
                        }
                    }
                    else if (sourcePort->isA<Output>())
                    {
                        for (PortElementPtr processNodePort : downstreamPortMap[processNode])
                        {
                            processNodePort->setConnectedNode(destSubNode);
                        }
                    }
                }
            }

            // The processed node has been replaced, so remove it from the graph.
            removeNode(processNode->getName());
        }
    }
}

vector<ElementPtr> GraphElement::topologicalSort() const
{
    // Calculate a topological order of the children, using Kahn's algorithm
    // to avoid recursion.
    //
    // Running time: O(numNodes + numEdges).

    const vector<ElementPtr>& children = getChildren();

    // Calculate in-degrees for all children.
    std::unordered_map<ElementPtr, size_t> inDegree(children.size());
    std::deque<ElementPtr> childQueue;
    for (ElementPtr child : children)
    {
        size_t connectionCount = 0;
        for (size_t i = 0; i < child->getUpstreamEdgeCount(); ++i)
        {
            if (child->getUpstreamEdge(nullptr, i))
            {
                connectionCount++;
            }
        }

        inDegree[child] = connectionCount;

        // Enqueue children with in-degree 0.
        if (connectionCount == 0)
        {
            childQueue.push_back(child);
        }
    }

    size_t visitCount = 0;
    vector<ElementPtr> result;

    while (!childQueue.empty())
    {
        // Pop the queue and add to topological order.
        ElementPtr child = childQueue.front();
        childQueue.pop_front();
        result.push_back(child);

        // Find connected nodes and decrease their in-degree, 
        // adding node to the queue if in-degrees becomes 0.
        if (child->isA<Node>())
        {
            for (PortElementPtr port : child->asA<Node>()->getDownstreamPorts())
            {
                const ElementPtr downstreamElem = port->isA<Output>() ? port : port->getParent();
                if (inDegree[downstreamElem] > 1)
                {
                    inDegree[downstreamElem]--;
                }
                else
                {
                    inDegree[downstreamElem] = 0;
                    childQueue.push_back(downstreamElem);
                }
            }
        }

        visitCount++;
    }

    // Check if there was a cycle.
    if (visitCount != children.size())
    {
        throw ExceptionFoundCycle("Encountered a cycle in graph: " + getName());
    }

    return result;
}

string GraphElement::asStringDot() const
{
    string dot = "digraph {\n";

    // Print the nodes
    for (NodePtr node : getNodes())
    {
        dot += "    \"" + node->getName() + "\" ";
        NodeDefPtr nodeDef = node->getNodeDef();
        const string& nodeGroup = nodeDef ? nodeDef->getNodeGroup() : EMPTY_STRING;
        if (nodeGroup == CONDITIONAL_NODE_GROUP)
        {
            dot += "[shape=diamond];\n";
        }
        else
        {
            dot += "[shape=box];\n";
        }
    }
 
    // Print the connections
    std::set<Edge> processedEdges;
    for (OutputPtr output : getOutputs())
    {
        for (Edge edge : output->traverseGraph())
        {
            if (!processedEdges.count(edge))
            {
                processedEdges.insert(edge);
                ElementPtr upstreamElem = edge.getUpstreamElement();
                ElementPtr downstreamElem = edge.getDownstreamElement();
                ElementPtr connectingElem = edge.getConnectingElement();
                dot += "    \"" + upstreamElem->getName();
                dot += "\" -> \"" + downstreamElem->getName();
                dot += "\" [label=\"";
                dot += connectingElem ? connectingElem->getName() : EMPTY_STRING;
                dot += "\"];\n";
            }
        }
    }

    dot += "}\n";

    return dot;
}

//
// NodeGraph methods
//

void NodeGraph::setNodeDef(ConstNodeDefPtr nodeDef)
{
    if (nodeDef)
    {
        setNodeDefString(nodeDef->getName());
    }
    else
    {
        removeAttribute(NODE_DEF_ATTRIBUTE);
    }
}

NodeDefPtr NodeGraph::getNodeDef() const
{
    return resolveRootNameReference<NodeDef>(getNodeDefString());
}

InterfaceElementPtr NodeGraph::getImplementation() const
{
    NodeDefPtr nodedef = getNodeDef();
    return nodedef ? nodedef->getImplementation() : InterfaceElementPtr();
}

bool NodeGraph::validate(string* message) const
{
    bool res = true;
    if (hasNodeDefString())
    {
        NodeDefPtr nodeDef = getNodeDef();
        validateRequire(nodeDef != nullptr, res, message, "NodeGraph implementation refers to non-existent NodeDef");
        if (nodeDef)
        {
            if (nodeDef->isMultiOutputType())
            {
                validateRequire(getOutputCount() == nodeDef->getOutputCount(), res, message, "NodeGraph implementation has a different number of outputs than its NodeDef");
            }
            else
            {
                validateRequire(getOutputCount() == 1, res, message, "NodeGraph implementation has a different number of outputs than its NodeDef");
            }
        }
    }
    return GraphElement::validate(message) && res;
}

ConstNodeDefPtr NodeGraph::getDeclaration(const string&) const
{
    return getNodeDef();
}

} // namespace MaterialX
