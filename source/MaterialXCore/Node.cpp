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

const string Backdrop::CONTAINS_ATTRIBUTE = "contains";
const string Backdrop::WIDTH_ATTRIBUTE = "width";
const string Backdrop::HEIGHT_ATTRIBUTE = "height";

//
// Node methods
//
NodePtr GraphElement::addNode(const string& category,
                              const string& name,
                              const string& type)
{
    if (category.empty())
    {
        throw Exception("No category specified: type: " + type + ". name: " + name + ". category: " + category);
    }
    if (type.empty())
    {
        throw Exception("No type specified: type: " + type + ". name: " + name + ". category: " + category);
    }
    NodePtr node = addChild<Node>(name);
    node->setCategory(category);
    node->setType(type);
    return node;
}

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

OutputPtr Node::getNodeDefOutput(ElementPtr connectingElement)
{
    const string* outputName = nullptr;
    const PortElementPtr port = connectingElement->asA<PortElement>();
    if (port)
    {
        // The connecting element is an input/output port,
        // so get the name of the output connected to this 
        // port. If no explicit output is specified this will
        // return an empty string which is handled below.
        outputName = &port->getOutputString();

        // Handle case where it's an input to a top level output
        InputPtr connectedInput = connectingElement->asA<Input>();
        OutputPtr output = OutputPtr();
        if (connectedInput)
        {
            output = connectedInput->getConnectedOutput();
        }
        if (output)
        {
            if (output->getParent() == output->getDocument())
            {
                outputName = &output->getOutputString();
            }
        }
    }
    else
    {
        const BindInputPtr bindInput = connectingElement->asA<BindInput>();
        if (bindInput)
        {
            // Handle the case where the edge involves a bindinput.
            const OutputPtr output = bindInput->getConnectedOutput();
            if (output)
            {
                if (output->getParent()->isA<NodeGraph>())
                {
                    // The bindinput connects to a graph output,
                    // so this is the output we're looking for.
                    outputName = &output->getName();
                }
                else
                {
                    // The bindinput connects to a free floating output,
                    // so we have an extra level of indirection. Hence 
                    // get its connected output.
                    outputName = &output->getOutputString();
                }
            }
        }
    }
    if (outputName && !outputName->empty())
    {
        // Find this output on our nodedef.
        NodeDefPtr nodeDef = getNodeDef();
        if (nodeDef)
        {
            return nodeDef->getActiveOutput(*outputName);
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
    std::sort(downstreamPorts.begin(), downstreamPorts.end(), [](const ConstElementPtr& a, const ConstElementPtr& b)
    {
        return a->getName() > b->getName();
    });
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
        for (const auto& pair : graphImplMap)
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
            for (const auto& subNodePair : subNodeMap)
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

    // Create a unique name for each child element.
    vector<ElementPtr> children = topologicalSort();
    StringMap nameMap;
    StringSet nameSet;
    for (ElementPtr elem : children)
    {
        string uniqueName = elem->getCategory();
        while(nameSet.count(uniqueName))
        {
            uniqueName = incrementName(uniqueName);
        }
        nameMap[elem->getName()] = uniqueName;
        nameSet.insert(uniqueName);
    }

    // Write out all nodes.
    for (ElementPtr elem : children)
    {
        NodePtr node = elem->asA<Node>();
        if (node)
        {
            dot += "    \"" + nameMap[node->getName()] + "\" ";
            NodeDefPtr nodeDef = node->getNodeDef();
            const string& nodeGroup = nodeDef ? nodeDef->getNodeGroup() : EMPTY_STRING;
            if (nodeGroup == NodeDef::CONDITIONAL_NODE_GROUP)
            {
                dot += "[shape=diamond];\n";
            }
            else
            {
                dot += "[shape=box];\n";
            }
        }
    }

    // Write out all connections.
    std::set<Edge> processedEdges;
    StringSet processedInterfaces;
    for (OutputPtr output : getOutputs())
    {
        for (Edge edge : output->traverseGraph())
        {
            if (!processedEdges.count(edge))
            {
                ElementPtr upstreamElem = edge.getUpstreamElement();
                ElementPtr downstreamElem = edge.getDownstreamElement();
                ElementPtr connectingElem = edge.getConnectingElement();

                dot += "    \"" + nameMap[upstreamElem->getName()];
                dot += "\" -> \"" + nameMap[downstreamElem->getName()];
                dot += "\" [label=\"";
                dot += connectingElem ? connectingElem->getName() : EMPTY_STRING;
                dot += "\"];\n";

                NodePtr upstreamNode = upstreamElem->asA<Node>();
                if (upstreamNode && !processedInterfaces.count(upstreamNode->getName()))
                {
                    for (InputPtr input : upstreamNode->getInputs())
                    {
                        if (input->hasInterfaceName())
                        {
                            dot += "    \"" + input->getInterfaceName();
                            dot += "\" -> \"" + nameMap[upstreamElem->getName()];
                            dot += "\" [label=\"";
                            dot += input->getName();
                            dot += "\"];\n";
                        }
                    }
                    processedInterfaces.insert(upstreamNode->getName());
                }

                processedEdges.insert(edge);
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
            validateRequire(getOutputCount() == nodeDef->getActiveOutputs().size(), res, message, "NodeGraph implementation has a different number of outputs than its NodeDef");
        }
    }
    return GraphElement::validate(message) && res;
}

ConstNodeDefPtr NodeGraph::getDeclaration(const string&) const
{
    return getNodeDef();
}

} // namespace MaterialX
