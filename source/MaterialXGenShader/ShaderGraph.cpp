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

ShaderGraph::ShaderGraph(const ShaderGraph* parent, const string& name, ConstDocumentPtr document, const StringSet& reservedWords) :
    ShaderNode(parent, name),
    _document(document)
{
    // Add all reserved words as taken identifiers
    for (const string& n : reservedWords)
    {
        _identifiers[n] = 1;
    }
}

void ShaderGraph::addInputSockets(const InterfaceElement& elem, GenContext& context)
{
    for (ValueElementPtr port : elem.getActiveValueElements())
    {
        if (!port->isA<Output>())
        {
            const string& portValue = port->getResolvedValueString();
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
    for (const OutputPtr& output : elem.getActiveOutputs())
    {
        ShaderGraphOutputSocket* outputSocket = addOutputSocket(output->getName(), TypeDesc::get(output->getType()));
        outputSocket->setChannels(output->getChannels());
    }
    if (numOutputSockets() == 0)
    {
        addOutputSocket("out", TypeDesc::get(elem.getType()));
    }
}

void ShaderGraph::createConnectedNodes(const ElementPtr& downstreamElement,
                                       const ElementPtr& upstreamElement,
                                       ElementPtr connectingElement,
                                       GenContext& context,
                                       ShaderNode* rootNode)
{
    // Create the node if it doesn't exists
    NodePtr upstreamNode = upstreamElement->asA<Node>();
    if (!upstreamNode)
    {
        throw ExceptionShaderGenError("Upstream element to connect is not a node '" 
            + upstreamElement->getName() + "'");
    }
    const string& newNodeName = upstreamNode->getName();
    ShaderNode* newNode = getNode(newNodeName);
    if (!newNode)
    {
        newNode = createNode(*upstreamNode, context);
    }

    //
    // Make connections
    //

    // Find the output to connect to.
    if (!connectingElement && downstreamElement->isA<Output>())
    {
        // Edge case for having an output downstream but no connecting
        // element (input) reported upstream. In this case we set the
        // output itself as connecting element, which handles finding
        // the nodedef output in case of a multioutput node upstream.
        connectingElement = downstreamElement->asA<Output>();
    }
    OutputPtr nodeDefOutput = connectingElement ? upstreamNode->getNodeDefOutput(connectingElement) : nullptr;
    ShaderOutput* output = nodeDefOutput ? newNode->getOutput(nodeDefOutput->getName()) : newNode->getOutput();
    if (!output)
    {
        throw ExceptionShaderGenError("Could not find an output named '" + (nodeDefOutput ? nodeDefOutput->getName() : string("out")) +
            "' on upstream node '" + upstreamNode->getName() + "'");
    }

    // First check if this was a bind input connection
    // In this case we must have a root node as well
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
            else
            {
                throw ExceptionShaderGenError("Could not find downstream node ' " + downstreamNode->getName() + "'");
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

        createConnectedNodes(downstreamElement,
                             upstreamElement,
                             edge.getConnectingElement(),
                             context,
                             rootNode);
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
        string geomNodeDefName = "ND_" + geomprop.getGeomProp() + "_" + input->getType()->getName();
        NodeDefPtr geomNodeDef = _document->getNodeDef(geomNodeDefName);
        if (!geomNodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef named '" + geomNodeDefName +
                "' for defaultgeomprop on input '" + input->getFullName() + "'");
        }

        ShaderNodePtr geomNode = ShaderNode::create(this, geomNodeName, *geomNodeDef, context);
        addNode(geomNode);

        // Set node inputs if given.
        const string& namePath = geomprop.getNamePath();
        const string& space = geomprop.getSpace();
        if (!space.empty())
        {
            ShaderInput* spaceInput = geomNode->getInput(GeomPropDef::SPACE_ATTRIBUTE);
            ValueElementPtr nodeDefSpaceInput = geomNodeDef->getActiveValueElement(GeomPropDef::SPACE_ATTRIBUTE);
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
        const string& geomProp = geomprop.getGeomProp();
        if (!geomProp.empty())
        {
            ShaderInput* geomPropInput = geomNode->getInput(GeomPropDef::GEOM_PROP_ATTRIBUTE);
            if (geomPropInput)
            {
                geomPropInput->setValue(Value::createValue<string>(geomProp));
                geomPropInput->setPath(namePath);
            }
        }

        node = geomNode.get();
    }

    input->makeConnection(node->getOutput());
}

void ShaderGraph::addColorTransformNode(ShaderInput* input, const ColorSpaceTransform& transform, GenContext& context)
{
    ColorManagementSystemPtr colorManagementSystem = context.getShaderGenerator().getColorManagementSystem();
    if (!input->isBindInput() && (!colorManagementSystem || input->getConnection()))
    {
        // Ignore unbound inputs with connections as they are not 
        // allowed to have colorspaces specified.
        return;
    }
    const string colorTransformNodeName = input->getFullName() + "_cm";
    ShaderNodePtr colorTransformNodePtr = colorManagementSystem->createNode(this, transform, colorTransformNodeName, context);

    if (colorTransformNodePtr)
    {
        addNode(colorTransformNodePtr);

        ShaderNode* colorTransformNode = colorTransformNodePtr.get();
        ShaderOutput* colorTransformNodeOutput = colorTransformNode->getOutput(0);

        ShaderInput* shaderInput = colorTransformNode->getInput(0);
        shaderInput->setVariable(input->getFullName());
        shaderInput->setValue(input->getValue());
        shaderInput->setPath(input->getPath());
        shaderInput->setUnit(EMPTY_STRING);

        if (input->isBindInput())
        {
            ShaderOutput* oldConnection = input->getConnection();
            shaderInput->makeConnection(oldConnection);
        }

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

    const string colorTransformNodeName = output->getFullName() + "_cm";
    ShaderNodePtr colorTransformNodePtr = colorManagementSystem->createNode(this, transform, colorTransformNodeName, context);

    if (colorTransformNodePtr)
    {
        addNode(colorTransformNodePtr);

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

void ShaderGraph::addUnitTransformNode(ShaderInput* input, const UnitTransform& transform, GenContext& context)
{
    UnitSystemPtr unitSystem = context.getShaderGenerator().getUnitSystem();
    if (!input->isBindInput() && (!unitSystem || input->getConnection()))
    {
        return;
    }
    const string unitTransformNodeName = input->getFullName() + "_unit";
    ShaderNodePtr unitTransformNodePtr = unitSystem->createNode(this, transform, unitTransformNodeName, context);

    if (unitTransformNodePtr)
    {
        addNode(unitTransformNodePtr);

        ShaderNode* unitTransformNode = unitTransformNodePtr.get();
        ShaderOutput* unitTransformNodeOutput = unitTransformNode->getOutput(0);

        ShaderInput* shaderInput = unitTransformNode->getInput(0);
        shaderInput->setVariable(input->getFullName());
        shaderInput->setValue(input->getValue());
        shaderInput->setPath(input->getPath());
        shaderInput->setUnit(input->getUnit());

        if (input->isBindInput())
        {
            ShaderOutput* oldConnection = input->getConnection();
            shaderInput->makeConnection(oldConnection);
        }

        input->makeConnection(unitTransformNodeOutput);
    }
}

void ShaderGraph::addUnitTransformNode(ShaderOutput* output, const UnitTransform& transform, GenContext& context)
{
    UnitSystemPtr unitSystem = context.getShaderGenerator().getUnitSystem();
    if (!unitSystem)
    {
        return;
    }

    const string unitTransformNodeName = output->getFullName() + "_unit";
    ShaderNodePtr unitTransformNodePtr = unitSystem->createNode(this, transform, unitTransformNodeName, context);

    if (unitTransformNodePtr)
    {
        addNode(unitTransformNodePtr);

        ShaderNode* unitTransformNode = unitTransformNodePtr.get();
        ShaderOutput* unitTransformNodeOutput = unitTransformNode->getOutput(0);

        ShaderInputSet inputs = output->getConnections();
        for (ShaderInput* input : inputs)
        {
            string inname = input->getFullName();
            input->breakConnection();
            input->makeConnection(unitTransformNodeOutput);
        }

        // Connect the node to the upstream output
        ShaderInput* unitTransformNodeInput = unitTransformNode->getInput(0);
        unitTransformNodeInput->makeConnection(output);
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
    ShaderGraphPtr graph = std::make_shared<ShaderGraph>(parent, graphName, nodeGraph.getDocument(), context.getReservedWords());

    // Clear classification
    graph->_classification = 0;

    // Create input sockets from the nodedef
    graph->addInputSockets(*nodeDef, context);

    // Create output sockets from the nodegraph
    graph->addOutputSockets(nodeGraph);

    // Traverse all outputs and create all upstream dependencies
    for (OutputPtr graphOutput : nodeGraph.getActiveOutputs())
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

ShaderGraphPtr ShaderGraph::createSurfaceShader(
    const string& name,
    const ShaderGraph* parent,
    NodePtr node,
    GenContext& context,
    ElementPtr& root,
    MaterialPtr& material)
{
    NodeDefPtr nodeDef = node->getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Could not find a nodedef for shadernode '" + node->getName() + "'");
    }

    ShaderGraphPtr graph = std::make_shared<ShaderGraph>(parent, name, node->getDocument(), context.getReservedWords());

    // Create input sockets
    graph->addInputSockets(*nodeDef, context);

    // Create output sockets
    graph->addOutputSockets(*nodeDef);

    // Create this shader node in the graph.
    const string& newNodeName = node->getName();
    ShaderNodePtr newNode = ShaderNode::create(graph.get(), newNodeName, *nodeDef, context);
    graph->addNode(newNode);

    // Connect it to the graph output
    ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket();
    outputSocket->makeConnection(newNode->getOutput());
    outputSocket->setPath(node->getNamePath());

    string targetColorSpace;
    ColorManagementSystemPtr colorManagementSystem = context.getShaderGenerator().getColorManagementSystem();
    if (colorManagementSystem)
    {
        targetColorSpace = context.getOptions().targetColorSpaceOverride.empty() ?
            node->getDocument()->getColorSpace() : context.getOptions().targetColorSpaceOverride;
    }

    const string& targetDistanceUnit = context.getOptions().targetDistanceUnit;
    UnitSystemPtr unitSystem = context.getShaderGenerator().getUnitSystem();

    // Set node parameter values onto graph input sockets
    for (ParameterPtr elem : nodeDef->getActiveParameters())
    {
        ShaderGraphInputSocket* inputSocket = graph->getInputSocket(elem->getName());
        ShaderInput* input = newNode->getInput(elem->getName());
        if (!inputSocket || !input)
        {
            throw ExceptionShaderGenError("Shader parameter '" + elem->getName() + "' doesn't match an existing input on graph '" + graph->getName() + "'");
        }

        ParameterPtr nodeParam = node->getParameter(elem->getName());
        if (nodeParam)
        {
            // Copy value from binding
            ValuePtr nodeParamValue = nodeParam->getResolvedValue();
            if (nodeParamValue)
            {
                inputSocket->setValue(nodeParamValue);

                graph->populateColorTransformMap(colorManagementSystem, input, nodeParam, targetColorSpace, true);
                graph->populateUnitTransformMap(unitSystem, input, nodeParam, targetDistanceUnit, true);
            }
            inputSocket->setPath(nodeParam->getNamePath());
            input->setPath(inputSocket->getPath());
            const string& nodeParamUnit = nodeParam->getUnit();
            if (!nodeParamUnit.empty())
            {
                inputSocket->setUnit(nodeParamUnit);
                input->setUnit(nodeParamUnit);
            }
        }

        // Connect graph input socket to the node input
        inputSocket->makeConnection(input);
    }

    // Set node input values onto grah input sockets
    for (const InputPtr& nodeDefInput : nodeDef->getActiveInputs())
    {
        ShaderGraphInputSocket* inputSocket = graph->getInputSocket(nodeDefInput->getName());
        ShaderInput* input = newNode->getInput(nodeDefInput->getName());
        if (!inputSocket || !input)
        {
            throw ExceptionShaderGenError("Shader input '" + nodeDefInput->getName() + "' doesn't match an existing input on graph '" + graph->getName() + "'");
        }

        InputPtr nodeInput = node->getInput(nodeDefInput->getName());
        if (nodeInput)
        {
            // Copy value from binding
            ValuePtr nodeInputValue = nodeInput->getResolvedValue();
            if (nodeInputValue)
            {
                inputSocket->setValue(nodeInputValue);
                graph->populateColorTransformMap(colorManagementSystem, input, nodeInput, targetColorSpace, true);
                graph->populateUnitTransformMap(unitSystem, input, nodeInput, targetDistanceUnit, true);
            }
            inputSocket->setPath(nodeInput->getNamePath());
            input->setPath(inputSocket->getPath());
            const string& nodeInputUnit = nodeInput->getUnit();
            if (!nodeInputUnit.empty())
            {
                inputSocket->setUnit(nodeInputUnit);
                input->setUnit(nodeInputUnit);
            }
        }

        GeomPropDefPtr geomprop = nodeDefInput->getDefaultGeomProp();
        if (geomprop)
        {
            inputSocket->setGeomProp(geomprop->getName());
            input->setGeomProp(geomprop->getName());
        }

        // If no explicit connection, connect to geometric node if a geomprop is used
        // or otherwise to the graph interface.
        const string& connection = nodeInput ? nodeInput->getOutputString() : EMPTY_STRING;
        if (connection.empty())
        {
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

    // Add shader node paths and unit value
    const string& nodePath = node->getNamePath();
    for (const ValueElementPtr& nodeInput : nodeDef->getActiveInputs())
    {
        const string& inputName = nodeInput->getName();
        const string path = nodePath + NAME_PATH_SEPARATOR + inputName;
        const string& unit = nodeInput->getUnit();
        ShaderInput* input = newNode->getInput(inputName);
        if (input && input->getPath().empty())
        {
            input->setPath(path);
        }
        if (input && input->getUnit().empty() && !unit.empty())
        {
            input->setUnit(unit);
        }
        ShaderGraphInputSocket* inputSocket = graph->getInputSocket(inputName);
        if (inputSocket && inputSocket->getPath().empty())
        {
            inputSocket->setPath(path);
        }
        if (inputSocket && inputSocket->getUnit().empty() && !unit.empty())
        {
            inputSocket->setUnit(unit);
        }
    }
    for (const ParameterPtr& nodeParameter : nodeDef->getActiveParameters())
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

    // Start traversal from this shader node
    root = node;    
    material = nullptr; // node->getParent()->asA<Material>(); -- send over material node instead?
    
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

        InterfaceElementPtr interface;
        if (outputParent->isA<NodeGraph>())
        {
            // A nodegraph output.
            NodeGraphPtr nodeGraph = outputParent->asA<NodeGraph>();
            NodeDefPtr nodeDef = nodeGraph->getNodeDef();
            if (nodeDef)
            {
                interface = nodeDef;
            }
            else
            {
                interface = nodeGraph;
            }
        }
        else if (outputParent->isA<Document>())
        {
            // A free floating output.
            outputParent = output->getConnectedNode();
            interface = outputParent->asA<InterfaceElement>();
        }
        if (!interface)
        {
            throw ExceptionShaderGenError("Given output '" + output->getName() + "' has no interface valid for shader generation");
        }

        graph = std::make_shared<ShaderGraph>(parent, name, element->getDocument(), context.getReservedWords());

        // Clear classification
        graph->_classification = 0;

        // Create input sockets
        graph->addInputSockets(*interface, context);

        // Create the given output socket
        ShaderGraphOutputSocket* outputSocket = graph->addOutputSocket(output->getName(), TypeDesc::get(output->getType()));
        outputSocket->setPath(output->getNamePath());
        outputSocket->setChannels(output->getChannels());
        const string& outputUnit = output->getUnit();
        if (!outputUnit.empty())
        {
            outputSocket->setUnit(outputUnit);
        }

        // Start traversal from this output
        root = output;
    }

    else if (element->isA<Node>())
    {
        NodePtr node = element->asA<Node>();

        // Handle shader nodes different from other nodes
        if (node->getType() == SURFACE_SHADER_TYPE_STRING)
        {
            graph = createSurfaceShader(name, parent, node, context,
                root, material);
        }
        else
        {
            NodeDefPtr nodeDef = node->getNodeDef();
            if (!nodeDef)
            {
                throw ExceptionShaderGenError("Could not find a nodedef for node '" + node->getName() + "'");
            }

            graph = std::make_shared<ShaderGraph>(parent, name, element->getDocument(), context.getReservedWords());

            // Create input sockets
            graph->addInputSockets(*nodeDef, context);

            // Create output sockets
            graph->addOutputSockets(*nodeDef);

            // Create this shader node in the graph.
            ShaderNodePtr newNode = ShaderNode::create(graph.get(), node->getName(), *nodeDef, context);
            graph->addNode(newNode);

            // Connect it to the graph outputs
            for (size_t i = 0; i < newNode->numOutputs(); ++i)
            {
                ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket(i);
                outputSocket->makeConnection(newNode->getOutput(i));
                outputSocket->setPath(node->getNamePath());
            }

            // Handle node input ports
            for (const ValueElementPtr& nodedefPort : nodeDef->getActiveValueElements())
            {
                if (nodedefPort->isA<Output>())
                    continue;

                ShaderGraphInputSocket* inputSocket = graph->getInputSocket(nodedefPort->getName());
                ShaderInput* input = newNode->getInput(nodedefPort->getName());
                if (!inputSocket || !input)
                {
                    throw ExceptionShaderGenError("Node port '" + nodedefPort->getName() + "' doesn't match an existing input on graph '" + graph->getName() + "'");
                }

                ValueElementPtr nodePort = node->getValueElement(nodedefPort->getName());
                if (nodePort)
                {
                    ValuePtr value = nodePort->getResolvedValue();
                    if (value)
                    {
                        inputSocket->setValue(value);
                    }

                    inputSocket->setPath(nodePort->getNamePath());
                    input->setPath(inputSocket->getPath());

                    const string& unit = nodePort->getUnit();
                    if (!unit.empty())
                    {
                        inputSocket->setUnit(unit);
                        input->setUnit(unit);
                    }
                }

                if (nodedefPort->isA<Input>())
                {
                    GeomPropDefPtr geomprop = nodedefPort->asA<Input>()->getDefaultGeomProp();
                    if (geomprop)
                    {
                        inputSocket->setGeomProp(geomprop->getName());
                        input->setGeomProp(geomprop->getName());
                    }
                }

                // Connect to the graph input
                inputSocket->makeConnection(input);
            }

            // No traversal of upstream dependencies
            root = nullptr;
        }
    }

    else if (element->isA<ShaderRef>())
    {
        ShaderRefPtr shaderRef = element->asA<ShaderRef>();
        NodeDefPtr nodeDef = shaderRef->getNodeDef();
        if (!nodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef for shader '" + shaderRef->getName() + "'");
        }

        graph = std::make_shared<ShaderGraph>(parent, name, element->getDocument(), context.getReservedWords());

        // Create input sockets
        graph->addInputSockets(*nodeDef, context);

        // Create output sockets
        graph->addOutputSockets(*nodeDef);

        // Create this shader node in the graph.
        const string& newNodeName = shaderRef->getName();
        ShaderNodePtr newNode = ShaderNode::create(graph.get(), newNodeName, *nodeDef, context);
        graph->addNode(newNode);

        // Connect it to the graph output
        ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket();
        outputSocket->makeConnection(newNode->getOutput());
        outputSocket->setPath(shaderRef->getNamePath());

        string targetColorSpace;
        ColorManagementSystemPtr colorManagementSystem = context.getShaderGenerator().getColorManagementSystem();
        if (colorManagementSystem)
        {
            targetColorSpace = context.getOptions().targetColorSpaceOverride.empty() ?
                element->getDocument()->getColorSpace() : context.getOptions().targetColorSpaceOverride;
        }

        // Handle node parameters
        const string& targetDistanceUnit = context.getOptions().targetDistanceUnit;
        UnitSystemPtr unitSystem = context.getShaderGenerator().getUnitSystem();
        for (ParameterPtr elem : nodeDef->getActiveParameters())
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
                ValuePtr bindParamValue = bindParam->getResolvedValue();
                if (bindParamValue)
                {
                    inputSocket->setValue(bindParamValue);

                    input->setBindInput();
                    graph->populateColorTransformMap(colorManagementSystem, input, bindParam, targetColorSpace, true);
                    graph->populateUnitTransformMap(unitSystem, input, bindParam, targetDistanceUnit, true);
                }
                inputSocket->setPath(bindParam->getNamePath());
                input->setPath(inputSocket->getPath());
                const string& bindParamUnit = bindParam->getUnit();
                if (!bindParamUnit.empty())
                {
                    inputSocket->setUnit(bindParamUnit);
                    input->setUnit(bindParamUnit);
                }
            }

            // Connect to the graph input
            inputSocket->makeConnection(input);
        }

        // Handle node inputs
        for (const InputPtr& nodeDefInput : nodeDef->getActiveInputs())
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
                ValuePtr bindInputValue = bindInput->getResolvedValue();
                if (bindInputValue)
                {
                    inputSocket->setValue(bindInputValue);

                    input->setBindInput();
                    graph->populateColorTransformMap(colorManagementSystem, input, bindInput, targetColorSpace, true);
                    graph->populateUnitTransformMap(unitSystem, input, bindInput, targetDistanceUnit, true);
                }
                inputSocket->setPath(bindInput->getNamePath());
                input->setPath(inputSocket->getPath());
                const string& bindInputUnit = bindInput->getUnit();
                if (!bindInputUnit.empty())
                {
                    inputSocket->setUnit(bindInputUnit);
                    input->setUnit(bindInputUnit);
                }
            }

            GeomPropDefPtr geomprop = nodeDefInput->getDefaultGeomProp();
            if (geomprop)
            {
                inputSocket->setGeomProp(geomprop->getName());
                input->setGeomProp(geomprop->getName());
            }

            // If no explicit connection, connect to geometric node if a geomprop is used
            // or otherwise to the graph interface.
            const string& connection = bindInput ? bindInput->getOutputString() : EMPTY_STRING;
            if (connection.empty())
            {
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

        // Add shaderRef nodedef paths
        const string& nodePath = shaderRef->getNamePath();
        for (const ValueElementPtr& nodeInput : nodeDef->getActiveInputs())
        {
            const string& inputName = nodeInput->getName();
            const string path = nodePath + NAME_PATH_SEPARATOR + inputName;
            const string& unit = nodeInput->getUnit();
            ShaderInput* input = newNode->getInput(inputName);
            if (input && input->getPath().empty())
            {
                input->setPath(path);
            }
            if (input && input->getUnit().empty() && !unit.empty())
            {
                input->setUnit(unit);
            }
            ShaderGraphInputSocket* inputSocket = graph->getInputSocket(inputName);
            if (inputSocket && inputSocket->getPath().empty())
            {
                inputSocket->setPath(path);
            }
            if (inputSocket && inputSocket->getUnit().empty() && !unit.empty())
            {
                inputSocket->setUnit(unit);
            }
        }
        for (const ParameterPtr& nodeParameter : nodeDef->getActiveParameters())
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

    if (!graph)
    {
        throw ExceptionShaderGenError("Shader generation from element '" + element->getName() + "' of type '" + element->getCategory() + "' is not supported");
    }

    // Traverse and create all dependencies upstream
    if (root)
    {
        graph->addUpstreamDependencies(*root, material, context);
    }

    // Add classification according to root node
    ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket();
    graph->_classification |= outputSocket->getConnection() ? outputSocket->getConnection()->getNode()->_classification : 0;

    graph->finalize(context);

    return graph;
}

ShaderNode* ShaderGraph::createNode(const Node& node, GenContext& context)
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

    // Check if the node is a convolution node and mark the graph as such.
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
    for (const InputPtr& nodeDefInput : nodeDef->getActiveInputs())
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

    string targetColorSpace;
    ColorManagementSystemPtr colorManagementSystem = context.getShaderGenerator().getColorManagementSystem();
    if (colorManagementSystem)
    {
        targetColorSpace = context.getOptions().targetColorSpaceOverride.empty() ?
            _document->getActiveColorSpace() : context.getOptions().targetColorSpaceOverride;
    }

    if (colorManagementSystem && !targetColorSpace.empty())
    {
        for (InputPtr input : node.getInputs())
        {
            ShaderInput* shaderInput = newNode->getInput(input->getName());
            populateColorTransformMap(colorManagementSystem, shaderInput, input, targetColorSpace, true);
        }
        for (ParameterPtr parameter : node.getParameters())
        {
            if (parameter->getType() == FILENAME_TYPE_STRING)
            {
                ShaderOutput* shaderOutput = newNode->getOutput();
                if (shaderOutput)
                {
                    populateColorTransformMap(colorManagementSystem, shaderOutput, parameter, targetColorSpace, false);
                }
            }
            else
            {
                ShaderInput* shaderInput = newNode->getInput(parameter->getName());
                populateColorTransformMap(colorManagementSystem, shaderInput, parameter, targetColorSpace, true);
            }
        }
    }

    // Unit Conversion: Only applicable if Unit system and a "targetDistanceUnit" is defined for now.
    UnitSystemPtr unitSystem = context.getShaderGenerator().getUnitSystem();
    const string& targetDistanceUnit = context.getOptions().targetDistanceUnit;
    if (unitSystem && !targetDistanceUnit.empty())
    {
        for (InputPtr input : node.getInputs())
        {
            ShaderInput* inputPort = newNode->getInput(input->getName());
            populateUnitTransformMap(unitSystem, inputPort, input, targetDistanceUnit, true);
        }
        for (ParameterPtr parameter : node.getParameters())
        {
            ShaderInput* inputPort = newNode->getInput(parameter->getName());
            if (parameter->getType() == FILENAME_TYPE_STRING)
            {
                // Check output port for any parameters which are file names
                ShaderOutput* shaderOutput = newNode->getOutput();
                if (shaderOutput)
                {
                    populateUnitTransformMap(unitSystem, shaderOutput, parameter, targetDistanceUnit, false);
                }
            }
            populateUnitTransformMap(unitSystem, inputPort, parameter, targetDistanceUnit, true);
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

void ShaderGraph::addNode(ShaderNodePtr node)
{
    _nodeMap[node->getName()] = node;
    _nodeOrder.push_back(node.get());
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
    for (const auto& it : _inputColorTransformMap)
    {
        addColorTransformNode(it.first, it.second, context);
    }
    for (const auto& it : _outputColorTransformMap)
    {
        addColorTransformNode(it.first, it.second, context);
    }
    _inputColorTransformMap.clear();
    _outputColorTransformMap.clear();

    // Insert unit transformation nodes where needed
    for (const auto& it : _inputUnitTransformMap)
    {
        addUnitTransformNode(it.first, it.second, context);
    }
    for (const auto& it : _outputUnitTransformMap)
    {
        addUnitTransformNode(it.first, it.second, context);
    }
    _inputUnitTransformMap.clear();
    _outputUnitTransformMap.clear();

    // Optimize the graph, removing redundant paths.
    optimize(context);

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
                            inputSocket->setUnit(input->getUnit());
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
    setVariableNames(context);

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
        output->breakConnections();
    }
}

void ShaderGraph::optimize(GenContext& context)
{
    size_t numEdits = 0;
    for (ShaderNode* node : getNodes())
    {
        if (node->hasClassification(ShaderNode::Classification::CONSTANT))
        {
            // Constant nodes can be removed by assigning their value downstream
            // But don't remove it if it's connected upstream, i.e. it's value
            // input is published.
            ShaderInput* valueInput = node->getInput(0);
            if (!valueInput->getConnection())
            {
                bypass(context, node, 0);
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
                bypass(context, node, branch);

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
                bypass(context, node, branch);

                ++numEdits;
            }
        }
    }

    if (numEdits > 0)
    {
        std::set<ShaderNode*> usedNodes;

        // Traverse the graph to find nodes still in use
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

void ShaderGraph::bypass(GenContext& context, ShaderNode* node, size_t inputIndex, size_t outputIndex)
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
            const string& inputUnit = input->getUnit();
            if (!inputUnit.empty())
            {
                downstream->setUnit(inputUnit);
            }

            // Swizzle the input value. Once done clear the channel to indicate
            // no further swizzling is reqiured.
            const string& channels = downstream->getChannels();
            if (!channels.empty())
            {
                downstream->setValue(context.getShaderGenerator().getSyntax().getSwizzledValue(input->getValue(),
                                                                                          input->getType(), 
                                                                                          channels,
                                                                                          downstream->getType()));
                downstream->setType(downstream->getType());
                downstream->setChannels(EMPTY_STRING);
            }
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
    for (const auto& it : _nodeMap)
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
        for (const auto& output : node->getOutputs())
        {
            for (const auto& input : output->getConnections())
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

void ShaderGraph::setVariableNames(GenContext& context)
{
    // Make sure inputs and outputs have variable names valid for the
    // target shading language, and are unique to avoid name conflicts.

    const Syntax& syntax = context.getShaderGenerator().getSyntax();

    for (ShaderGraphInputSocket* inputSocket : getInputSockets())
    {
        const string variable = syntax.getVariableName(inputSocket->getName(), inputSocket->getType(), _identifiers);
        inputSocket->setVariable(variable);
    }
    for (ShaderGraphOutputSocket* outputSocket : getOutputSockets())
    {
        const string variable = syntax.getVariableName(outputSocket->getName(), outputSocket->getType(), _identifiers);
        outputSocket->setVariable(variable);
    }
    for (ShaderNode* node : getNodes())
    {
        for (ShaderInput* input : node->getInputs())
        {
            string variable = input->getFullName();
            variable = syntax.getVariableName(variable, input->getType(), _identifiers);
            input->setVariable(variable);
        }
        for (ShaderOutput* output : node->getOutputs())
        {
            // Node outputs use long names for better code readability
            string variable = output->getFullName();
            variable = syntax.getVariableName(variable, output->getType(), _identifiers);
            output->setVariable(variable);
        }
    }
}

void ShaderGraph::populateColorTransformMap(ColorManagementSystemPtr colorManagementSystem, ShaderPort* shaderPort, 
                                            ValueElementPtr input, const string& targetColorSpace, bool asInput)
{
    if (!colorManagementSystem)
    {
        return;
    }

    const string& sourceColorSpace = input->getActiveColorSpace();
    if (shaderPort && !sourceColorSpace.empty())
    {
        if(shaderPort->getType() == Type::COLOR3 || shaderPort->getType() == Type::COLOR4)
        {
            // If we're converting between two identical color spaces than we have no work to do.
            if (sourceColorSpace != targetColorSpace)
            {
                ColorSpaceTransform transform(sourceColorSpace, targetColorSpace, shaderPort->getType());
                if (colorManagementSystem->supportsTransform(transform))
                {
                    if (asInput)
                    {
                        _inputColorTransformMap.emplace(static_cast<ShaderInput*>(shaderPort), transform);
                    }
                    else
                    {
                        _outputColorTransformMap.emplace(static_cast<ShaderOutput*>(shaderPort), transform);
                    }
                }
            }
        }
    }
}

void ShaderGraph::populateUnitTransformMap(UnitSystemPtr unitSystem, ShaderPort* shaderPort, ValueElementPtr input, const string& globalTargetUnitSpace, bool asInput)
{
    const string& sourceUnitSpace = input->getUnit();
    if (!shaderPort || sourceUnitSpace.empty())
    {
        return;
    }

    const string& unitType = input->getUnitType();
    if (!input->getDocument()->getUnitTypeDef(unitType))
    {
        return;
    }

    string targetUnitSpace = input->getActiveUnit();
    if (targetUnitSpace.empty())
    {
        targetUnitSpace = globalTargetUnitSpace;
    }

    // Don't perform unit conversion if targetUnitSpace is unspecified.
    if (targetUnitSpace.empty())
    {
        return;
    }

    // TODO: Consider this to be an optimization option as
    // this allows for the source and target unit to be the same value
    // while still allowing target unit updates on a compiled shader as the
    // target is exposed as an input uniform.
    //if (sourceUnitSpace == targetUnitSpace)
    //{
    //    return;
    //}

    // Only support convertion for float and vectors. arrays, matrices are not supported.
    // TODO: This should be provided by the UnitSystem.
    bool supportedType = (shaderPort->getType() == Type::FLOAT ||
                        shaderPort->getType() == Type::VECTOR2 ||
                        shaderPort->getType() == Type::VECTOR3 ||
                        shaderPort->getType() == Type::VECTOR4);
    if (supportedType)
    {
        UnitTransform transform(sourceUnitSpace, targetUnitSpace, shaderPort->getType(), unitType);
        if (unitSystem->supportsTransform(transform))
        {
            shaderPort->setUnit(sourceUnitSpace);
            if (asInput)
            {
                _inputUnitTransformMap.emplace(static_cast<ShaderInput*>(shaderPort), transform);
            }
            else
            {
                _outputUnitTransformMap.emplace(static_cast<ShaderOutput*>(shaderPort), transform);
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
        throw ExceptionFoundCycle("Encountered cycle at element: " + upstream->getFullName());
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
