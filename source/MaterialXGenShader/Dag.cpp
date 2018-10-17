#include <MaterialXGenShader/Dag.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenImplementation.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>

#include <MaterialXFormat/File.h>

#include <iostream>
#include <sstream>
#include <stack>

namespace MaterialX
{

Dag::Dag(const string& name, DocumentPtr document)
    : DagNode(name)
    , _document(document)
{
}

void Dag::addInputSockets(const InterfaceElement& elem)
{
    for (ValueElementPtr port : elem.getChildrenOfType<ValueElement>())
    {
        if (!port->isA<Output>())
        {
            DagInputSocket* inputSocket = addInputSocket(port->getName(), TypeDesc::get(port->getType()));
            if (!port->getValueString().empty())
            {
                inputSocket->value = port->getValue();
            }
        }
    }
}

void Dag::addOutputSockets(const InterfaceElement& elem)
{
    for (const OutputPtr& output : elem.getOutputs())
    {
        addOutputSocket(output->getName(), TypeDesc::get(output->getType()));
    }
    if (numOutputSockets() == 0)
    {
        addOutputSocket("out", TypeDesc::get(elem.getType()));
    }
}

void Dag::addUpstreamDependencies(const Element& root, ConstMaterialPtr material, ShaderGenerator& shadergen)
{
    // Keep track of our root node in the graph.
    // This is needed when the graph is a shader graph and we need 
    // to make connections for BindInputs during traversal below.
    DagNode* rootNode = getNode(root.getName());

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
        DagNode* newNode = getNode(newNodeName);
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
            DagInput* input = rootNode->getInput(connectingElement->getName());
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
                DagNode* downstream = getNode(downstreamNode->getName());
                if (downstream && connectingElement)
                {
                    DagInput* input = downstream->getInput(connectingElement->getName());
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
                DagOutputSocket* outputSocket = getOutputSocket(downstreamElement->getName());
                if (outputSocket)
                {
                    outputSocket->makeConnection(newNode->getOutput());
                }
            }
        }
    }
}

void Dag::addDefaultGeomNode(DagInput* input, const GeomProp& geomprop, ShaderGenerator& shadergen)
{
    const string geomNodeName = "default_" + geomprop.getName();
    DagNode* node = getNode(geomNodeName);

    if (!node)
    {
        // Find the nodedef for the geometric node referenced by the geomprop. Use the type of the 
        // input here and ignore the type of the geomprop. They are required to have the same type.
        string geomNodeDefName = "ND_" + geomprop.getName() + "_" + input->type->getName();
        NodeDefPtr geomNodeDef = _document->getNodeDef(geomNodeDefName);
        if (!geomNodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef named '" + geomNodeDefName +
                "' for geomprop on input '" + input->node->getName() + "." + input->name + "'");
        }

        DagNodePtr geomNodePtr = DagNode::create(geomNodeName, *geomNodeDef, shadergen);
        _nodeMap[geomNodeName] = geomNodePtr;
        _nodeOrder.push_back(geomNodePtr.get());

        // Set node inputs if given.
        const string& space = geomprop.getSpace();
        if (!space.empty())
        {
            DagInput* spaceInput = geomNodePtr->getInput("space");
            if (spaceInput)
            {
                spaceInput->value = Value::createValue<string>(space);
            }
        }
        const string& index = geomprop.getIndex();
        if (!index.empty())
        {
            DagInput* indexInput = geomNodePtr->getInput("index");
            if (indexInput)
            {
                indexInput->value = Value::createValue<string>(index);
            }
        }
        const string& attrname = geomprop.getAttrName();
        if (!attrname.empty())
        {
            DagInput* attrnameInput = geomNodePtr->getInput("attrname");
            if (attrnameInput)
            {
                attrnameInput->value = Value::createValue<string>(attrname);
            }
        }

        node = geomNodePtr.get();
    }

    input->makeConnection(node->getOutput());
}

void Dag::addColorTransformNode(DagOutput* output, const string& colorTransform, ShaderGenerator& shadergen)
{
    const string nodeDefName = "ND_" + colorTransform + "_" + output->type->getName();
    NodeDefPtr nodeDef = _document->getNodeDef(nodeDefName);
    if (!nodeDef)
    {
        // Color transformations are by design not defined for all data types, only for color types.
        // So if a nodedef for the given output type is not found we just ignore this transform.
        return;
    }

    const string nodeName = output->node->getName() + "_" + colorTransform;
    DagNodePtr nodePtr = DagNode::create(nodeName, *nodeDef, shadergen);
    _nodeMap[nodeName] = nodePtr;
    _nodeOrder.push_back(nodePtr.get());

    DagNode* node = nodePtr.get();
    DagOutput* nodeOutput = node->getOutput(0);

    // Connect the node to the downstream inputs
    // Iterate a copy of the connection set since the original
    // set will change when breaking the old connections
    DagInputSet downstreamConnections = output->connections;
    for (DagInput* downstreamInput : downstreamConnections)
    {
        downstreamInput->breakConnection();
        downstreamInput->makeConnection(nodeOutput);
    }

    // Connect the node to the upstream output
    DagInput* nodeInput = node->getInput(0);
    nodeInput->makeConnection(output);
}

DagPtr Dag::create(NodeGraphPtr nodeGraph, ShaderGenerator& shadergen)
{
    NodeDefPtr nodeDef = nodeGraph->getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Can't find nodedef '" + nodeGraph->getNodeDefString() + "' referenced by nodegraph '" + nodeGraph->getName() + "'");
    }

    DagPtr graph = std::make_shared<Dag>(nodeGraph->getName(), nodeGraph->getDocument());

    // Clear classification
    graph->_classification = 0;

    // Create input sockets from the nodedef
    graph->addInputSockets(*nodeDef);

    // Create output sockets from the nodegraph
    graph->addOutputSockets(*nodeGraph);

    // Traverse all outputs and create all upstream dependencies
    for (OutputPtr graphOutput : nodeGraph->getOutputs())
    {
        graph->addUpstreamDependencies(*graphOutput, nullptr, shadergen);
    }

    // Add classification according to last node
    // TODO: What if the graph has multiple outputs?
    {
        DagOutputSocket* outputSocket = graph->getOutputSocket();
        graph->_classification |= outputSocket->connection ? outputSocket->connection->node->_classification : 0;
    }

    graph->finalize(shadergen);

    return graph;
}

DagPtr Dag::create(const string& name, ElementPtr element, ShaderGenerator& shadergen)
{
    DagPtr graph;
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

        graph = std::make_shared<Dag>(name, element->getDocument());

        // Clear classification
        graph->_classification = 0;

        // Create input sockets
        graph->addInputSockets(*interface);

        // Create the given output socket
        graph->addOutputSocket(output->getName(), TypeDesc::get(output->getType()));

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

        graph = std::make_shared<Dag>(name, element->getDocument());

        // Create input sockets
        graph->addInputSockets(*nodeDef);

        // Create output sockets
        graph->addOutputSockets(*nodeDef);

        // Create this shader node in the graph.
        const string& newNodeName = shaderRef->getName();
        DagNodePtr newNode = DagNode::create(newNodeName, *nodeDef, shadergen, nullptr);
        graph->_nodeMap[newNodeName] = newNode;
        graph->_nodeOrder.push_back(newNode.get());

        // Connect it to the graph output
        DagOutputSocket* outputSocket = graph->getOutputSocket();
        outputSocket->makeConnection(newNode->getOutput());

        // Handle node parameters
        for (ParameterPtr elem : nodeDef->getParameters())
        {
            DagInputSocket* inputSocket = graph->getInputSocket(elem->getName());
            DagInput* input = newNode->getInput(elem->getName());
            if (!inputSocket || !input)
            {
                throw ExceptionShaderGenError("Shader parameter '" + elem->getName() + "' doesn't match an existing input on graph '" + graph->getName() + "'");
            }

            BindParamPtr bindParam = shaderRef->getBindParam(elem->getName());
            if (bindParam)
            {
                // Copy value from binding
                if (!bindParam->getValueString().empty())
                {
                    inputSocket->value = bindParam->getValue();
                }
            }

            // Connect to the graph input
            inputSocket->makeConnection(input);
        }

        // Handle node inputs
        for (const InputPtr& nodeDefInput : nodeDef->getInputs())
        {
            DagInputSocket* inputSocket = graph->getInputSocket(nodeDefInput->getName());
            DagInput* input = newNode->getInput(nodeDefInput->getName());
            if (!inputSocket || !input)
            {
                throw ExceptionShaderGenError("Shader input '" + nodeDefInput->getName() + "' doesn't match an existing input on graph '" + graph->getName() + "'");
            }

            BindInputPtr bindInput = shaderRef->getBindInput(nodeDefInput->getName());

            if (bindInput)
            {
                // Copy value from binding
                if (!bindInput->getValueString().empty())
                {
                    inputSocket->value = bindInput->getValue();
                }
            }

            // If no explicit connection, connect to geometric node if geomprop is used
            // or otherwise to the graph interface.
            const string& connection = bindInput ? bindInput->getOutputString() : EMPTY_STRING;
            if (connection.empty())
            {
                GeomPropPtr geomprop = nodeDefInput->getGeomProp();
                if (geomprop)
                {
                    graph->addDefaultGeomNode(input, *geomprop, shadergen);
                }
                else
                {
                    inputSocket->makeConnection(input);
                }
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

    // Add classification according to root node
    DagOutputSocket* outputSocket = graph->getOutputSocket();
    graph->_classification |= outputSocket->connection ? outputSocket->connection->node->_classification : 0;

    graph->finalize(shadergen);

    return graph;
}

DagNode* Dag::addNode(const Node& node, ShaderGenerator& shadergen)
{
    NodeDefPtr nodeDef = node.getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Could not find a nodedef for node '" + node.getName() + "'");
    }
    
    // Create this node in the graph.
    const string& name = node.getName();
    DagNodePtr newNode = DagNode::create(name, *nodeDef, shadergen, &node);
    _nodeMap[name] = newNode;
    _nodeOrder.push_back(newNode.get());

    // Check if the node is a convotion. If so mark that the graph has a convolution
    if (newNode->hasClassification(Classification::CONVOLUTION2D))
    {
        _classification |= Classification::CONVOLUTION2D;
    }

    // Check if any of the node inputs should be connected to the graph interface
    for (ValueElementPtr elem : node.getChildrenOfType<ValueElement>())
    {
        const string& interfaceName = elem->getInterfaceName();
        if (!interfaceName.empty())
        {
            DagInputSocket* inputSocket = getInputSocket(interfaceName);
            if (!inputSocket)
            {
                throw ExceptionShaderGenError("Interface name '" + interfaceName + "' doesn't match an existing input on nodegraph '" + getName() + "'");
            }
            DagInput* input = newNode->getInput(elem->getName());
            if (input)
            {
                input->makeConnection(inputSocket);
            }
        }
    }

    // Handle the "geomprop" directives on the nodedef inputs.
    // Create and connect default geometric nodes on unconnected inputs.
    for (const InputPtr& nodeDefInput : nodeDef->getInputs())
    {
        DagInput* input = newNode->getInput(nodeDefInput->getName());
        InputPtr nodeInput = node.getInput(nodeDefInput->getName());

        const string& connection = nodeInput ? nodeInput->getNodeName() : EMPTY_STRING;
        if (connection.empty() && !input->connection)
        {
            GeomPropPtr geomprop = nodeDefInput->getGeomProp();
            if (geomprop)
            {
                addDefaultGeomNode(input, *geomprop, shadergen);
            }
        }
    }

    // Check if this is a file texture node that requires color transformation.
    if (newNode->hasClassification(DagNode::Classification::FILETEXTURE))
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

DagInputSocket* Dag::addInputSocket(const string& name, const TypeDesc* type)
{
    return DagNode::addOutput(name, type);
}

DagOutputSocket* Dag::addOutputSocket(const string& name, const TypeDesc* type)
{
    return DagNode::addInput(name, type);
}

void Dag::renameInputSocket(const string& name, const string& newName)
{
    return DagNode::renameOutput(name, newName);
}

void Dag::renameOutputSocket(const string& name, const string& newName)
{
    return DagNode::renameInput(name, newName);
}

DagNode* Dag::getNode(const string& name)
{
    auto it = _nodeMap.find(name);
    return it != _nodeMap.end() ? it->second.get() : nullptr;
}

void Dag::finalize(ShaderGenerator& shadergen)
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
    for (DagNode* node : _nodeOrder)
    {
        if (node->hasClassification(DagNode::Classification::SHADER))
        {
            for (DagEdge edge : node->getOutput()->traverseUpstream())
            {
                if (edge.upstream)
                {
                    if (edge.upstream->node->hasClassification(DagNode::Classification::CLOSURE))
                    {
                        node->_usedClosures.insert(edge.upstream->node);
                    }
                }
            }
        }
    }
}

void Dag::disconnect(DagNode* node)
{
    for (DagInput* input : node->getInputs())
    {
        input->breakConnection();
    }
    for (DagOutput* output : node->getOutputs())
    {
        output->breakConnection();
    }
}

void Dag::optimize()
{
    size_t numEdits = 0;
    for (DagNode* node : getNodes())
    {
        if (node->hasClassification(DagNode::Classification::CONSTANT))
        {
            // Constant nodes can be removed by assigning their value downstream
            // But don't remove it if it's connected upstream, i.e. it's value 
            // input is published.
            DagInput* valueInput = node->getInput(0);
            if (!valueInput->connection)
            {
                bypass(node, 0);
                ++numEdits;
            }
        }
        else if (node->hasClassification(DagNode::Classification::IFELSE))
        {
            // Check if we have a constant conditional expression
            DagInput* intest = node->getInput("intest");
            if (!intest->connection || intest->connection->node->hasClassification(DagNode::Classification::CONSTANT))
            {
                // Find which branch should be taken
                DagInput* cutoff = node->getInput("cutoff");
                ValuePtr value = intest->connection ? intest->connection->node->getInput(0)->value : intest->value;
                const float intestValue = value ? value->asA<float>() : 0.0f;
                const int branch = (intestValue <= cutoff->value->asA<float>() ? 2 : 3);

                // Bypass the conditional using the taken branch
                bypass(node, branch);

                ++numEdits;
            }
        }
        else if (node->hasClassification(DagNode::Classification::SWITCH))
        {
            // Check if we have a constant conditional expression
            DagInput* which = node->getInput("which");
            if (!which->connection || which->connection->node->hasClassification(DagNode::Classification::CONSTANT))
            {
                // Find which branch should be taken
                ValuePtr value = which->connection ? which->connection->node->getInput(0)->value : which->value;
                const int branch = int(value==nullptr ? 0 :
                    (which->type == Type::BOOLEAN ? value->asA<bool>() :
                    (which->type == Type::FLOAT ? value->asA<float>() : value->asA<int>())));

                // Bypass the conditional using the taken branch
                bypass(node, branch);

                ++numEdits;
            }
        }
    }

    if (numEdits > 0)
    {
        std::set<DagNode*> usedNodes;

        // Travers the graph to find nodes still in use
        for (DagOutputSocket* outputSocket : getOutputSockets())
        {
            if (outputSocket->connection)
            {
                for (DagEdge edge : outputSocket->connection->traverseUpstream())
                {
                    usedNodes.insert(edge.upstream->node);
                }
            }
        }

        // Remove any unused nodes
        for (DagNode* node : _nodeOrder)
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

void Dag::bypass(DagNode* node, size_t inputIndex, size_t outputIndex)
{
    DagInput* input = node->getInput(inputIndex);
    DagOutput* output = node->getOutput(outputIndex);

    DagOutput* upstream = input->connection;
    if (upstream)
    {
        // Re-route the upstream output to the downstream inputs.
        // Iterate a copy of the connection set since the
        // original set will change when breaking connections.
        DagInputSet downstreamConnections = output->connections;
        for (DagInput* downstream : downstreamConnections)
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
        DagInputSet downstreamConnections = output->connections;
        for (DagInput* downstream : downstreamConnections)
        {
            output->breakConnection(downstream);
            downstream->value = input->value;
        }
    }
}

void Dag::topologicalSort()
{
    // Calculate a topological order of the children, using Kahn's algorithm
    // to avoid recursion.
    //
    // Running time: O(numNodes + numEdges).

    // Calculate in-degrees for all nodes, and enqueue those with degree 0.
    std::unordered_map<DagNode*, int> inDegree(_nodeMap.size());
    std::deque<DagNode*> nodeQueue;
    for (auto it : _nodeMap)
    {
        DagNode* node = it.second.get();

        int connectionCount = 0;
        for (const DagInput* input : node->getInputs())
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
        DagNode* node = nodeQueue.front();
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

void Dag::calculateScopes()
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
    DagNode* lastNode = _nodeOrder[lastNodeIndex];
    lastNode->getScopeInfo().type = DagNode::ScopeInfo::Type::GLOBAL;

    std::set<DagNode*> nodeUsed;
    nodeUsed.insert(lastNode);

    // Iterate nodes in reversed toplogical order such that every node is visited AFTER 
    // each of the nodes that depend on it have been processed first.
    for (int nodeIndex = int(lastNodeIndex); nodeIndex >= 0; --nodeIndex)
    {
        DagNode* node = _nodeOrder[nodeIndex];

        // Once we visit a node the scopeInfo has been determined and it will not be changed
        // By then we have visited all the nodes that depend on it already
        if (nodeUsed.count(node) == 0)
        {
            continue;
        }

        const bool isIfElse = node->hasClassification(DagNode::Classification::IFELSE);
        const bool isSwitch = node->hasClassification(DagNode::Classification::SWITCH);

        const DagNode::ScopeInfo& currentScopeInfo = node->getScopeInfo();

        for (size_t inputIndex = 0; inputIndex < node->numInputs(); ++inputIndex)
        {
            DagInput* input = node->getInput(inputIndex);

            if (input->connection)
            {
                DagNode* upstreamNode = input->connection->node;

                // Create scope info for this network brach
                // If it's a conditonal branch the scope is adjusted
                DagNode::ScopeInfo newScopeInfo = currentScopeInfo;
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
                DagNode::ScopeInfo& upstreamScopeInfo = upstreamNode->getScopeInfo();
                upstreamScopeInfo.merge(newScopeInfo);

                nodeUsed.insert(upstreamNode);
            }
        }
    }
}

void Dag::validateNames(ShaderGenerator& shadergen)
{
    // Make sure inputs and outputs have names valid for the 
    // target shading language, and are unique to avoid name 
    // conflicts when emitting variable names for them.

    // Names in use for the graph is recorded in 'uniqueNames'.
    Syntax::UniqueNameMap uniqueNames;
    for (DagInputSocket* inputSocket : getInputSockets())
    {
        string name = inputSocket->name;
        shadergen.getSyntax()->makeUnique(name, uniqueNames);
        renameInputSocket(inputSocket->name, name);
    }
    for (DagOutputSocket* outputSocket : getOutputSockets())
    {
        string name = outputSocket->name;
        shadergen.getSyntax()->makeUnique(outputSocket->name, uniqueNames);
        renameOutputSocket(outputSocket->name, name);
    }
    for (DagNode* node : getNodes())
    {
        for (DagOutput* output : node->getOutputs())
        {
            // Node outputs use long names for better code readability
            string name = output->node->getName() + "_" + output->name;
            shadergen.getSyntax()->makeUnique(name, uniqueNames);
            node->renameOutput(output->name, name);
        }
    }
}

} // namespace MaterialX
