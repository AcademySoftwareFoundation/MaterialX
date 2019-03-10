//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/ShaderGraph.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

//
// ShaderGraph methods
//

ShaderGraph::ShaderGraph(const ShaderGraph* parent, const string& name, ConstDocumentPtr document) :
    ShaderNode(parent, name),
    _document(document)
{
}

void ShaderGraph::addInputSockets(const InterfaceElement& elem, GenContext& context)
{
    for (ValueElementPtr port : elem.getChildrenOfType<ValueElement>())
    {
        if (!port->isA<Output>())
        {
            const string& portValue = port->getValueString();
            std::pair<const TypeDesc*, ValuePtr> enumResult;
            if (context.getShaderGenerator().remapEnumeration(*port, portValue, enumResult))
            {
                ShaderGraphInputSocket* inputSocket = addInputSocket(port->getName(), enumResult.first);
                inputSocket->setValue(enumResult.second);
            }
            else
            {
                ShaderGraphInputSocket* inputSocket = addInputSocket(port->getName(), TypeDesc::get(port->getType()));
                if (!portValue.empty())
                {
                    inputSocket->setValue(port->getValue());
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

void ShaderGraph::addUpstreamDependencies(const Element& root, ConstMaterialPtr material, GenContext& context)
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
            newNode = addNode(*upstreamNode, context);
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

void ShaderGraph::addDefaultGeomNode(ShaderInput* input, const GeomPropDef& geomprop, GenContext& context)
{
    const string geomNodeName = "geomprop_" + geomprop.getName();
    ShaderNode* node = getNode(geomNodeName);

    if (!node)
    {
        // Find the nodedef for the geometric node referenced by the geomprop. Use the type of the
        // input here and ignore the type of the geomprop. They are required to have the same type.
        string geomNodeDefName = "ND_" + geomprop.getNode() + "_" + input->getType()->getName();
        NodeDefPtr geomNodeDef = _document->getNodeDef(geomNodeDefName);
        if (!geomNodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef named '" + geomNodeDefName +
                "' for geomprop on input '" + input->getNode()->getName() + "." + input->getName() + "'");
        }

        ShaderNodePtr geomNode = ShaderNode::create(this, geomNodeName, *geomNodeDef, context);
        _nodeMap[geomNodeName] = geomNode;
        _nodeOrder.push_back(geomNode.get());

        // Set node inputs if given.
        const string& namePath = geomprop.getNamePath();
        const string& space = geomprop.getSpace();
        if (!space.empty())
        {
            ShaderInput* spaceInput = geomNode->getInput(GeomPropDef::SPACE_ATTRIBUTE);
            ValueElementPtr nodeDefSpaceInput = geomNodeDef->getChildOfType<ValueElement>(GeomPropDef::SPACE_ATTRIBUTE);
            if (spaceInput && nodeDefSpaceInput)
            {
                std::pair<const TypeDesc*, ValuePtr> enumResult;
                if (context.getShaderGenerator().remapEnumeration(*nodeDefSpaceInput, space, enumResult))
                {
                    spaceInput->setValue(enumResult.second);
                }
                else
                {
                    spaceInput->setValue(Value::createValue<string>(space));
                }
                spaceInput->setPath(namePath);
            }
        }
        const string& index = geomprop.getIndex();
        if (!index.empty())
        {
            ShaderInput* indexInput = geomNode->getInput("index");
            if (indexInput)
            {
                indexInput->setValue(Value::createValue<string>(index));
                indexInput->setPath(namePath);
            }
        }
        const string& attrname = geomprop.getAttrName();
        if (!attrname.empty())
        {
            ShaderInput* attrnameInput = geomNode->getInput("attrname");
            if (attrnameInput)
            {
                attrnameInput->setValue(Value::createValue<string>(attrname));
                attrnameInput->setPath(namePath);
            }
        }

        node = geomNode.get();
    }

    input->makeConnection(node->getOutput());
}

void ShaderGraph::addColorTransformNode(ShaderInput* input, const ColorSpaceTransform& transform, GenContext& context)
{
    ColorManagementSystemPtr colorManagementSystem = context.getShaderGenerator().getColorManagementSystem();
    if (!colorManagementSystem)
    {
        return;
    }
    string colorTransformNodeName = input->getNode()->getName() + "_" + input->getName() + "_cm";
    ShaderNodePtr colorTransformNodePtr = colorManagementSystem->createNode(this, transform, colorTransformNodeName, context);

    if (colorTransformNodePtr)
    {
        _nodeMap[colorTransformNodePtr->getName()] = colorTransformNodePtr;
        _nodeOrder.push_back(colorTransformNodePtr.get());

        ShaderNode* colorTransformNode = colorTransformNodePtr.get();
        ShaderOutput* colorTransformNodeOutput = colorTransformNode->getOutput(0);

        ShaderInput* shaderInput = colorTransformNode->getInput(0);
        shaderInput->setVariable(input->getNode()->getName() + "_" + input->getName());
        shaderInput->setValue(input->getValue());
        shaderInput->setPath(input->getPath());

        input->makeConnection(colorTransformNodeOutput);
    }
}

void ShaderGraph::addColorTransformNode(ShaderOutput* output, const ColorSpaceTransform& transform, GenContext& context)
{
    ColorManagementSystemPtr colorManagementSystem = context.getShaderGenerator().getColorManagementSystem();
    if (!colorManagementSystem)
    {
        return;
    }
    string colorTransformNodeName = output->getNode()->getName() + "_" + output->getName() + "_cm";
    ShaderNodePtr colorTransformNodePtr = colorManagementSystem->createNode(this, transform, colorTransformNodeName, context);

    if (colorTransformNodePtr)
    {
        _nodeMap[colorTransformNodePtr->getName()] = colorTransformNodePtr;
        _nodeOrder.push_back(colorTransformNodePtr.get());

        ShaderNode* colorTransformNode = colorTransformNodePtr.get();
        ShaderOutput* colorTransformNodeOutput = colorTransformNode->getOutput(0);

        ShaderInputSet inputs = output->getConnections();
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

ShaderGraphPtr ShaderGraph::create(const ShaderGraph* parent, const NodeGraph& nodeGraph, GenContext& context)
{
    NodeDefPtr nodeDef = nodeGraph.getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Can't find nodedef '" + nodeGraph.getNodeDefString() + "' referenced by nodegraph '" + nodeGraph.getName() + "'");
    }

    string graphName = nodeGraph.getName();
    context.getShaderGenerator().getSyntax().makeValidName(graphName);
    ShaderGraphPtr graph = std::make_shared<ShaderGraph>(parent, graphName, nodeGraph.getDocument());

    // Clear classification
    graph->_classification = 0;

    // Create input sockets from the nodedef
    graph->addInputSockets(*nodeDef, context);

    // Create output sockets from the nodegraph
    graph->addOutputSockets(nodeGraph);

    // Traverse all outputs and create all upstream dependencies
    for (OutputPtr graphOutput : nodeGraph.getOutputs())
    {
        graph->addUpstreamDependencies(*graphOutput, nullptr, context);
    }

    // Add classification according to last node
    // TODO: What if the graph has multiple outputs?
    {
        ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket();
        graph->_classification |= outputSocket->getConnection() ? outputSocket->getConnection()->getNode()->_classification : 0;
    }

    // Finalize the graph
    graph->finalize(context);

    return graph;
}

ShaderGraphPtr ShaderGraph::create(const ShaderGraph* parent, const string& name, ElementPtr element, GenContext& context)
{
    ShaderGraphPtr graph;
    ElementPtr root;
    MaterialPtr material;

    if (element->isA<Output>())
    {
        OutputPtr output = element->asA<Output>();
        ElementPtr outputParent = output->getParent();
        InterfaceElementPtr interface = outputParent->asA<InterfaceElement>();

        if (outputParent->isA<NodeGraph>())
        {
            NodeDefPtr nodeDef = outputParent->asA<NodeGraph>()->getNodeDef();
            if (nodeDef)
            {
                interface = nodeDef;
            }
        }

        if (!interface)
        {
            outputParent = output->getConnectedNode();
            interface = outputParent ? outputParent->asA<InterfaceElement>() : nullptr;
            if (!interface)
            {
                throw ExceptionShaderGenError("Given output '" + output->getName() + "' has no interface valid for shader generation");
            }
        }

        graph = std::make_shared<ShaderGraph>(parent, name, element->getDocument());

        // Clear classification
        graph->_classification = 0;

        // Create input sockets
        graph->addInputSockets(*interface, context);

        // Create the given output socket
        ShaderGraphOutputSocket* outputSocket = graph->addOutputSocket(output->getName(), TypeDesc::get(output->getType()));
        outputSocket->setPath(output->getNamePath());

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

        graph = std::make_shared<ShaderGraph>(parent, name, element->getDocument());

        // Create input sockets
        graph->addInputSockets(*nodeDef, context);

        // Create output sockets
        graph->addOutputSockets(*nodeDef);

        // Create this shader node in the graph.
        const string& newNodeName = shaderRef->getName();
        ShaderNodePtr newNode = ShaderNode::create(graph.get(), newNodeName, *nodeDef, context);
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
                    inputSocket->setValue(bindParam->getValue());
                }
                inputSocket->setPath(bindParam->getNamePath());
                input->setPath(inputSocket->getPath());
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
                    inputSocket->setValue(bindInput->getValue());
                }
                inputSocket->setPath(bindInput->getNamePath());
                input->setPath(inputSocket->getPath());
            }

            // If no explicit connection, connect to geometric node if a geomprop is used
            // or otherwise to the graph interface.
            const string& connection = bindInput ? bindInput->getOutputString() : EMPTY_STRING;
            if (connection.empty())
            {
                GeomPropDefPtr geomprop = nodeDefInput->getDefaultGeomProp();
                if (geomprop)
                {
                    graph->addDefaultGeomNode(input, *geomprop, context);
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
            if (input && input->getPath().empty())
            {
                input->setPath(path);
            }
            ShaderGraphInputSocket* inputSocket = graph->getInputSocket(inputName);
            if (inputSocket && inputSocket->getPath().empty())
            {
                inputSocket->setPath(path);
            }
        }
        const vector<ParameterPtr> nodeParameters = nodeDef->getChildrenOfType<Parameter>();
        for (const ParameterPtr& nodeParameter : nodeParameters)
        {
            const string& paramName = nodeParameter->getName();
            const string path = nodePath + NAME_PATH_SEPARATOR + paramName;
            ShaderInput* input = newNode->getInput(paramName);
            if (input && input->getPath().empty())
            {
                input->setPath(path);
            }
            ShaderGraphInputSocket* inputSocket = graph->getInputSocket(paramName);
            if (inputSocket && inputSocket->getPath().empty())
            {
                inputSocket->setPath(path);
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
    graph->addUpstreamDependencies(*root, material, context);

    // Add classification according to root node
    ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket();
    graph->_classification |= outputSocket->getConnection() ? outputSocket->getConnection()->getNode()->_classification : 0;

    graph->finalize(context);

    return graph;
}

ShaderNode* ShaderGraph::addNode(const Node& node, GenContext& context)
{
    NodeDefPtr nodeDef = node.getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Could not find a nodedef for node '" + node.getName() + "'");
    }

    // Create this node in the graph.
    const string& name = node.getName();
    ShaderNodePtr newNode = ShaderNode::create(this, name, *nodeDef, context);
    newNode->setValues(node, *nodeDef, context);
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
        if (connection.empty() && !input->getConnection())
        {
            GeomPropDefPtr geomprop = nodeDefInput->getDefaultGeomProp();
            if (geomprop)
            {
                addDefaultGeomNode(input, *geomprop, context);
            }
        }
    }

    ColorManagementSystemPtr colorManagementSystem = context.getShaderGenerator().getColorManagementSystem();
    const string& targetColorSpace = context.getOptions().targetColorSpaceOverride.empty() ?
        _document->getActiveColorSpace() : context.getOptions().targetColorSpaceOverride;

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

const ShaderNode* ShaderGraph::getNode(const string& name) const
{
    return const_cast<ShaderGraph*>(this)->getNode(name);
}

void ShaderGraph::finalize(GenContext& context)
{
    // Insert color transformation nodes where needed
    for (auto it : _inputColorTransformMap)
    {
        addColorTransformNode(it.first, it.second, context);
    }
    for (auto it : _outputColorTransformMap)
    {
        addColorTransformNode(it.first, it.second, context);
    }
    _inputColorTransformMap.clear();
    _outputColorTransformMap.clear();

    // Optimize the graph, removing redundant paths.
    optimize();

    if (context.getOptions().shaderInterfaceType == SHADER_INTERFACE_COMPLETE)
    {
        // Publish all node inputs that has not been connected already.
        for (const ShaderNode* node : getNodes())
        {
            for (ShaderInput* input : node->getInputs())
            {
                if (!input->getConnection())
                {
                    // Check if the type is editable otherwise we can't
                    // publish the input as an editable uniform.
                    if (input->getType()->isEditable() && node->isEditable(*input))
                    {
                        // Use a consistent naming convention: <nodename>_<inputname>
                        // so application side can figure out what uniforms to set
                        // when node inputs change on application side.
                        const string interfaceName = node->getName() + "_" + input->getName();

                        ShaderGraphInputSocket* inputSocket = getInputSocket(interfaceName);
                        if (!inputSocket)
                        {
                            inputSocket = addInputSocket(interfaceName, input->getType());
                            inputSocket->setPath(input->getPath());
                            inputSocket->setValue(input->getValue());
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
    setVariableNames(context.getShaderGenerator().getSyntax());

    // Track closure nodes used by each surface shader.
    //
    // TODO: Optimize this search for closures.
    //       No need to do a full traversal when 
    //       texture nodes are reached.
    for (ShaderNode* node : _nodeOrder)
    {
        if (node->hasClassification(ShaderNode::Classification::SHADER))
        {
            for (ShaderGraphEdge edge : ShaderGraph::traverseUpstream(node->getOutput()))
            {
                if (edge.upstream)
                {
                    if (edge.upstream->getNode()->hasClassification(ShaderNode::Classification::CLOSURE))
                    {
                        node->_usedClosures.insert(edge.upstream->getNode());
                    }
                }
            }
        }
    }
}

void ShaderGraph::disconnect(ShaderNode* node) const
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
            if (!valueInput->getConnection())
            {
                bypass(node, 0);
                ++numEdits;
            }
        }
        else if (node->hasClassification(ShaderNode::Classification::IFELSE))
        {
            // Check if we have a constant conditional expression
            ShaderInput* intest = node->getInput("intest");
            if (!intest->getConnection() || intest->getConnection()->getNode()->hasClassification(ShaderNode::Classification::CONSTANT))
            {
                // Find which branch should be taken
                ShaderInput* cutoff = node->getInput("cutoff");
                ValuePtr value = intest->getConnection() ? intest->getConnection()->getNode()->getInput(0)->getValue() : intest->getValue();
                const float intestValue = value ? value->asA<float>() : 0.0f;
                const int branch = (intestValue <= cutoff->getValue()->asA<float>() ? 2 : 3);

                // Bypass the conditional using the taken branch
                bypass(node, branch);

                ++numEdits;
            }
        }
        else if (node->hasClassification(ShaderNode::Classification::SWITCH))
        {
            // Check if we have a constant conditional expression
            ShaderInput* which = node->getInput("which");
            if (!which->getConnection() || which->getConnection()->getNode()->hasClassification(ShaderNode::Classification::CONSTANT))
            {
                // Find which branch should be taken
                ValuePtr value = which->getConnection() ? which->getConnection()->getNode()->getInput(0)->getValue() : which->getValue();
                const int branch = int(value==nullptr ? 0 :
                    (which->getType() == Type::BOOLEAN ? value->asA<bool>() :
                    (which->getType() == Type::FLOAT ? value->asA<float>() : value->asA<int>())));

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
            if (outputSocket->getConnection())
            {
                for (ShaderGraphEdge edge : ShaderGraph::traverseUpstream(outputSocket->getConnection()))
                {
                    usedNodes.insert(edge.upstream->getNode());
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

    ShaderOutput* upstream = input->getConnection();
    if (upstream)
    {
        // Re-route the upstream output to the downstream inputs.
        // Iterate a copy of the connection set since the
        // original set will change when breaking connections.
        ShaderInputSet downstreamConnections = output->getConnections();
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
        ShaderInputSet downstreamConnections = output->getConnections();
        for (ShaderInput* downstream : downstreamConnections)
        {
            output->breakConnection(downstream);
            downstream->setValue(input->getValue());
            downstream->setPath(input->getPath());
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
            if (input->getConnection() && input->getConnection()->getNode() != this)
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
            for (auto input : output->getConnections())
            {
                if (input->getNode() != this)
                {
                    if (--inDegree[input->getNode()] <= 0)
                    {
                        nodeQueue.push_back(input->getNode());
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
    lastNode->getScopeInfo().type = ShaderNode::ScopeInfo::GLOBAL;

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

            if (input->getConnection())
            {
                ShaderNode* upstreamNode = input->getConnection()->getNode();

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

void ShaderGraph::setVariableNames(const Syntax& syntax)
{
    // Make sure inputs and outputs have variable names valid for the
    // target shading language, and are unique to avoid name conflicts.

    // Names in use for the graph is recorded in 'uniqueNames'.
    Syntax::UniqueNameMap uniqueNames;
    for (ShaderGraphInputSocket* inputSocket : getInputSockets())
    {
        string variable = inputSocket->getName();
        syntax.makeUnique(variable, uniqueNames);
        inputSocket->setVariable(variable);
    }
    for (ShaderGraphOutputSocket* outputSocket : getOutputSockets())
    {
        string variable = outputSocket->getName();
        syntax.makeUnique(variable, uniqueNames);
        outputSocket->setVariable(variable);
    }
    for (ShaderNode* node : getNodes())
    {
        for (ShaderInput* input : node->getInputs())
        {
            string variable = input->getNode()->getName() + "_" + input->getName();
            syntax.makeUnique(variable, uniqueNames);
            input->setVariable(variable);
        }
        for (ShaderOutput* output : node->getOutputs())
        {
            // Node outputs use long names for better code readability
            string variable = output->getNode()->getName() + "_" + output->getName();
            syntax.makeUnique(variable, uniqueNames);
            output->setVariable(variable);
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
        if (!shaderInput->getConnection())
        {
            if(shaderInput->getType() == Type::COLOR3 || shaderInput->getType() == Type::COLOR4)
            {
                // If we're converting between two identical color spaces than we have no work to do.
                if (sourceColorSpace != targetColorSpace)
                {
                    ColorSpaceTransform transform(sourceColorSpace, targetColorSpace, shaderInput->getType());
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

//
// ShaderGraphEdgeIterator methods
//

ShaderGraphEdgeIterator::ShaderGraphEdgeIterator(ShaderOutput* output) :
    _upstream(output),
    _downstream(nullptr)
{
}

ShaderGraphEdgeIterator& ShaderGraphEdgeIterator::operator++()
{
    if (_upstream && _upstream->getNode()->numInputs())
    {
        // Traverse to the first upstream edge of this element.
        _stack.push_back(StackFrame(_upstream, 0));

        ShaderInput* input = _upstream->getNode()->getInput(0);
        ShaderOutput* output = input->getConnection();

        if (output && !output->getNode()->isAGraph())
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
        while (parentFrame.second + 1 < parentFrame.first->getNode()->numInputs())
        {
            ShaderInput* input = parentFrame.first->getNode()->getInput(++parentFrame.second);
            ShaderOutput* output = input->getConnection();

            if (output && !output->getNode()->isAGraph())
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
        throw ExceptionFoundCycle("Encountered cycle at element: " + upstream->getNode()->getName() + "." + upstream->getName());
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
