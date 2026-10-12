//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXCore/Node.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Material.h>

#include <deque>
#include <map>
#include <unordered_set>

MATERIALX_NAMESPACE_BEGIN

namespace
{

void clearPortConnection(PortElementPtr port)
{
    if (!port)
    {
        return;
    }
    port->removeAttribute(PortElement::NODE_NAME_ATTRIBUTE);
    port->removeAttribute(PortElement::NODE_GRAPH_ATTRIBUTE);
    port->removeAttribute(PortElement::OUTPUT_ATTRIBUTE);
    port->removeAttribute(ValueElement::INTERFACE_NAME_ATTRIBUTE);
}

void copyConnectionAttributes(PortElementPtr source, PortElementPtr dest)
{
    if (!source || !dest)
    {
        return;
    }

    clearPortConnection(dest);
    if (source->hasNodeName())
    {
        dest->setNodeName(source->getNodeName());
    }
    if (source->hasNodeGraphString())
    {
        dest->setNodeGraphString(source->getNodeGraphString());
    }
    if (source->hasOutputString())
    {
        dest->setOutputString(source->getOutputString());
    }
    if (source->hasInterfaceName())
    {
        dest->setInterfaceName(source->getInterfaceName());
    }
    dest->removeAttribute(ValueElement::VALUE_ATTRIBUTE);
}

bool inputConnectsOutsideSelection(InputPtr input, const std::unordered_set<string>& selectedNodeNames)
{
    if (!input)
    {
        return false;
    }
    if (input->hasNodeName())
    {
        return selectedNodeNames.find(input->getNodeName()) == selectedNodeNames.end();
    }
    return input->hasNodeGraphString() || input->hasInterfaceName() || input->hasOutputString();
}

void preserveExternalUpstreamConnections(NodeGraphPtr nodeGraph, const std::unordered_set<string>& selectedNodeNames)
{
    if (!nodeGraph)
    {
        return;
    }

    for (NodePtr copiedNode : nodeGraph->getNodes())
    {
        for (InputPtr input : copiedNode->getInputs())
        {
            if (!inputConnectsOutsideSelection(input, selectedNodeNames))
            {
                continue;
            }

            string interfaceName = nodeGraph->createValidChildName(copiedNode->getName() + "_" + input->getName());
            InputPtr interfaceInput = nodeGraph->addInput(interfaceName, input->getType());
            copyConnectionAttributes(input, interfaceInput);
            input->setConnectedInterfaceName(interfaceInput->getName());
        }
    }
}

string getBoundaryOutputType(NodePtr sourceNode, PortElementPtr downstreamPort)
{
    if (downstreamPort && !downstreamPort->getType().empty())
    {
        return downstreamPort->getType();
    }
    if (sourceNode)
    {
        const string& outputName = downstreamPort ? downstreamPort->getOutputString() : EMPTY_STRING;
        if (!outputName.empty())
        {
            OutputPtr output = sourceNode->getOutput(outputName);
            if (!output && sourceNode->getNodeDef())
            {
                output = sourceNode->getNodeDef()->getOutput(outputName);
            }
            if (output && !output->getType().empty())
            {
                return output->getType();
            }
        }
        return sourceNode->getType();
    }
    return DEFAULT_TYPE_STRING;
}

OutputPtr getOrCreateBoundaryOutput(NodeGraphPtr nodeGraph,
                                    NodePtr sourceNode,
                                    PortElementPtr downstreamPort,
                                    std::map<string, OutputPtr>& boundaryOutputs)
{
    const string& sourceOutput = downstreamPort ? downstreamPort->getOutputString() : EMPTY_STRING;
    string key = sourceNode->getName() + "|" + sourceOutput;
    auto existing = boundaryOutputs.find(key);
    if (existing != boundaryOutputs.end())
    {
        return existing->second;
    }

    string outputBaseName = sourceNode->getName() + (sourceOutput.empty() ? "_out" : "_" + sourceOutput);
    OutputPtr boundaryOutput = nodeGraph->addOutput(nodeGraph->createValidChildName(outputBaseName),
                                                    getBoundaryOutputType(sourceNode, downstreamPort));
    boundaryOutput->setNodeName(sourceNode->getName());
    if (!sourceOutput.empty())
    {
        boundaryOutput->setOutputString(sourceOutput);
    }
    boundaryOutput->removeAttribute(ValueElement::VALUE_ATTRIBUTE);
    boundaryOutputs[key] = boundaryOutput;
    return boundaryOutput;
}

void preserveExternalDownstreamConnections(NodeGraphPtr nodeGraph,
                                           const vector<NodePtr>& nodesToGroup,
                                           const std::unordered_set<string>& selectedNodeNames)
{
    if (!nodeGraph)
    {
        return;
    }

    std::map<string, OutputPtr> boundaryOutputs;
    for (NodePtr sourceNode : nodesToGroup)
    {
        if (!sourceNode)
        {
            continue;
        }
        for (PortElementPtr downstreamPort : sourceNode->getDownstreamPorts())
        {
            if (!downstreamPort)
            {
                continue;
            }

            ElementPtr parent = downstreamPort->getParent();
            if (parent && selectedNodeNames.find(parent->getName()) != selectedNodeNames.end())
            {
                continue;
            }

            OutputPtr boundaryOutput = getOrCreateBoundaryOutput(nodeGraph, sourceNode, downstreamPort, boundaryOutputs);
            downstreamPort->setNodeGraphString(nodeGraph->getName());
            downstreamPort->setOutputString(boundaryOutput->getName());
            downstreamPort->removeAttribute(PortElement::NODE_NAME_ATTRIBUTE);
            downstreamPort->removeAttribute(ValueElement::INTERFACE_NAME_ATTRIBUTE);
            downstreamPort->removeAttribute(ValueElement::VALUE_ATTRIBUTE);
        }
    }
}

} // anonymous namespace

const string Backdrop::CONTAINS_ATTRIBUTE = "contains";
const string Backdrop::WIDTH_ATTRIBUTE = "width";
const string Backdrop::HEIGHT_ATTRIBUTE = "height";

//
// Node methods
//

void Node::setNameGlobal(const string& name)
{
    vector<PortElementPtr> downStreamPorts = getDownstreamPorts();
    setName(name);
    const string& newName = getName();
    for (PortElementPtr& port : downStreamPorts)
    {
        port->setNodeName(newName);
    }
}

void Node::setConnectedNode(const string& inputName, ConstNodePtr node)
{
    InputPtr input = getInput(inputName);
    if (!input)
    {
        input = addInput(inputName);
    }
    if (node)
    {
        const string& type = node->getType();
        if (type != MULTI_OUTPUT_TYPE_STRING)
        {
            input->setType(type);
        }
    }
    input->setConnectedNode(node);
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

void Node::setConnectedNodeName(const string& inputName, const string& nodeName)
{
    InputPtr input = getInput(inputName);
    if (!input)
    {
        input = addInput(inputName);
    }
    input->setNodeName(nodeName);
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

NodeDefPtr Node::getNodeDef(const string& target, bool allowRoughMatch) const
{
    if (hasNodeDefString())
    {
        return resolveNameReference<NodeDef>(getNodeDefString());
    }
    vector<NodeDefPtr> nodeDefs = getDocument()->getMatchingNodeDefs(getQualifiedName(getCategory()));
    vector<NodeDefPtr> secondary = getDocument()->getMatchingNodeDefs(getCategory());
    vector<NodeDefPtr> roughMatches;
    nodeDefs.insert(nodeDefs.end(), secondary.begin(), secondary.end());
    for (NodeDefPtr nodeDef : nodeDefs)
    {
        if (!targetStringsMatch(nodeDef->getTarget(), target) ||
            !nodeDef->isVersionCompatible(getVersionString()) ||
            nodeDef->getType() != getType())
        {
            continue;
        }
        if (!hasExactInputMatch(nodeDef))
        {
            if (allowRoughMatch)
            {
                roughMatches.push_back(nodeDef);
            }
            continue;
        }
        return nodeDef;
    }
    if (!roughMatches.empty())
    {
        return roughMatches[0];
    }
    return NodeDefPtr();
}

Edge Node::getUpstreamEdge(size_t index) const
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

    return getNullEdge();
}

OutputPtr Node::getNodeDefOutput(ElementPtr connectingElement)
{
    string outputName;
    const PortElementPtr port = connectingElement->asA<PortElement>();
    if (port)
    {
        // The connecting element is an input/output port,
        // so get the name of the output connected to this
        // port. If no explicit output is specified this will
        // return an empty string which is handled below.
        outputName = port->getOutputString();

        // Handle case where it's an input to a top level output
        InputPtr connectedInput = connectingElement->asA<Input>();
        OutputPtr output;
        if (connectedInput)
        {
            InputPtr interfaceInput = connectedInput->getInterfaceInput();
            if (interfaceInput)
            {
                output = interfaceInput->getConnectedOutput();
                outputName = interfaceInput->getOutputString();
            }
            else
            {
                output = connectedInput->getConnectedOutput();
            }
        }
        if (output)
        {
            if (connectedInput ||
                output->getParent() == output->getDocument())
            {
                if (!output->getOutputString().empty())
                {
                    outputName = output->getOutputString();
                }
            }
        }
    }
    if (!outputName.empty())
    {
        // Find this output on our nodedef.
        NodeDefPtr nodeDef = getNodeDef();
        if (nodeDef)
        {
            return nodeDef->getActiveOutput(outputName);
        }
    }
    return OutputPtr();
}

vector<PortElementPtr> Node::getDownstreamPorts() const
{
    vector<PortElementPtr> downstreamPorts;
    for (PortElementPtr port : getDocument()->getMatchingPorts(getQualifiedName(getName())))
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
    validateRequire(!getCategory().empty(), res, message, "Node element is missing a category");
    validateRequire(hasType(), res, message, "Node element is missing a type");

    NodeDefPtr nodeDef = getNodeDef(EMPTY_STRING, true);
    if (nodeDef)
    {
        string matchMessage;
        bool exactMatch = hasExactInputMatch(nodeDef, &matchMessage);
        validateRequire(exactMatch, res, message, "Node interface error: " + matchMessage);

        const vector<OutputPtr>& activeOutputs = nodeDef->getActiveOutputs();
        const size_t numActiveOutputs = activeOutputs.size();
        if (numActiveOutputs > 1)
        {
            validateRequire(getType() == MULTI_OUTPUT_TYPE_STRING, res, message, "Node type is not 'multioutput' for node with multiple outputs");
        }
        else if (numActiveOutputs == 1)
        {
            validateRequire(getType() == activeOutputs[0]->getType(), res, message, "Node type does not match output port type");
        }
    }
    else
    {
        bool categoryDeclared = !getDocument()->getMatchingNodeDefs(getCategory()).empty();
        validateRequire(!categoryDeclared, res, message, "Node interface doesn't support this output type");
    }

    return InterfaceElement::validate(message) && res;
}

//
// GraphElement methods
//

NodePtr GraphElement::addMaterialNode(const string& name, ConstNodePtr shaderNode)
{
    bool isVolumeShader = shaderNode && shaderNode->getType() == VOLUME_SHADER_TYPE_STRING;
    string category = isVolumeShader ? VOLUME_MATERIAL_NODE_STRING : SURFACE_MATERIAL_NODE_STRING;
    NodePtr materialNode = addNode(category, name, MATERIAL_TYPE_STRING);
    if (shaderNode)
    {
        InputPtr input = materialNode->addInput(shaderNode->getType(), shaderNode->getType());
        input->setConnectedNode(shaderNode);
    }
    return materialNode;
}

void GraphElement::flattenSubgraphs(const string& target, NodePredicate filter)
{
    vector<NodePtr> nodeQueue = getNodes();
    while (!nodeQueue.empty())
    {
        // Determine which nodes require processing, and precompute declarations
        // and graph implementations for these nodes.
        using PortElementVec = vector<PortElementPtr>;
        std::vector<NodePtr> processNodeVec;
        std::unordered_map<NodePtr, NodeGraphPtr> graphImplMap;
        std::unordered_map<NodePtr, ConstInterfaceElementPtr> declarationMap;
        std::unordered_map<NodePtr, PortElementVec> downstreamPortMap;
        for (NodePtr node : nodeQueue)
        {
            if (filter && !filter(node))
            {
                continue;
            }

            InterfaceElementPtr implement = node->getImplementation(target);
            if (implement && implement->isA<NodeGraph>())
            {
                processNodeVec.push_back(node);
                graphImplMap[node] = implement->asA<NodeGraph>();
                declarationMap[node] = node->getDeclaration(target);
                downstreamPortMap[node] = node->getDownstreamPorts();
                for (NodePtr sourceSubNode : implement->asA<NodeGraph>()->getNodes())
                {
                    downstreamPortMap[sourceSubNode] = sourceSubNode->getDownstreamPorts();
                }
            }
        }
        nodeQueue.clear();

        // Iterate through nodes with graph implementations.
        for (NodePtr processNode : processNodeVec)
        {
            NodeGraphPtr sourceSubGraph = graphImplMap[processNode];
            std::unordered_map<NodePtr, NodePtr> subNodeMap;

            // Create a new instance of each original subnode.
            for (NodePtr sourceSubNode : sourceSubGraph->getNodes())
            {
                string origName = sourceSubNode->getName();
                string destName = createValidChildName(origName);
                NodePtr destSubNode = addNode(sourceSubNode->getCategory(), destName);

                destSubNode->copyContentFrom(sourceSubNode);
                setChildIndex(destSubNode->getName(), getChildIndex(processNode->getName()));

                // Store the mapping between subgraphs.
                subNodeMap[sourceSubNode] = destSubNode;

                // Add the subnode to the queue, allowing processing of nested subgraphs.
                nodeQueue.push_back(destSubNode);
            }

            // Update properties of generated subnodes.
            for (const auto& subNodePair : subNodeMap)
            {
                NodePtr sourceSubNode = subNodePair.first;
                NodePtr destSubNode = subNodePair.second;

                // Update node connections.
                for (PortElementPtr sourcePort : downstreamPortMap[sourceSubNode])
                {
                    if (sourcePort->isA<Input>())
                    {
                        auto it = subNodeMap.find(sourcePort->getParent()->asA<Node>());
                        if (it != subNodeMap.end())
                        {
                            InputPtr processNodeInput = it->second->getInput(sourcePort->getName());
                            if (processNodeInput)
                            {
                                processNodeInput->setNodeName(destSubNode->getName());
                            }
                        }
                    }
                    else if (sourcePort->isA<Output>())
                    {
                        for (PortElementPtr processNodePort : downstreamPortMap[processNode])
                        {
                            processNodePort->setNodeName(destSubNode->getName());
                        }
                    }
                }

                // Transfer interface properties.
                for (InputPtr destInput : destSubNode->getInputs())
                {
                    if (destInput->hasInterfaceName())
                    {
                        InputPtr sourceInput = processNode->getInput(destInput->getInterfaceName());
                        if (sourceInput)
                        {
                            destInput->copyContentFrom(sourceInput);
                            NodePtr connectedNode = destInput->getConnectedNode();
                            // Update downstream port map with the new instance
                            if (connectedNode && downstreamPortMap.count(connectedNode) > 0)
                            {
                                downstreamPortMap[connectedNode] = connectedNode->getDownstreamPorts();
                            }
                        }
                        else
                        {
                            ConstInterfaceElementPtr declaration = declarationMap[processNode];
                            InputPtr declInput = declaration ? declaration->getActiveInput(destInput->getInterfaceName()) : nullptr;
                            if (declInput)
                            {
                                if (declInput->hasValueString())
                                {
                                    destInput->setValueString(declInput->getValueString());
                                }
                                if (declInput->hasDefaultGeomPropString())
                                {
                                    ConstGeomPropDefPtr geomPropDef = getDocument()->getGeomPropDef(declInput->getDefaultGeomPropString());
                                    if (geomPropDef)
                                    {
                                        destInput->setConnectedNode(addGeomNode(geomPropDef, "geomNode"));
                                    }
                                }
                            }
                            destInput->removeAttribute(ValueElement::INTERFACE_NAME_ATTRIBUTE);
                        }
                    }
                }
            }

            // Update downstream ports with connections to subgraph outputs.
            for (PortElementPtr downstreamPort : downstreamPortMap[processNode])
            {
                if (downstreamPort->hasOutputString())
                {
                    OutputPtr subGraphOutput = sourceSubGraph->getOutput(downstreamPort->getOutputString());
                    if (subGraphOutput)
                    {
                        string destName = subGraphOutput->getNodeName();
                        NodePtr sourceSubNode = sourceSubGraph->getNode(destName);
                        NodePtr destNode = sourceSubNode ? subNodeMap[sourceSubNode] : nullptr;
                        if (destNode)
                        {
                            destName = destNode->getName();
                        }
                        downstreamPort->setNodeName(destName);
                        downstreamPort->setOutputString(EMPTY_STRING);
                    }
                }
            }

            // The processed node has been replaced, so remove it from the graph.
            removeNode(processNode->getName());
        }
    }
}

NodeGraphPtr GraphElement::createNodeGraphFromNodes(const vector<NodePtr>& nodes, const string& name)
{
    // Filter to valid direct-child nodes and collect their names.
    std::unordered_set<string> selectedNodeNames;
    vector<NodePtr> validNodes;
    for (const NodePtr& node : nodes)
    {
        if (node && node->getParent() == getSelf() && selectedNodeNames.insert(node->getName()).second)
        {
            validNodes.push_back(node);
        }
    }
    if (validNodes.empty())
    {
        return nullptr;
    }

    // Create the nodegraph and copy the selected nodes into it.
    NodeGraphPtr nodeGraph = addChild<NodeGraph>(name);
    for (const NodePtr& sourceNode : validNodes)
    {
        NodePtr copiedNode = nodeGraph->addNode(sourceNode->getCategory(), sourceNode->getName(), sourceNode->getType());
        copiedNode->copyContentFrom(sourceNode);
    }

    // Preserve boundary connections. Downstream preservation reads the original
    // nodes' downstream ports, so it must run before the originals are removed.
    preserveExternalUpstreamConnections(nodeGraph, selectedNodeNames);
    preserveExternalDownstreamConnections(nodeGraph, validNodes, selectedNodeNames);

    // Remove the original nodes from this graph element.
    for (const string& nodeName : selectedNodeNames)
    {
        removeChild(nodeName);
    }

    return nodeGraph;
}

ElementVec GraphElement::topologicalSort() const
{
    // Calculate a topological order of the children, using Kahn's algorithm
    // to avoid recursion.
    //
    // Running time: O(numNodes + numEdges).

    const ElementVec& children = getChildren();

    // Calculate in-degrees for all children.
    std::unordered_map<ElementPtr, size_t> inDegree(children.size());
    std::deque<ElementPtr> childQueue;
    for (ElementPtr child : children)
    {
        size_t connectionCount = 0;
        for (size_t i = 0; i < child->getUpstreamEdgeCount(); ++i)
        {
            Edge upstreamEdge = child->getUpstreamEdge(i);
            if (upstreamEdge)
            {
                if (upstreamEdge.getUpstreamElement())
                {
                    ElementPtr elem = upstreamEdge.getUpstreamElement()->getParent();
                    if (elem == child->getParent())
                    {
                        connectionCount++;
                    }
                }
            }
        }

        inDegree[child] = connectionCount;

        // Enqueue children with in-degree 0.
        if (connectionCount == 0)
        {
            childQueue.push_back(child);
        }
    }

    ElementVec result;
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
    }

    return result;
}

NodePtr GraphElement::addGeomNode(ConstGeomPropDefPtr geomPropDef, const string& namePrefix)
{
    string geomNodeName = namePrefix + "_" + geomPropDef->getName();
    NodePtr geomNode = getNode(geomNodeName);
    if (!geomNode)
    {
        geomNode = addNode(geomPropDef->getGeomProp(), geomNodeName, geomPropDef->getType());
        if (geomPropDef->hasAttribute(GeomPropDef::SPACE_ATTRIBUTE))
        {
            geomNode->setInputValue(GeomPropDef::SPACE_ATTRIBUTE, geomPropDef->getAttribute(GeomPropDef::SPACE_ATTRIBUTE));
        }
        if (geomPropDef->hasAttribute(GeomPropDef::INDEX_ATTRIBUTE))
        {
            geomNode->setInputValue(GeomPropDef::INDEX_ATTRIBUTE, geomPropDef->getAttribute(GeomPropDef::INDEX_ATTRIBUTE), getTypeString<int>());
        }
    }
    return geomNode;
}

string GraphElement::asStringDot() const
{
    string dot = "digraph {\n";

    // Create a unique name for each child element.
    ElementVec children = topologicalSort();
    StringMap nameMap;
    StringSet nameSet;
    for (ElementPtr elem : children)
    {
        string uniqueName = elem->getCategory();
        while (nameSet.count(uniqueName))
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

void NodeGraph::setNameGlobal(const string& name)
{
    vector<PortElementPtr> downStreamPorts = getDownstreamPorts();
    setName(name);
    const string& newName = getName();
    for (PortElementPtr& port : downStreamPorts)
    {
        port->setNodeGraphString(newName);
    }
}

vector<OutputPtr> NodeGraph::getMaterialOutputs() const
{
    vector<OutputPtr> materialOutputs;
    for (auto graphOutput : getActiveOutputs())
    {
        if (graphOutput->getType() == MATERIAL_TYPE_STRING)
        {
            NodePtr node = graphOutput->getConnectedNode();
            if (node && node->getType() == MATERIAL_TYPE_STRING)
            {
                materialOutputs.push_back(graphOutput);
            }
        }
    }
    return materialOutputs;
}

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

InputPtr Node::addInputFromNodeDef(const string& inputName)
{
    InputPtr nodeInput = getInput(inputName);
    if (!nodeInput)
    {
        NodeDefPtr nodeDef = getNodeDef();
        InputPtr nodeDefInput = nodeDef ? nodeDef->getActiveInput(inputName) : nullptr;
        if (nodeDefInput)
        {
            nodeInput = addInput(nodeDefInput->getName(), nodeDefInput->getType());
            if (nodeDefInput->hasValueString())
            {
                nodeInput->setValueString(nodeDefInput->getValueString());
            }
        }
    }
    return nodeInput;
}

void Node::addInputsFromNodeDef()
{
    NodeDefPtr nodeDef = getNodeDef();
    if (nodeDef)
    {
        for (InputPtr nodeDefInput : nodeDef->getActiveInputs())
        {
            const string& inputName = nodeDefInput->getName();
            InputPtr nodeInput = getInput(inputName);
            if (!nodeInput)
            {
                nodeInput = addInput(inputName, nodeDefInput->getType());
                if (nodeDefInput->hasValueString())
                {
                    nodeInput->setValueString(nodeDefInput->getValueString());
                }
            }
        }
    }
}

InputPtr NodeGraph::addInterfaceName(const string& inputPath, const string& interfaceName)
{
    NodeDefPtr nodeDef = getNodeDef();
    InterfaceElementPtr interfaceElement = nodeDef ? nodeDef->asA<InterfaceElement>() : getSelf()->asA<InterfaceElement>();
    if (interfaceElement->getChild(interfaceName))
    {
        throw Exception("Interface: " + interfaceName + " has already been declared on the interface: " + interfaceElement->getNamePath());
    }

    InputPtr interfaceInput;
    ElementPtr elem = getDescendant(inputPath);
    InputPtr input = elem ? elem->asA<Input>() : nullptr;
    if (input && !input->getConnectedNode())
    {
        input->setInterfaceName(interfaceName);
        interfaceInput = interfaceElement->getInput(interfaceName);
        if (!interfaceInput)
        {
            interfaceInput = interfaceElement->addInput(interfaceName, input->getType());
        }
        if (input->hasValue())
        {
            interfaceInput->setValueString(input->getValueString());
            input->removeAttribute(Input::VALUE_ATTRIBUTE);
        }
    }
    return interfaceInput;
}

void NodeGraph::removeInterfaceName(const string& inputPath)
{
    ElementPtr desc = getDescendant(inputPath);
    InputPtr input = desc ? desc->asA<Input>() : nullptr;
    if (input)
    {
        const string& interfaceName = input->getInterfaceName();
        if (!interfaceName.empty())
        {
            NodeDefPtr nodeDef = getNodeDef();
            InterfaceElementPtr interface = nodeDef ? nodeDef->asA<InterfaceElement>() : getSelf()->asA<InterfaceElement>();
            ElementPtr interfacePort = interface->getChild(interfaceName);
            if (interfacePort)
            {
                InputPtr interfaceInput = interfacePort->asA<Input>();
                if (interfaceInput && interfaceInput->hasValue())
                {
                    input->setValueString(interfaceInput->getValueString());
                }
                interface->removeChild(interfaceName);
            }
            input->setInterfaceName(EMPTY_STRING);
        }
    }
}

void NodeGraph::modifyInterfaceName(const string& inputPath, const string& interfaceName)
{
    NodeDefPtr nodeDef = getNodeDef();
    InterfaceElementPtr interfaceElement = nodeDef ? nodeDef->asA<InterfaceElement>() : getSelf()->asA<InterfaceElement>();

    ElementPtr desc = getDescendant(inputPath);
    InputPtr input = desc ? desc->asA<Input>() : nullptr;
    if (input)
    {
        const string& previousName = input->getInterfaceName();
        if (previousName != interfaceName)
        {
            ElementPtr previousChild = interfaceElement->getChild(previousName);
            if (previousChild)
            {
                previousChild->setName(interfaceName);
            }
            input->setInterfaceName(interfaceName);
        }
    }
}

NodeDefPtr NodeGraph::getNodeDef() const
{
    NodeDefPtr nodedef = resolveNameReference<NodeDef>(getNodeDefString());
    // If not directly defined look for an implementation which has a nodedef association
    if (!nodedef)
    {
        for (auto impl : getDocument()->getImplementations())
        {
            if (impl->getNodeGraph() == getQualifiedName(getName()))
            {
                nodedef = impl->getNodeDef();
            }
        }
    }
    return nodedef;
}

InterfaceElementPtr NodeGraph::getImplementation() const
{
    NodeDefPtr nodedef = getNodeDef();
    return nodedef ? nodedef->getImplementation() : InterfaceElementPtr();
}

vector<PortElementPtr> NodeGraph::getDownstreamPorts() const
{
    vector<PortElementPtr> downstreamPorts;
    for (PortElementPtr port : getDocument()->getMatchingPorts(getQualifiedName(getName())))
    {
        ElementPtr node = port->getParent();
        ElementPtr graph = node ? node->getParent() : nullptr;
        if (graph && graph->isA<GraphElement>() && graph == getParent())
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

bool NodeGraph::validate(string* message) const
{
    bool res = true;

    validateRequire(!hasVersionString(), res, message, "NodeGraph elements do not support version strings");
    if (hasNodeDefString())
    {
        NodeDefPtr nodeDef = getNodeDef();
        validateRequire(nodeDef != nullptr, res, message, "NodeGraph implementation refers to non-existent NodeDef");
        if (nodeDef)
        {
            vector<OutputPtr> graphOutputs = getOutputs();
            vector<OutputPtr> nodeDefOutputs = nodeDef->getActiveOutputs();
            validateRequire(graphOutputs.size() == nodeDefOutputs.size(), res, message, "NodeGraph implementation has a different number of outputs than its NodeDef");
            if (graphOutputs.size() == 1 && nodeDefOutputs.size() == 1)
            {
                validateRequire(graphOutputs[0]->getType() == nodeDefOutputs[0]->getType(), res, message, "NodeGraph implementation has a different output type than its NodeDef");
            }
        }
    }

    return GraphElement::validate(message) && res;
}

ConstInterfaceElementPtr NodeGraph::getDeclaration(const string&) const
{
    ConstNodeDefPtr nodeDef = getNodeDef();
    if (nodeDef)
    {
        return nodeDef;
    }
    if (!hasNodeDefString())
    {
        return getSelf()->asA<InterfaceElement>();
    }
    return nullptr;
}

//
// Backdrop methods
//

void Backdrop::setContainsElements(const vector<ConstTypedElementPtr>& elems)
{
    if (!elems.empty())
    {
        StringVec stringVec;
        for (ConstTypedElementPtr elem : elems)
        {
            stringVec.push_back(elem->getName());
        }
        setTypedAttribute(CONTAINS_ATTRIBUTE, stringVec);
    }
    else
    {
        removeAttribute(CONTAINS_ATTRIBUTE);
    }
}

vector<TypedElementPtr> Backdrop::getContainsElements() const
{
    vector<TypedElementPtr> vec;
    ConstGraphElementPtr graph = getAncestorOfType<GraphElement>();
    if (graph)
    {
        for (const string& str : getTypedAttribute<StringVec>(CONTAINS_ATTRIBUTE))
        {
            TypedElementPtr elem = graph->getChildOfType<TypedElement>(str);
            if (elem)
            {
                vec.push_back(elem);
            }
        }
    }
    return vec;
}

bool Backdrop::validate(string* message) const
{
    bool res = true;
    if (hasContainsString())
    {
        StringVec stringVec = getTypedAttribute<StringVec>("contains");
        vector<TypedElementPtr> elemVec = getContainsElements();
        validateRequire(stringVec.size() == elemVec.size(), res, message, "Invalid element in contains string");
    }
    return Element::validate(message) && res;
}

MATERIALX_NAMESPACE_END
