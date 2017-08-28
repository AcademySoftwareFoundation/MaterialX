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

const string NodeGraph::NODE_DEF_ATTRIBUTE = "nodedef";

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

NodeDefPtr Node::getReferencedNodeDef() const
{
    for (NodeDefPtr nodeDef : getDocument()->getMatchingNodeDefs(getCategory()))
    {
        if (nodeDef->getType() == getType())
        {
            bool matching = true;
            for (InputPtr input : getInputs())
            {
                InputPtr matchingInput = nodeDef->getInput(input->getName());
                if (!matchingInput || matchingInput->getType() != input->getType())
                {
                    matching = false;
                    continue;
                }
            }
            if (matching)
            {
                return nodeDef;
            }
        }
    }
    return NodeDefPtr();
}

ElementPtr Node::getImplementation(const string& target) const
{
    NodeDefPtr nodeDef = getReferencedNodeDef();
    if (nodeDef)
    {
        vector<ElementPtr> implementations = getDocument()->getMatchingImplementations(nodeDef->getName());
        for (ElementPtr implementation : implementations)
        {
            const string& implTarget = implementation->getTarget();
            if (implTarget.empty() || implTarget == target)
            {
                return implementation;
            }
        }
    }

    return ElementPtr();
}

Edge Node::getUpstreamEdge(MaterialPtr material, size_t index)
{
    if (index < getUpstreamEdgeCount())
    {
        InputPtr input = getInputs()[index];
        ElementPtr upstreamNode = input->getConnectedNode();
        if (upstreamNode)
        {
            return Edge(getSelf(), input, upstreamNode);
        }
        const string& interfaceName = input->getInterfaceName();
        if (!interfaceName.empty())
        {
            const string& nodeDefName = getParent()->asA<NodeGraph>()->getNodeDef();
            NodeDefPtr nodeDef = getDocument()->getNodeDef(nodeDefName);
            if (nodeDef)
            {
                ValueElementPtr interfacePort = nodeDef->getChildOfType<ValueElement>(interfaceName);
                if (interfacePort)
                {
                    return Edge(getSelf(), input, interfacePort);
                }
            }
        }
    }

    return NULL_EDGE;
}

vector<PortElementPtr> Node::getDownstreamPorts() const
{
    vector<PortElementPtr> downstreamPorts;
    for (PortElementPtr port : getDocument()->getMatchingPorts(getSelf()))
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
// NodeGraph methods
//

void NodeGraph::flattenSubgraphs(const string& target)
{
    vector<NodePtr> initialNodes = getNodes();
    std::deque<NodePtr> nodeQueue(initialNodes.begin(), initialNodes.end());

    while (!nodeQueue.empty())
    {
        NodePtr refNode = nodeQueue.front();
        nodeQueue.pop_front();

        ElementPtr implement = refNode->getImplementation(target);
        if (!implement || !implement->isA<NodeGraph>())
        {
            continue;
        }

        NodeGraphPtr origSubGraph = implement->asA<NodeGraph>();
        std::unordered_map<NodePtr, NodePtr> subNodeMap;

        // Create a new instance of each original subnode.
        for (NodePtr origSubNode : origSubGraph->getNodes())
        {
            string newName = createValidChildName(implement->getName() + "_" + origSubNode->getName());
            NodePtr newSubNode = addNode(origSubNode->getCategory(), newName);
            newSubNode->copyContentFrom(origSubNode);
            setChildIndex(newSubNode->getName(), getChildIndex(refNode->getName()));

            // Transfer interface properties from the reference node to the new subnode.
            for (ValueElementPtr newValue : newSubNode->getChildrenOfType<ValueElement>())
            {
                if (!newValue->hasInterfaceName())
                {
                    continue;
                }

                ValueElementPtr refValue = refNode->getChildOfType<ValueElement>(newValue->getInterfaceName());
                if (refValue)
                {
                    if (refValue->hasValueString())
                    {
                        newValue->setValueString(refValue->getValueString());
                    }
                    if (newValue->isA<Input>() && refValue->isA<Input>())
                    {
                        InputPtr refInput = refValue->asA<Input>();
                        if (refInput->hasNodeName())
                        {
                            InputPtr newInput = newValue->asA<Input>();
                            newInput->setNodeName(refInput->getNodeName());
                        }
                    }
                    if (refValue->hasInterfaceName())
                    {
                        newValue->setInterfaceName(refValue->getInterfaceName());
                    }
                    else
                    {
                        newValue->removeAttribute(ValueElement::INTERFACE_NAME_ATTRIBUTE);
                    }
                    newValue->setPublicName(refValue->getPublicName());
                }
            }

            // Store the mapping between subgraphs.
            subNodeMap[origSubNode] = newSubNode;

            // Check if the new subnode has a graph implementation.
            // If so this subgraph will need to be flattened as well.
            ElementPtr subNodeImplement = newSubNode->getImplementation(target);
            if (subNodeImplement && subNodeImplement->isA<NodeGraph>())
            {
                nodeQueue.push_back(newSubNode);
            }
        }

        // Transfer internal connections between subgraphs.
        for (auto subNodePair : subNodeMap)
        {
            NodePtr origSubNode = subNodePair.first;
            NodePtr newSubNode = subNodePair.second;
            for (PortElementPtr origPort : origSubNode->getDownstreamPorts())
            {
                if (origPort->isA<Input>())
                {
                    auto it = subNodeMap.find(origPort->getParent()->asA<Node>());
                    if (it != subNodeMap.end())
                    {
                        it->second->setConnectedNode(origPort->getName(), newSubNode);
                    }
                }
                else if (origPort->isA<Output>())
                {
                    for (PortElementPtr outerPort : refNode->getDownstreamPorts())
                    {
                        outerPort->setConnectedNode(newSubNode);
                    }
                }
            }
        }

        // The original referencing node has been replaced, so remove it from
        // the graph.
        removeNode(refNode->getName());
    }
}

vector<ElementPtr> NodeGraph::topologicalSort() const
{
    // Calculate a topological order of the children, using Kahn's algorithm
    // to avoid recursion.
    //
    // Running time: O(numNodes + numEdges).

    const vector<ElementPtr>& children = getChildren();

    // Calculate in-degrees for all nodes, and enqueue those with degree 0.
    std::unordered_map<ElementPtr, int> inDegree(children.size());
    std::deque<ElementPtr> childQueue;
    for (ElementPtr child : children)
    {
        int connectionCount = 0;
        if (child->isA<Output>())
        {
            connectionCount += int(!child->asA<Output>()->getNodeName().empty());
        }
        else
        {
            for (InputPtr input : child->getChildrenOfType<Input>())
            {
                connectionCount += int(!input->getNodeName().empty());
            }
        }

        inDegree[child] = connectionCount;

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
            for (auto port : child->asA<Node>()->getDownstreamPorts())
            {
                const ElementPtr downstreamElem = port->isA<Output>() ? port : port->getParent();
                if (--inDegree[downstreamElem] <= 0)
                {
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

} // namespace MaterialX
