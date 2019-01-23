#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
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

ShaderGraph::ShaderGraph(const string& name, DocumentPtr document)
    : ShaderNode(name)
    , _document(document)
{
}

void ShaderGraph::addInputSockets(const InterfaceElement& elem, ShaderGenerator& shadergen)
{
    for (ValueElementPtr port : elem.getChildrenOfType<ValueElement>())
    {
        if (!port->isA<Output>())
        {
            ShaderGraphInputSocket* inputSocket = nullptr;
            const TypeDesc* enumerationType = nullptr;
            ValuePtr enumValue = shadergen.remapEnumeration(port, elem, enumerationType);
            if (enumerationType)
            {
                inputSocket = addInputSocket(port->getName(), enumerationType);
                if (enumValue)
                {
                    inputSocket->value = enumValue;
                }
            }
            else
            {
                const string& elemType = port->getType();
                inputSocket = addInputSocket(port->getName(), TypeDesc::get(elemType));
                if (!port->getValueString().empty())
                {
                    inputSocket->value = port->getValue();
                }
            }
        }
    }
}

void ShaderGraph::addOutputSockets(const InterfaceElement& elem)
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

void ShaderGraph::addUpstreamDependencies(const Element& root, ConstMaterialPtr material, ShaderGenerator& shadergen, const GenOptions& options)
{
    // Keep track of our root node in the graph.
    // This is needed when the graph is a shader graph and we need
    // to make connections for BindInputs during traversal below.
    ShaderNode* rootNode = getNode(root.getName());

    std::set<ElementPtr> processedOutputs;
    for (Edge edge : root.traverseGraph(material))
    {
        ElementPtr upstreamElement = edge.getUpstreamElement();
        if (!upstreamElement)
        {
            continue;
        }

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
        ShaderNode* newNode = getNode(newNodeName);
        if (!newNode)
        {
            newNode = addNode(*upstreamNode, shadergen, options);
        }

        //
        // Make connections
        //

        // Find the output to connect to
        OutputPtr nodeDefOutput = upstreamNode->getNodeDefOutput(edge);
        ShaderOutput* output = nodeDefOutput ? newNode->getOutput(nodeDefOutput->getName()) : newNode->getOutput();
        if (!output)
        {
            throw ExceptionShaderGenError("Could not find an output named '" + (nodeDefOutput ? nodeDefOutput->getName() : string("out")) +
                "' on upstream node '" + upstreamNode->getName() + "'");
        }

        // First check if this was a bind input connection
        // In this case we must have a root node as well
        ElementPtr connectingElement = edge.getConnectingElement();
        if (rootNode && connectingElement && connectingElement->isA<BindInput>())
        {
            // Connect to the corresponding input on the root node
            ShaderInput* input = rootNode->getInput(connectingElement->getName());
            if (input)
            {
                input->breakConnection();
                input->makeConnection(output);
            }
        }
        else
        {
            // Check if it was a node downstream
            NodePtr downstreamNode = downstreamElement->asA<Node>();
            if (downstreamNode)
            {
                // We have a node downstream
                ShaderNode* downstream = getNode(downstreamNode->getName());
                if (downstream && connectingElement)
                {
                    ShaderInput* input = downstream->getInput(connectingElement->getName());
                    if (!input)
                    {
                        throw ExceptionShaderGenError("Could not find an input named '" + connectingElement->getName() +
                            "' on downstream node '" + downstream->getName() + "'");
                    }
                    input->makeConnection(output);
                }
            }
            else
            {
                // Not a node, then it must be an output
                ShaderGraphOutputSocket* outputSocket = getOutputSocket(downstreamElement->getName());
                if (outputSocket)
                {
                    outputSocket->makeConnection(output);
                }
            }
        }
    }
}

void ShaderGraph::addDefaultGeomNode(ShaderInput* input, const GeomPropDef& geomprop, ShaderGenerator& shadergen, const GenOptions& options)
{
    const string geomNodeName = "geomprop_" + geomprop.getName();
    ShaderNode* node = getNode(geomNodeName);

    if (!node)
    {
        // Find the nodedef for the geometric node referenced by the geomprop. Use the type of the
        // input here and ignore the type of the geomprop. They are required to have the same type.
        string geomNodeDefName = "ND_" + geomprop.getNode() + "_" + input->type->getName();
        NodeDefPtr geomNodeDef = _document->getNodeDef(geomNodeDefName);
        if (!geomNodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef named '" + geomNodeDefName +
                "' for geomprop on input '" + input->node->getName() + "." + input->name + "'");
        }

        ShaderNodePtr geomNodePtr = ShaderNode::create(geomNodeName, *geomNodeDef, shadergen, options);
        _nodeMap[geomNodeName] = geomNodePtr;
        _nodeOrder.push_back(geomNodePtr.get());

        // Set node inputs if given.
        const string& namePath = geomprop.getNamePath();
        const string& space = geomprop.getSpace();
        if (!space.empty())
        {
            ShaderInput* spaceInput = geomNodePtr->getInput("space");
            if (spaceInput)
            {
                const TypeDesc* enumerationType = nullptr;
                const string& inputName("space");
                const string& inputType("string");
                ValuePtr value = shadergen.remapEnumeration(inputName, space, inputType, *geomNodeDef, enumerationType);
                if (value)
                {
                    spaceInput->value = value;
                }
                else
                {
                    spaceInput->value = Value::createValue<string>(space);
                }
                spaceInput->path = namePath;
            }
        }
        const string& index = geomprop.getIndex();
        if (!index.empty())
        {
            ShaderInput* indexInput = geomNodePtr->getInput("index");
            if (indexInput)
            {
                indexInput->value = Value::createValue<string>(index);
                indexInput->path = namePath;
            }
        }
        const string& attrname = geomprop.getAttrName();
        if (!attrname.empty())
        {
            ShaderInput* attrnameInput = geomNodePtr->getInput("attrname");
            if (attrnameInput)
            {
                attrnameInput->value = Value::createValue<string>(attrname);
                attrnameInput->path = namePath;
            }
        }

        node = geomNodePtr.get();
    }

    input->makeConnection(node->getOutput());
}

void ShaderGraph::addColorTransformNode(ShaderInput* input, const ColorSpaceTransform& transform, ShaderGenerator& shadergen, const GenOptions& options)
{
    ColorManagementSystemPtr colorManagementSystem = shadergen.getColorManagementSystem();
    if (!colorManagementSystem)
    {
        return;
    }
    string colorTransformNodeName = input->node->getName() + "_" + input->name + "_cm";
    ShaderNodePtr colorTransformNodePtr = colorManagementSystem->createNode(transform, colorTransformNodeName, shadergen, options);

    if (colorTransformNodePtr)
    {
        // We can potentially improve this workflow in the future by replacing the constant node with the transform node
        if (input->node->hasClassification(ShaderNode::Classification::CONSTANT))
        {
            input->node->_classification |= ShaderNode::Classification::DO_NOT_OPTIMIZE;
        }

        _nodeMap[colorTransformNodePtr->getName()] = colorTransformNodePtr;
        _nodeOrder.push_back(colorTransformNodePtr.get());

        ShaderNode* colorTransformNode = colorTransformNodePtr.get();
        ShaderOutput* colorTransformNodeOutput = colorTransformNode->getOutput(0);

        ShaderInput* shaderInput = colorTransformNode->getInput(0);
        shaderInput->variable = input->node->getName() + "_" + input->name;
        shaderInput->value = input->value;
        shaderInput->flags |= ShaderPort::VARIABLE_NOT_RENAMABLE;
        shaderInput->path = input->path;

        input->makeConnection(colorTransformNodeOutput);
    }
}

void ShaderGraph::addColorTransformNode(ShaderOutput* output, const ColorSpaceTransform& transform, ShaderGenerator& shadergen, const GenOptions& options)
{
    ColorManagementSystemPtr colorManagementSystem = shadergen.getColorManagementSystem();
    if (!colorManagementSystem)
    {
        return;
    }
    string colorTransformNodeName = output->node->getName() + "_" + output->name + "_cm";
    ShaderNodePtr colorTransformNodePtr = colorManagementSystem->createNode(transform, colorTransformNodeName, shadergen, options);

    if (colorTransformNodePtr)
    {
        _nodeMap[colorTransformNodePtr->getName()] = colorTransformNodePtr;
        _nodeOrder.push_back(colorTransformNodePtr.get());

        ShaderNode* colorTransformNode = colorTransformNodePtr.get();
        ShaderOutput* colorTransformNodeOutput = colorTransformNode->getOutput(0);

        std::vector<ShaderInput*> inputs(output->connections.begin(), output->connections.end());
        for (ShaderInput* input : inputs)
        {
            input->breakConnection();
            input->makeConnection(colorTransformNodeOutput);
        }

        // Connect the node to the upstream output
        ShaderInput* colorTransformNodeInput = colorTransformNode->getInput(0);
        colorTransformNodeInput->makeConnection(output);
    }
}

ShaderGraphPtr ShaderGraph::create(NodeGraphPtr nodeGraph, ShaderGenerator& shadergen, const GenOptions& options)
{
    NodeDefPtr nodeDef = nodeGraph->getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Can't find nodedef '" + nodeGraph->getNodeDefString() + "' referenced by nodegraph '" + nodeGraph->getName() + "'");
    }

    ShaderGraphPtr graph = std::make_shared<ShaderGraph>(nodeGraph->getName(), nodeGraph->getDocument());

    // Clear classification
    graph->_classification = 0;

    // Create input sockets from the nodedef
    graph->addInputSockets(*nodeDef, shadergen);

    // Create output sockets from the nodegraph
    graph->addOutputSockets(*nodeGraph);

    // Traverse all outputs and create all upstream dependencies
    for (OutputPtr graphOutput : nodeGraph->getOutputs())
    {
        graph->addUpstreamDependencies(*graphOutput, nullptr, shadergen, options);
    }

    // Add classification according to last node
    // TODO: What if the graph has multiple outputs?
    {
        ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket();
        graph->_classification |= outputSocket->connection ? outputSocket->connection->node->_classification : 0;
    }

    // Finalize the graph
    graph->finalize(shadergen, options);

    return graph;
}

ShaderGraphPtr ShaderGraph::create(const string& name, ElementPtr element, ShaderGenerator& shadergen, const GenOptions& options)
{
    ShaderGraphPtr graph;
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

        graph = std::make_shared<ShaderGraph>(name, element->getDocument());

        // Clear classification
        graph->_classification = 0;

        // Create input sockets
        graph->addInputSockets(*interface, shadergen);

        // Create the given output socket
        ShaderGraphOutputSocket* outputSocket = graph->addOutputSocket(output->getName(), TypeDesc::get(output->getType()));
        outputSocket->path = output->getNamePath();

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

        graph = std::make_shared<ShaderGraph>(name, element->getDocument());

        // Create input sockets
        graph->addInputSockets(*nodeDef, shadergen);

        // Create output sockets
        graph->addOutputSockets(*nodeDef);

        // Create this shader node in the graph.
        const string& newNodeName = shaderRef->getName();
        ShaderNodePtr newNode = ShaderNode::create(newNodeName, *nodeDef, shadergen, options);
        graph->_nodeMap[newNodeName] = newNode;
        graph->_nodeOrder.push_back(newNode.get());

        // Connect it to the graph output
        ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket();
        outputSocket->makeConnection(newNode->getOutput());

        // Handle node parameters
        for (ParameterPtr elem : nodeDef->getParameters())
        {
            ShaderGraphInputSocket* inputSocket = graph->getInputSocket(elem->getName());
            ShaderInput* input = newNode->getInput(elem->getName());
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
                inputSocket->path = bindParam->getNamePath();
                input->path = inputSocket->path;
            }

            // Connect to the graph input
            inputSocket->makeConnection(input);
        }

        // Handle node inputs
        for (const InputPtr& nodeDefInput : nodeDef->getInputs())
        {
            ShaderGraphInputSocket* inputSocket = graph->getInputSocket(nodeDefInput->getName());
            ShaderInput* input = newNode->getInput(nodeDefInput->getName());
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
                inputSocket->path = bindInput->getNamePath();
                input->path = inputSocket->path;
            }

            // If no explicit connection, connect to geometric node if a geomprop is used
            // or otherwise to the graph interface.
            const string& connection = bindInput ? bindInput->getOutputString() : EMPTY_STRING;
            if (connection.empty())
            {
                GeomPropDefPtr geomprop = nodeDefInput->getDefaultGeomProp();
                if (geomprop)
                {
                    graph->addDefaultGeomNode(input, *geomprop, shadergen, options);
                }
                else
                {
                    inputSocket->makeConnection(input);
                }
            }
        }

        // Add shareRef nodedef paths
        const vector<InputPtr> nodeInputs = nodeDef->getChildrenOfType<Input>();
        const string& nodePath = shaderRef->getNamePath();
        for (const ValueElementPtr& nodeInput : nodeInputs)
        {
            const string& inputName = nodeInput->getName();
            const string path = nodePath + NAME_PATH_SEPARATOR + inputName;
            ShaderInput* input = newNode->getInput(inputName);
            if (input && input->path.empty())
            {
                input->path = path;
            }
            ShaderGraphInputSocket* inputSocket = graph->getInputSocket(inputName);
            if (inputSocket && inputSocket->path.empty())
            {
                inputSocket->path = input->path;
            }
        }
        const vector<ParameterPtr> nodeParameters = nodeDef->getChildrenOfType<Parameter>();
        for (const ParameterPtr& nodeParameter : nodeParameters)
        {
            const string& paramName = nodeParameter->getName();
            const string path = nodePath + NAME_PATH_SEPARATOR + paramName;
            ShaderInput* input = newNode->getInput(paramName);
            if (input && input->path.empty())
            {
                input->path = path;
            }
            ShaderGraphInputSocket* inputSocket = graph->getInputSocket(paramName);
            if (inputSocket && inputSocket->path.empty())
            {
                inputSocket->path = input->path;
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
    graph->addUpstreamDependencies(*root, material, shadergen, options);

    // Add classification according to root node
    ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket();
    graph->_classification |= outputSocket->connection ? outputSocket->connection->node->_classification : 0;

    graph->finalize(shadergen, options);

    return graph;
}

ShaderNode* ShaderGraph::addNode(const Node& node, ShaderGenerator& shadergen, const GenOptions& options)
{
    NodeDefPtr nodeDef = node.getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Could not find a nodedef for node '" + node.getName() + "'");
    }

    // Create this node in the graph.
    const string& name = node.getName();
    ShaderNodePtr newNode = ShaderNode::create(name, *nodeDef, shadergen, options);
    newNode->setValues(node, *nodeDef, shadergen);
    newNode->setPaths(node, *nodeDef);
    _nodeMap[name] = newNode;
    _nodeOrder.push_back(newNode.get());

    // Check if the node is a convolution. If so mark that the graph has a convolution
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
            ShaderGraphInputSocket* inputSocket = getInputSocket(interfaceName);
            if (!inputSocket)
            {
                throw ExceptionShaderGenError("Interface name '" + interfaceName + "' doesn't match an existing input on nodegraph '" + getName() + "'");
            }
            ShaderInput* input = newNode->getInput(elem->getName());

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
        ShaderInput* input = newNode->getInput(nodeDefInput->getName());
        InputPtr nodeInput = node.getInput(nodeDefInput->getName());

        const string& connection = nodeInput ? nodeInput->getNodeName() : EMPTY_STRING;
        if (connection.empty() && !input->connection)
        {
            GeomPropDefPtr geomprop = nodeDefInput->getDefaultGeomProp();
            if (geomprop)
            {
                addDefaultGeomNode(input, *geomprop, shadergen, options);
            }
        }
    }

    ColorManagementSystemPtr colorManagementSystem = shadergen.getColorManagementSystem();
    const string& targetColorSpace =_document->getActiveColorSpace();

    if (colorManagementSystem && !targetColorSpace.empty())
    {
        for (InputPtr input : node.getInputs())
        {
            populateInputColorTransformMap(colorManagementSystem, newNode, input, targetColorSpace);
        }
        for (ParameterPtr parameter : node.getParameters())
        {
            populateInputColorTransformMap(colorManagementSystem, newNode, parameter, targetColorSpace);
        }

        // Check if this is a file texture node that requires color transformation.
        if (newNode->hasClassification(ShaderNode::Classification::FILETEXTURE))
        {
            ParameterPtr file = node.getParameter("file");
            if (file)
            {
                const TypeDesc* fileType = TypeDesc::get(node.getType());

                // Only color3 and color4 textures require color transformation.
                if (fileType == Type::COLOR3 || fileType == Type::COLOR4)
                {
                    const string& sourceColorSpace = file->getActiveColorSpace();

                    // If we're converting between two identical color spaces than we have no work to do.
                    if (!sourceColorSpace.empty() && sourceColorSpace != targetColorSpace)
                    {
                        ShaderOutput* shaderOutput = newNode->getOutput();
                        if (shaderOutput)
                        {
                            // Store the output and it's color transform so we can create this
                            // color transformation later when finalizing the graph.
                            ColorSpaceTransform transform(sourceColorSpace, targetColorSpace, fileType);
                            if (colorManagementSystem->supportsTransform(transform))
                            {
                                _outputColorTransformMap.emplace(shaderOutput, transform);
                            }
                        }
                    }
                }
            }
        }
    }

    return newNode.get();
}

ShaderGraphInputSocket* ShaderGraph::addInputSocket(const string& name, const TypeDesc* type)
{
    return ShaderNode::addOutput(name, type);
}

ShaderGraphOutputSocket* ShaderGraph::addOutputSocket(const string& name, const TypeDesc* type)
{
    return ShaderNode::addInput(name, type);
}

ShaderGraphEdgeIterator ShaderGraph::traverseUpstream(ShaderOutput* output)
{
    return ShaderGraphEdgeIterator(output);
}

ShaderNode* ShaderGraph::getNode(const string& name)
{
    auto it = _nodeMap.find(name);
    return it != _nodeMap.end() ? it->second.get() : nullptr;
}

void ShaderGraph::finalize(ShaderGenerator& shadergen, const GenOptions& options)
{
    // Insert color transformation nodes where needed
    for (auto it : _inputColorTransformMap)
    {
        addColorTransformNode(it.first, it.second, shadergen, options);
    }
    for (auto it : _outputColorTransformMap)
    {
        addColorTransformNode(it.first, it.second, shadergen, options);
    }
    _inputColorTransformMap.clear();
    _outputColorTransformMap.clear();

    // Optimize the graph, removing redundant paths.
    optimize();

    if (options.shaderInterfaceType == SHADER_INTERFACE_COMPLETE)
    {
        // Create uniforms for all node inputs that has not been connected already
        for (ShaderNode* node : getNodes())
        {
            for (ShaderInput* input : node->getInputs())
            {
                if (!input->connection)
                {
                    // Check if the type is editable otherwise we can't
                    // publish the input as an editable uniform.
                    if (input->type->isEditable() && node->isEditable(*input))
                    {
                        // Use a consistent naming convention: <nodename>_<inputname>
                        // so application side can figure out what uniforms to set
                        // when node inputs change on application side.
                        const string interfaceName = node->getName() + "_" + input->name;

                        ShaderGraphInputSocket* inputSocket = getInputSocket(interfaceName);
                        if (!inputSocket)
                        {
                            inputSocket = addInputSocket(interfaceName, input->type);
                            // Copy relevant data from internal input to the published socket
                            inputSocket->copyData(*input);
                        }
                        inputSocket->makeConnection(input);
                    }
                }
            }
        }
    }

    // Sort the nodes in topological order.
    topologicalSort();

    // Calculate scopes for all nodes in the graph.
    calculateScopes();

    // Set variable names for inputs and outputs in the graph.
    setVariableNames(shadergen);

    // Track closure nodes used by each surface shader.
    for (ShaderNode* node : _nodeOrder)
    {
        if (node->hasClassification(ShaderNode::Classification::SHADER))
        {
            for (ShaderGraphEdge edge : ShaderGraph::traverseUpstream(node->getOutput()))
            {
                if (edge.upstream)
                {
                    if (edge.upstream->node->hasClassification(ShaderNode::Classification::CLOSURE))
                    {
                        node->_usedClosures.insert(edge.upstream->node);
                    }
                }
            }
        }
    }
}

void ShaderGraph::disconnect(ShaderNode* node)
{
    for (ShaderInput* input : node->getInputs())
    {
        input->breakConnection();
    }
    for (ShaderOutput* output : node->getOutputs())
    {
        output->breakConnection();
    }
}

void ShaderGraph::optimize()
{
    size_t numEdits = 0;
    for (ShaderNode* node : getNodes())
    {
        if (!node->hasClassification(ShaderNode::Classification::DO_NOT_OPTIMIZE) && node->hasClassification(ShaderNode::Classification::CONSTANT))
        {
            // Constant nodes can be removed by assigning their value downstream
            // But don't remove it if it's connected upstream, i.e. it's value
            // input is published.
            ShaderInput* valueInput = node->getInput(0);
            if (!valueInput->connection)
            {
                bypass(node, 0);
                ++numEdits;
            }
        }
        else if (node->hasClassification(ShaderNode::Classification::IFELSE))
        {
            // Check if we have a constant conditional expression
            ShaderInput* intest = node->getInput("intest");
            if (!intest->connection || intest->connection->node->hasClassification(ShaderNode::Classification::CONSTANT))
            {
                // Find which branch should be taken
                ShaderInput* cutoff = node->getInput("cutoff");
                ValuePtr value = intest->connection ? intest->connection->node->getInput(0)->value : intest->value;
                const float intestValue = value ? value->asA<float>() : 0.0f;
                const int branch = (intestValue <= cutoff->value->asA<float>() ? 2 : 3);

                // Bypass the conditional using the taken branch
                bypass(node, branch);

                ++numEdits;
            }
        }
        else if (node->hasClassification(ShaderNode::Classification::SWITCH))
        {
            // Check if we have a constant conditional expression
            ShaderInput* which = node->getInput("which");
            if (!which->connection || which->connection->node->hasClassification(ShaderNode::Classification::CONSTANT))
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
        std::set<ShaderNode*> usedNodes;

        // Travers the graph to find nodes still in use
        for (ShaderGraphOutputSocket* outputSocket : getOutputSockets())
        {
            if (outputSocket->connection)
            {
                for (ShaderGraphEdge edge : ShaderGraph::traverseUpstream(outputSocket->connection))
                {
                    usedNodes.insert(edge.upstream->node);
                }
            }
        }

        // Remove any unused nodes
        for (ShaderNode* node : _nodeOrder)
        {
            if (usedNodes.count(node) == 0)
            {
                // Break all connections
                disconnect(node);

                // Erase from storage
                _nodeMap.erase(node->getName());
            }
        }

        _nodeOrder.resize(usedNodes.size());
        _nodeOrder.assign(usedNodes.begin(), usedNodes.end());
    }
}

void ShaderGraph::bypass(ShaderNode* node, size_t inputIndex, size_t outputIndex)
{
    ShaderInput* input = node->getInput(inputIndex);
    ShaderOutput* output = node->getOutput(outputIndex);

    ShaderOutput* upstream = input->connection;
    if (upstream)
    {
        // Re-route the upstream output to the downstream inputs.
        // Iterate a copy of the connection set since the
        // original set will change when breaking connections.
        ShaderInputSet downstreamConnections = output->connections;
        for (ShaderInput* downstream : downstreamConnections)
        {
            output->breakConnection(downstream);
            downstream->makeConnection(upstream);
        }
    }
    else
    {
        // No node connected upstream to re-route,
        // so push the input's value and element path downstream instead.
        // Iterate a copy of the connection set since the
        // original set will change when breaking connections.
        ShaderInputSet downstreamConnections = output->connections;
        for (ShaderInput* downstream : downstreamConnections)
        {
            output->breakConnection(downstream);
            downstream->copyData(*input);
        }
    }
}

void ShaderGraph::topologicalSort()
{
    // Calculate a topological order of the children, using Kahn's algorithm
    // to avoid recursion.
    //
    // Running time: O(numNodes + numEdges).

    // Calculate in-degrees for all nodes, and enqueue those with degree 0.
    std::unordered_map<ShaderNode*, int> inDegree(_nodeMap.size());
    std::deque<ShaderNode*> nodeQueue;
    for (auto it : _nodeMap)
    {
        ShaderNode* node = it.second.get();

        int connectionCount = 0;
        for (const ShaderInput* input : node->getInputs())
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
        ShaderNode* node = nodeQueue.front();
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

void ShaderGraph::calculateScopes()
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
    ShaderNode* lastNode = _nodeOrder[lastNodeIndex];
    lastNode->getScopeInfo().type = ShaderNode::ScopeInfo::Type::GLOBAL;

    std::set<ShaderNode*> nodeUsed;
    nodeUsed.insert(lastNode);

    // Iterate nodes in reversed toplogical order such that every node is visited AFTER
    // each of the nodes that depend on it have been processed first.
    for (int nodeIndex = int(lastNodeIndex); nodeIndex >= 0; --nodeIndex)
    {
        ShaderNode* node = _nodeOrder[nodeIndex];

        // Once we visit a node the scopeInfo has been determined and it will not be changed
        // By then we have visited all the nodes that depend on it already
        if (nodeUsed.count(node) == 0)
        {
            continue;
        }

        const bool isIfElse = node->hasClassification(ShaderNode::Classification::IFELSE);
        const bool isSwitch = node->hasClassification(ShaderNode::Classification::SWITCH);

        const ShaderNode::ScopeInfo& currentScopeInfo = node->getScopeInfo();

        for (size_t inputIndex = 0; inputIndex < node->numInputs(); ++inputIndex)
        {
            ShaderInput* input = node->getInput(inputIndex);

            if (input->connection)
            {
                ShaderNode* upstreamNode = input->connection->node;

                // Create scope info for this network brach
                // If it's a conditonal branch the scope is adjusted
                ShaderNode::ScopeInfo newScopeInfo = currentScopeInfo;
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
                ShaderNode::ScopeInfo& upstreamScopeInfo = upstreamNode->getScopeInfo();
                upstreamScopeInfo.merge(newScopeInfo);

                nodeUsed.insert(upstreamNode);
            }
        }
    }
}

void ShaderGraph::setVariableNames(ShaderGenerator& shadergen)
{
    // Make sure inputs and outputs have variable names valid for the
    // target shading language, and are unique to avoid name conflicts.

    // Names in use for the graph is recorded in 'uniqueNames'.
    Syntax::UniqueNameMap uniqueNames;
    for (ShaderGraphInputSocket* inputSocket : getInputSockets())
    {
        if (ShaderPort::VARIABLE_NOT_RENAMABLE & inputSocket->flags)
        {
            inputSocket->variable = inputSocket->name;
        }
        shadergen.getSyntax()->makeUnique(inputSocket->variable, uniqueNames);
    }
    for (ShaderGraphOutputSocket* outputSocket : getOutputSockets())
    {
        outputSocket->variable = outputSocket->name;
        shadergen.getSyntax()->makeUnique(outputSocket->variable, uniqueNames);
    }
    for (ShaderNode* node : getNodes())
    {
        for (ShaderInput* input : node->getInputs())
        {
            // Node outputs use long names for better code readability
            if (ShaderPort::VARIABLE_NOT_RENAMABLE & input->flags)
            {
                input->variable = input->node->getName() + "_" + input->name;
            }
            shadergen.getSyntax()->makeUnique(input->variable, uniqueNames);
        }
        for (ShaderOutput* output : node->getOutputs())
        {
            // Node outputs use long names for better code readability
            output->variable = output->node->getName() + "_" + output->name;
            shadergen.getSyntax()->makeUnique(output->variable, uniqueNames);
        }
    }
}

void ShaderGraph::populateInputColorTransformMap(ColorManagementSystemPtr colorManagementSystem, ShaderNodePtr shaderNode, ValueElementPtr input, const string& targetColorSpace)
{
    ShaderInput* shaderInput = shaderNode->getInput(input->getName());
    const string& sourceColorSpace = input->getActiveColorSpace();
    if (shaderInput && !sourceColorSpace.empty())
    {
        // Can skip inputs with connections as they are not legally allowed to have colorspaces specified.
        if (!shaderInput->connection)
        {
            if(shaderInput->type == Type::COLOR3 || shaderInput->type == Type::COLOR4)
            {
                // If we're converting between two identical color spaces than we have no work to do.
                if (sourceColorSpace != targetColorSpace)
                {
                    ColorSpaceTransform transform(sourceColorSpace, targetColorSpace, shaderInput->type);
                    if (colorManagementSystem->supportsTransform(transform))
                    {
                        _inputColorTransformMap.emplace(shaderInput, transform);
                    }
                }
            }
        }
    }
}

namespace
{
    static const ShaderGraphEdgeIterator NULL_EDGE_ITERATOR(nullptr);
}

ShaderGraphEdgeIterator::ShaderGraphEdgeIterator(ShaderOutput* output)
    : _upstream(output)
    , _downstream(nullptr)
{
}

ShaderGraphEdgeIterator& ShaderGraphEdgeIterator::operator++()
{
    if (_upstream && _upstream->node->numInputs())
    {
        // Traverse to the first upstream edge of this element.
        _stack.push_back(StackFrame(_upstream, 0));

        ShaderInput* input = _upstream->node->getInput(0);
        ShaderOutput* output = input->connection;

        if (output && !output->node->isAGraph())
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
            *this = ShaderGraphEdgeIterator::end();
            return *this;
        }

        // Traverse to our siblings.
        StackFrame& parentFrame = _stack.back();
        while (parentFrame.second + 1 < parentFrame.first->node->numInputs())
        {
            ShaderInput* input = parentFrame.first->node->getInput(++parentFrame.second);
            ShaderOutput* output = input->connection;

            if (output && !output->node->isAGraph())
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

const ShaderGraphEdgeIterator& ShaderGraphEdgeIterator::end()
{
    return NULL_EDGE_ITERATOR;
}

void ShaderGraphEdgeIterator::extendPathUpstream(ShaderOutput* upstream, ShaderInput* downstream)
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

void ShaderGraphEdgeIterator::returnPathDownstream(ShaderOutput* upstream)
{
    _path.erase(upstream);
    _upstream = nullptr;
    _downstream = nullptr;
}

} // namespace MaterialX
