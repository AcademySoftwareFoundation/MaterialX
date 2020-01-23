//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtFileIo.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtPortDef.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtAttribute.h>

#include <MaterialXRuntime/Private/PvtStage.h>
#include <MaterialXRuntime/Private/PvtNodeDef.h>
#include <MaterialXRuntime/Private/PvtNode.h>
#include <MaterialXRuntime/Private/PvtNodeGraph.h>
#include <MaterialXRuntime/Private/PvtTraversal.h>

#include <MaterialXCore/Types.h>

#include <MaterialXGenShader/Util.h>
#include <sstream>

namespace MaterialX
{

namespace
{
    // Lists of known attributes which are handled explicitly by import/export.
    static const RtTokenSet nodedefAttrs    = { "name", "type", "node" };
    static const RtTokenSet portdefAttrs    = { "name", "type", "value", "nodename", "output", "colorspace", "unit" };
    static const RtTokenSet nodeAttrs       = { "name", "type", "node" };
    static const RtTokenSet nodegraphAttrs  = { "name", "nodedef" };
    static const RtTokenSet unknownAttrs    = { "name" };

    PvtNode* findNodeOrThrow(const RtToken& name, PvtElement* parent)
    {
        PvtDataHandle nodeH = parent->findChildByName(name);
        if (!nodeH)
        {
            throw ExceptionRuntimeError("Node '" + name.str() + "' is not a child of '" + parent->getName().str() + "'");
        }
        return nodeH->asA<PvtNode>();
    }

    RtPort findPortOrThrow(const RtToken& name, PvtNode* node)
    {
        RtPort port = node->findPort(name);
        if (!port)
        {
            throw ExceptionRuntimeError("Node '" + node->getName().str() + "' has no port named '" + name.str() + "'");
        }
        return port;
    }

    void createNodeConnections(const vector<NodePtr>& nodeElements, PvtElement* parent)
    {
        for (const NodePtr& nodeElem : nodeElements)
        {
            PvtNode* node = findNodeOrThrow(RtToken(nodeElem->getName()), parent);
            for (const InputPtr& elemInput : nodeElem->getInputs())
            {
                RtPort input = findPortOrThrow(RtToken(elemInput->getName()), node);
                const string& connectedNodeName = elemInput->getNodeName();
                if (!connectedNodeName.empty())
                {
                    PvtNode* connectedNode = findNodeOrThrow(RtToken(connectedNodeName), parent);
                    const RtToken outputName(elemInput->getOutputString());
                    RtPort output = findPortOrThrow(outputName != EMPTY_TOKEN ? outputName : PvtPortDef::DEFAULT_OUTPUT_NAME, connectedNode);
                    RtNode::connect(output, input);
                }
            }
        }
    }

    void readCustomAttributes(const ElementPtr src, PvtElement* dest, const RtTokenSet& knownAttrs)
    {
        // Read in all custom attributes so we can export the element again
        // without loosing data.
        for (const string& name : src->getAttributeNames())
        {
            if (!knownAttrs.count(RtToken(name)))
            {
                // Store all custom attributes as string tokens.
                const RtToken attrName(name);
                const RtToken attrValue(src->getAttribute(name));
                RtAttribute* attr = dest->addAttribute(attrName, RtType::TOKEN, RtAttrFlag::CUSTOM);
                attr->getValue().asToken() = attrValue;
            }
        }
    }

    void writeAttributes(const PvtElement* elem, ElementPtr dest)
    {
        for (size_t i = 0; i < elem->numAttributes(); ++i)
        {
            const RtAttribute* attr = elem->getAttribute(i);
            dest->setAttribute(attr->getName(), attr->getValueString());
        }
    }

    PvtDataHandle readNodeDef(const NodeDefPtr& src, PvtStage* stage)
    {
        const RtToken name(src->getName());
        const RtToken nodeName(src->getNodeString());

        PvtDataHandle nodedefH = PvtNodeDef::createNew(stage, name, nodeName);
        PvtNodeDef* nodedef = nodedefH->asA<PvtNodeDef>();

        readCustomAttributes(src, nodedef, nodedefAttrs);

        // Add outputs
        if (src->getOutputCount() > 0)
        {
            for (auto elem : src->getOutputs())
            {
                const RtToken portName(elem->getName());
                const RtToken portType(elem->getType());
                PvtDataHandle outputH = PvtPortDef::createNew(nodedef, portName, portType, RtPortFlag::OUTPUT);
                PvtPortDef* output = outputH->asA<PvtPortDef>();
                readCustomAttributes(elem, output, portdefAttrs);
            }
        }
        else
        {
            const RtToken type(src->getType());
            PvtPortDef::createNew(nodedef, PvtPortDef::DEFAULT_OUTPUT_NAME, type, RtPortFlag::OUTPUT);
        }

        // Add inputs
        for (auto elem : src->getChildrenOfType<ValueElement>())
        {
            if (elem->isA<Input>())
            {
                const RtToken portName(elem->getName());
                const RtToken portType(elem->getType());
                const string& valueStr = elem->getValueString();

                PvtDataHandle inputH = PvtPortDef::createNew(nodedef, portName, portType);
                PvtPortDef* input = inputH->asA<PvtPortDef>();
                if (!valueStr.empty())
                {
                    RtValue::fromString(portType, valueStr, input->getValue());
                }
                input->setColorSpace(RtToken(elem->getColorSpace()));
                // TODO: fix when units are implemented in core
                // input->setUnit(RtToken(elem->getUnit()));
                readCustomAttributes(elem, input, portdefAttrs);
            }
            else if (elem->isA<Parameter>())
            {
                const RtToken portName(elem->getName());
                const RtToken portType(elem->getType());
                const string& valueStr = elem->getValueString();

                PvtDataHandle inputH = PvtPortDef::createNew(nodedef, portName, portType, RtPortFlag::UNIFORM);
                PvtPortDef* input = inputH->asA<PvtPortDef>();
                if (!valueStr.empty())
                {
                    RtValue::fromString(portType, valueStr, input->getValue());
                }
                input->setColorSpace(RtToken(elem->getColorSpace()));
                // TODO: fix when units are implemented in core
                // input->setUnit(RtToken(elem->getUnit()));
                readCustomAttributes(elem, input, portdefAttrs);
            }
        }

        return nodedefH;
    }

    bool matchingSignature(const PvtNodeDef* nodedef, const RtToken nodeType, const vector<ValueElementPtr>& nodePorts)
    {
        // Check output types for single output nodes.
        if (nodedef->numOutputs() == 1 && nodedef->getOutput(0)->getType() != nodeType)
        {
            return false;
        }
        // Check all other ports.
        for (const ValueElementPtr& nodePort : nodePorts)
        {
            const PvtPortDef* nodedefPort = nodedef->findPort(RtToken(nodePort->getName()));
            if (!nodedefPort || nodedefPort->getType().str() != nodePort->getType())
            {
                return false;
            }
        }
        return true;
    }

    PvtDataHandle resolveNodeDef(const NodePtr& node, PvtStage* stage)
    {
        PvtDataHandle nodedefH;

        // First try resolving a nodedef from content in the current document.
        NodeDefPtr srcNodedef = node->getNodeDef();
        if (srcNodedef)
        {
            const RtToken nodedefName(srcNodedef->getName());
            nodedefH = stage->findChildByName(nodedefName);
            if (!nodedefH)
            {
                // NodeDef is not loaded yet so create it now.
                nodedefH = readNodeDef(srcNodedef, stage);
            }
        }

        if (!nodedefH)
        {
            // Try resolving among existing nodedefs in the stage
            // by matching signatures.

            const RtToken nodeName(node->getCategory());
            const RtToken nodeType(node->getType());
            const vector<ValueElementPtr> nodePorts = node->getActiveValueElements();

            RtObjectFilter<RtObjType::NODEDEF> nodedefFilter;
            PvtStageIterator it(stage->shared_from_this(), nodedefFilter);
            for (; !it.isDone(); ++it)
            {
                const PvtNodeDef* nodedef = (*it)->asA<PvtNodeDef>();
                if (nodedef->getNodeName() == nodeName && 
                    matchingSignature(nodedef, nodeType, nodePorts))
                {
                    return *it;
                }
            }
        }

        return nodedefH;
    }

    PvtDataHandle readNode(const NodePtr& src, PvtElement* parent, PvtStage* stage)
    {
        PvtDataHandle nodedefH = resolveNodeDef(src, stage);
        if (!nodedefH)
        {
            throw ExceptionRuntimeError("No matching nodedef was found for node '" + src->getName() + "'");
        }

        const RtToken nodeName(src->getName());
        PvtDataHandle nodeH = PvtNode::createNew(parent, nodedefH, nodeName);
        PvtNode* node = nodeH->asA<PvtNode>();

        readCustomAttributes(src, node, nodeAttrs);

        // Copy input values.
        for (auto elem : src->getChildrenOfType<ValueElement>())
        {
            const RtToken portName(elem->getName());
            RtPort port = node->findPort(portName);
            if (!port)
            {
                throw ExceptionRuntimeError("No port named '" + elem->getName() + "' was found on runtime node '" + node->getName().str() + "'");
            }
            const string& valueStr = elem->getValueString();
            if (!valueStr.empty())
            {
                const RtToken portType(elem->getType());
                RtValue::fromString(portType, valueStr, port.getValue());
            }
        }

        return nodeH;
    }

    PvtDataHandle readNodeGraph(const NodeGraphPtr& src, PvtElement* parent, PvtStage* stage)
    {
        const RtToken nodegraphName(src->getName());
        PvtDataHandle nodegraphH = PvtNodeGraph::createNew(parent, nodegraphName);
        PvtNodeGraph* nodegraph = nodegraphH->asA<PvtNodeGraph>();

        readCustomAttributes(src, nodegraph, nodegraphAttrs);

        bool fixedInterface = false;
        NodeDefPtr srcNodeDef = src->getNodeDef();
        if (srcNodeDef)
        {
            fixedInterface = true;

            vector<ValueElementPtr> elements = srcNodeDef->getChildrenOfType<ValueElement>();

            // Create outputs first
            for (auto elem : elements)
            {
                if (elem->isA<Output>())
                {
                    const RtToken name(elem->getName());
                    const RtToken type(elem->getType());
                    PvtPortDef::createNew(nodegraph, name, type, RtPortFlag::OUTPUT);
                }
            }
            // Create inputs second
            for (auto elem : elements)
            {
                if (!elem->isA<Output>())
                {
                    const RtToken name(elem->getName());
                    const RtToken type(elem->getType());
                    const uint32_t flags = elem->isA<Parameter>() ? RtPortFlag::UNIFORM : 0;
                    PvtPortDef::createNew(nodegraph, name, type, flags);
                }
            }
        }
        else
        {
            // No nodedef interface was set on the graph.
            // Create all outputs here, inputs are created
            // from internal interface connections below.
            for (auto elem : src->getOutputs())
            {
                const RtToken name(elem->getName());
                const RtToken type(elem->getType());
                PvtPortDef::createNew(nodegraph, name, type, RtPortFlag::OUTPUT);
            }
        }

        // Create all nodes.
        for (auto child : src->getChildren())
        {
            NodePtr srcNnode = child->asA<Node>();
            if (srcNnode)
            {
                PvtDataHandle nodeH = readNode(srcNnode, nodegraph, stage);
                PvtNode* node = nodeH->asA<PvtNode>();

                // Check for connections to the graph interface
                for (auto elem : srcNnode->getChildrenOfType<ValueElement>())
                {
                    const string& interfaceName = elem->getInterfaceName();
                    if (!interfaceName.empty())
                    {
                        const RtToken internalInputName(interfaceName);
                        RtPort inputSocket = nodegraph->findInputSocket(internalInputName);
                        if (!inputSocket)
                        {
                            // Create the input on the graph
                            if (fixedInterface)
                            {
                                // The input should have been created already.
                                // This is an error so throw up.
                                throw ExceptionRuntimeError("Interface name '" + interfaceName + "' does not match an input on the nodedef set for nodegraph '" +
                                    nodegraph->getName().str() + "'");
                            }
                            const RtToken portType(elem->getType());
                            PvtDataHandle portdefH = PvtPortDef::createNew(nodegraph, internalInputName, portType);
                            PvtPortDef* portdef = portdefH->asA<PvtPortDef>();

                            const string& valueStr = elem->getValueString();
                            if (!valueStr.empty())
                            {
                                RtValue::fromString(portType, valueStr, portdef->getValue());
                            }

                            inputSocket = nodegraph->findInputSocket(portdef->getName());
                        }
                        const RtToken inputName(elem->getName());
                        RtPort input = node->findPort(inputName);
                        PvtNode::connect(inputSocket, input);
                    }
                }
            }
        }

        // Create connections between all nodes.
        createNodeConnections(src->getNodes(), nodegraph);

        // Create connections between nodes and the graph outputs.
        for (const OutputPtr& elem : src->getOutputs())
        {
            const string& connectedNodeName = elem->getNodeName();
            if (!connectedNodeName.empty())
            {
                RtPort outputSocket = nodegraph->findOutputSocket(RtToken(elem->getName()));
                PvtNode* connectedNode = findNodeOrThrow(RtToken(connectedNodeName), nodegraph);
                const RtToken outputName(elem->getOutputString());
                RtPort output = findPortOrThrow(outputName != EMPTY_TOKEN ? outputName : PvtPortDef::DEFAULT_OUTPUT_NAME, connectedNode);
                RtNode::connect(output, outputSocket);
            }
        }

        return nodegraphH;
    }

    PvtDataHandle readUnknown(const ElementPtr& src, PvtElement* parent)
    {
        const RtToken name(src->getName());
        const RtToken category(src->getCategory());

        PvtDataHandle elemH = PvtUnknownElement::createNew(parent, name, category);
        PvtUnknownElement* elem = elemH->asA<PvtUnknownElement>();

        readCustomAttributes(src, elem, unknownAttrs);

        for (auto child : src->getChildren())
        {
            readUnknown(child, elem);
        }

        return elemH;
    }

    void readSourceUri(const DocumentPtr& doc, PvtStage* stage)
    {
        StringSet uris = doc->getReferencedSourceUris();
        for (const string& uri : uris)
        {
            stage->addSourceUri(RtToken(uri));
        }
    }

    void readDocument(const DocumentPtr& doc, PvtStage* stage, const RtReadOptions* readOptions)
    {
        // Set the source location 
        const std::string& uri = doc->getSourceUri();
        stage->addSourceUri(RtToken(uri));
        readCustomAttributes(doc, stage, {});

        RtReadOptions::ReadFilter filter = readOptions ? readOptions->readFilter : nullptr;

        // First, load all nodedefs. Having these available is needed
        // when node instances are loaded later.
        for (const NodeDefPtr& nodedef : doc->getNodeDefs())
        {
            if (!filter || filter(nodedef))
            {
                if (!stage->findChildByName(RtToken(nodedef->getName())))
                {
                    readNodeDef(nodedef, stage);
                }
            }
        }

        // Load all other elements.
        for (const ElementPtr& elem : doc->getChildren())
        {
            if (!filter || filter(elem))
            {
                // Make sure the element has not been loaded already.
                if (stage->findChildByName(RtToken(elem->getName())))
                {
                    continue;
                }

                if (elem->isA<Node>())
                {
                    readNode(elem->asA<Node>(), stage, stage);
                }
                else if (elem->isA<NodeGraph>())
                {
                    readNodeGraph(elem->asA<NodeGraph>(), stage, stage);
                }
                else
                {
                    readUnknown(elem, stage);
                }
            }
        }

        // Create connections between all root level nodes.
        createNodeConnections(doc->getNodes(), stage);
    }

    void writeNodeDef(const PvtNodeDef* nodedef, DocumentPtr dest)
    {
        const size_t numPorts = nodedef->numPorts();
        const size_t numOutputs = nodedef->numOutputs();

        NodeDefPtr destNodeDef = dest->addNodeDef(nodedef->getName(), EMPTY_STRING, nodedef->getNodeName());
        writeAttributes(nodedef, destNodeDef);

        for (size_t i = numOutputs; i < numPorts; ++i)
        {
            const PvtPortDef* input = nodedef->getPort(i);

            ValueElementPtr destInput;
            if (input->isUniform())
            {
                destInput = destNodeDef->addParameter(input->getName(), input->getType().str());
            }
            else
            {
                destInput = destNodeDef->addInput(input->getName(), input->getType().str());
            }

            destInput->setValueString(input->getValueString());

            if (input->getColorSpace())
            {
                destInput->setColorSpace(input->getColorSpace().str());
            }
            if (input->getUnit())
            {
                // TODO: fix when units are implemented in core.
                // destInput->setUnit(input->getUnit().str());
            }

            writeAttributes(input, destInput);
        }
        for (size_t i = 0; i < numOutputs; ++i)
        {
            const PvtPortDef* output = nodedef->getPort(i);
            OutputPtr destOutput = destNodeDef->addOutput(output->getName(), output->getType().str());
            writeAttributes(output, destOutput);
        }
    }

    template<typename T>
    NodePtr writeNode(const PvtNode* node, T dest)
    {
        const PvtNodeDef* nodedef = node->getNodeDef()->asA<PvtNodeDef>();

        const string type = nodedef->numOutputs() == 1 ? nodedef->getPort(0)->getType().str() : "multioutput";
        NodePtr destNode = dest->addNode(nodedef->getNodeName(), node->getName().str(), type);

        for (size_t i = 0; i < nodedef->numInputs(); ++i)
        {
            const PvtPortDef* inputDef = nodedef->getInput(i);
            RtPort input = const_cast<PvtNode*>(node)->findPort(inputDef->getName());

            if (input.isConnected() || !RtValue::compare(inputDef->getType(), inputDef->getValue(), input.getValue()))
            {
                ValueElementPtr valueElem;
                if (inputDef->isUniform())
                {
                    valueElem = destNode->addParameter(input.getName().str(), input.getType().str());
                    if (input.isConnected())
                    {
                        RtPort sourcePort = input.getSourcePort();
                        if (sourcePort.isSocket())
                        {
                            // This is a connection to the internal node of a graph
                            valueElem->setInterfaceName(sourcePort.getName());
                        }
                    }
                    else
                    {
                        valueElem->setValueString(input.getValueString());
                    }
                }
                else
                {
                    valueElem = destNode->addInput(input.getName().str(), input.getType().str());
                    if (input.isConnected())
                    {
                        RtPort sourcePort = input.getSourcePort();
                        if (sourcePort.isSocket())
                        {
                            // This is a connection to the internal node of a graph
                            valueElem->setInterfaceName(sourcePort.getName());
                        }
                        else
                        {
                            PvtNode* sourceNode = sourcePort.data()->asA<PvtNode>();
                            InputPtr inputElem = valueElem->asA<Input>();
                            inputElem->setNodeName(sourceNode->getName());
                            if (sourceNode->numOutputs() > 1 || sourceNode->hasApi(RtApiType::NODEGRAPH))
                            {
                                inputElem->setOutputString(sourcePort.getName());
                            }
                        }
                    }
                    else
                    {
                        valueElem->setValueString(input.getValueString());
                    }
                }

                if (input.getColorSpace())
                {
                    valueElem->setColorSpace(input.getColorSpace().str());
                }
                if (input.getUnit())
                {
                    // TODO: fix when units are implemented in core.
                    // valueElem->setUnit(input->getUnit().str());
                }
            }
        }

        // Write outputs.
        // TODO: Decide if ALL outputs should be written explicitly for nodes (same as for nodegraphs).
        if (nodedef->numOutputs() > 1)
        {
            for (size_t i = 0; i < nodedef->numOutputs(); ++i)
            {
                const PvtPortDef* output = nodedef->getOutput(i);
                OutputPtr destOutput = destNode->addOutput(output->getName(), output->getType().str());
            }
        }

        writeAttributes(node, destNode);

        return destNode;
    }

    void createMaterialNode(PvtNode* node, NodePtr mxNode, DocumentPtr doc)
    {
        // Check to see if the surfaceshader node is already connected to a material node
        size_t numOutputs = node->numOutputs();
        bool isConnectedToMaterialNode = false;
        for (size_t i=0; i<numOutputs; ++i)
        {
            RtPort outputPort = node->getPort(node->getOutputsOffset() + i);
            size_t numDestinationPorts = outputPort.numDestinationPorts();
            for (size_t j=0; j<numDestinationPorts; ++j)
            {
                RtNode connectedNode(outputPort.getDestinationPort(j).getNode());
                if (connectedNode.numOutputs() > 0 &&
                    connectedNode.getOutput(0).getType() == MATERIAL_TYPE_STRING )
                {
                    isConnectedToMaterialNode = true;
                    break;
                }
            }
        }
        
        if (!isConnectedToMaterialNode)
        {
            NodePtr materialNode = doc->addNode(SURFACE_MATERIAL_NODE_STRING, mxNode->getName() + "_SurfaceMaterial", MATERIAL_TYPE_STRING );
            materialNode->setConnectedNode(SURFACE_SHADER_TYPE_STRING, mxNode);
        }
    }

    void writeMaterialElementsHelper(PvtNode* node, NodePtr mxNode, const string& materialBaseName, const string& nodeName, DocumentPtr doc, const RtWriteOptions* writeOptions)
    {
        MaterialPtr material = doc->addMaterial(materialBaseName + "_Material");
        ShaderRefPtr shaderRef = material->addShaderRef("sref", nodeName);
        for (InputPtr input : mxNode->getActiveInputs())
        {
            BindInputPtr bindInput = shaderRef->addBindInput(input->getName(), input->getType());
            if (input->hasNodeName())
            {
                if (input->hasOutputString())
                {
                    bindInput->setNodeGraphString(input->getNodeName());
                    bindInput->setOutputString(input->getOutputString());
                }
            }
            else
            {
                bindInput->setValueString(input->getValueString());
            }
        }
        for (ParameterPtr param : mxNode->getActiveParameters())
        {
            BindParamPtr bindParam = shaderRef->addBindParam(param->getName(), param->getType());
            bindParam->setValueString(param->getValueString());
        }
        // Should we delete the surface shader?
        if (writeOptions->materialWriteOp & RtWriteOptions::MaterialWriteOp::DELETE)
        {
            doc->removeChild(node->getName());
        }
        // Should we create a look for the material element?
        if (writeOptions->materialWriteOp & RtWriteOptions::MaterialWriteOp::LOOK)
        {
            LookPtr look = doc->addLook();
            MaterialAssignPtr materialAssign = look->addMaterialAssign();
            materialAssign->setMaterial(material->getName());
            CollectionPtr collection = doc->addCollection();
            collection->setIncludeGeom("/*");
            materialAssign->setCollection(collection);
        }
    }

    void writeMaterialElements(PvtNode* node, NodePtr mxNode, const string& nodeName, DocumentPtr doc, const RtWriteOptions* writeOptions)
    {
        if (writeOptions->materialWriteOp & RtWriteOptions::MaterialWriteOp::ADD_MATERIAL_NODES_FOR_SHADERS)
        {
            // Get the connected material nodes and create material elements from them (using their names)
            size_t numOutputs = node->numOutputs();
            for (size_t i=0; i<numOutputs; ++i)
            {
                RtPort outputPort = node->getPort(node->getOutputsOffset() + i);
                size_t numDestinationPorts = outputPort.numDestinationPorts();
                for (size_t j = 0; j < numDestinationPorts; ++j)
                {
                    RtNode connectedNode(outputPort.getDestinationPort(j).getNode());
                    if (connectedNode.numOutputs() > 0 && connectedNode.getOutput(0).getType() == MATERIAL_TYPE_STRING )
                    {
                        writeMaterialElementsHelper(node, mxNode, connectedNode.getName(), nodeName, doc, writeOptions);
                    }
                }
            }
        }
        else
        {
            writeMaterialElementsHelper(node, mxNode, mxNode->getName(), nodeName, doc, writeOptions);
        }
    }

    void writeNodeGraph(const PvtNodeGraph* nodegraph, DocumentPtr dest)
    {
        NodeGraphPtr destNodeGraph = dest->addNodeGraph(nodegraph->getName());
        writeAttributes(nodegraph, destNodeGraph);

        // Write nodes.
        for (auto node : nodegraph->getChildren())
        {
            writeNode(node->asA<PvtNode>(), destNodeGraph);
        }

        // Write outputs.
        for (size_t i=0; i<nodegraph->numOutputs(); ++i)
        {
            const RtPort nodegraphOutput = nodegraph->getOutputSocket(i);
            OutputPtr output = destNodeGraph->addOutput(nodegraphOutput.getName(), nodegraphOutput.getType().str());

            if (nodegraphOutput.isConnected())
            {
                const RtPort sourcePort = nodegraphOutput.getSourcePort();
                if (sourcePort.isSocket())
                {
                    output->setInterfaceName(sourcePort.getName());
                }
                else
                {
                    const PvtNode* sourceNode = sourcePort.data()->asA<PvtNode>();
                    output->setNodeName(sourceNode->getName());
                    if (sourceNode->numOutputs() > 1)
                    {
                        output->setOutputString(sourcePort.getName());
                    }
                }
            }
        }
    }

    void writeUnknown(const PvtUnknownElement* unknown, ElementPtr dest)
    {
        ElementPtr unknownElem = dest->addChildOfCategory(unknown->getCategory(), unknown->getName());
        writeAttributes(unknown, unknownElem);

        for (auto child : unknown->getChildren())
        {
            writeUnknown(child->asA<PvtUnknownElement>(), unknownElem);
        }
    }

    void writeSourceUris(const PvtStage* stage, DocumentPtr doc)
    {
        const PvtDataHandleVec& refs = stage->getReferencedStages();
        for (size_t i = 0; i < refs.size(); i++)
        {
            const PvtStage* pStage = refs[i]->asA<PvtStage>();
            if (pStage->numReferences())
            {
                writeSourceUris(pStage, doc);
            }
            const RtTokenList& uris = pStage->getSourceUri();
            if (!uris.empty())
            {
                for (const RtToken& uri : uris)
                {
                    prependXInclude(doc, uri.str());
                }
            }
        }
    }

    void writeDocument(DocumentPtr& doc, PvtStage* stage, const RtWriteOptions* writeOptions)
    {
        writeAttributes(stage, doc);

        // Write out any dependent includes
        if (writeOptions && writeOptions->writeIncludes)
        {
            writeSourceUris(stage, doc);
        }

        RtWriteOptions::WriteFilter filter = writeOptions ? writeOptions->writeFilter : nullptr;
        for (size_t i = 0; i < stage->numChildren(); ++i)
        {
            PvtDataHandle elem = stage->getChild(i);
            if (!filter || filter(PvtObject::object(elem)))
            {
                if (elem->getObjType() == RtObjType::NODEDEF)
                {
                    writeNodeDef(elem->asA<PvtNodeDef>(), doc);
                }
                else if (elem->getObjType() == RtObjType::NODE)
                {
                    PvtNode* node = elem->asA<PvtNode>();
                    NodePtr mxNode = writeNode(node, doc);
                    const PvtNodeDef* nodedef = node->getNodeDef()->asA<PvtNodeDef>();
                    if (writeOptions && writeOptions->materialWriteOp != RtWriteOptions::MaterialWriteOp::NONE &&
                        nodedef->numOutputs() == 1 && nodedef->getPort(0)->getType() == SURFACE_SHADER_TYPE_STRING)
                    {
                        if (writeOptions->materialWriteOp & RtWriteOptions::MaterialWriteOp::ADD_MATERIAL_NODES_FOR_SHADERS)
                        {
                            createMaterialNode(node, mxNode, doc);
                        }
                        if (writeOptions->materialWriteOp & RtWriteOptions::MaterialWriteOp::WRITE_MATERIALS_AS_ELEMENTS)
                        {
                            writeMaterialElements(node, mxNode, nodedef->getNodeName(), doc, writeOptions);
                        }
                    }
                }
                else if (elem->getObjType() == RtObjType::NODEGRAPH)
                {
                    writeNodeGraph(elem->asA<PvtNodeGraph>(), doc);
                }
                else if (elem->getObjType() == RtObjType::UNKNOWN)
                {
                    writeUnknown(elem->asA<PvtUnknownElement>(), doc->asA<Element>());
                }
                else
                {
                    std::stringstream str;
                    str << "Exporting object type '" << size_t(elem->getObjType()) << "' is not supported";
                    throw ExceptionRuntimeError(str.str());
                }
            }
        }
    } 

} // end anonymous namespace

RtFileIo::RtFileIo(RtObject stage) :
    RtApiBase(stage)
{
}

RtApiType RtFileIo::getApiType() const
{
    return RtApiType::CORE_IO;
}


void RtFileIo::read(const FilePath& documentPath, const FileSearchPath& searchPaths, const RtReadOptions* readOptions)
{
    try
    {
        DocumentPtr document = createDocument();
        XmlReadOptions xmlReadOptions;
        xmlReadOptions.skipConflictingElements = true;
        if (readOptions)
        {
            xmlReadOptions.skipConflictingElements = readOptions->skipConflictingElements;
        }
        readFromXmlFile(document, documentPath, searchPaths, &xmlReadOptions);

        PvtStage* stage = data()->asA<PvtStage>();
        readDocument(document, stage, readOptions);
    }
    catch (Exception& e)
    {
        throw ExceptionRuntimeError("Could not read file: " + documentPath.asString() + ". Error: " + e.what());
    }
}
void RtFileIo::read(std::istream& stream, const RtReadOptions* readOptions)
{
    try
    {
        DocumentPtr document = createDocument();
        XmlReadOptions xmlReadOptions;
        xmlReadOptions.skipConflictingElements = true;
        if (readOptions)
        {
            xmlReadOptions.skipConflictingElements = readOptions->skipConflictingElements;
        }
        readFromXmlStream(document, stream, &xmlReadOptions);

        PvtStage* stage = data()->asA<PvtStage>();
        readDocument(document, stage, readOptions);
    }
    catch (Exception& e)
    {
        throw ExceptionRuntimeError(string("Could not read from stream. Error: ") + e.what());
    }
}

void RtFileIo::readLibraries(const StringVec& libraryPaths, const FileSearchPath& searchPaths)
{
    PvtStage* stage = data()->asA<PvtStage>();

    // Load all content into a document.
    DocumentPtr doc = createDocument();
    MaterialX::loadLibraries(libraryPaths, searchPaths, doc);
    readSourceUri(doc, stage);

    // First, load all nodedefs. Having these available is needed
    // when node instances are loaded later.
    for (const NodeDefPtr& nodedef : doc->getNodeDefs())
    {
        if (!stage->findChildByName(RtToken(nodedef->getName())))
        {
            readNodeDef(nodedef, stage);
        }
    }

    // Second, load all other elements.
    for (const ElementPtr& elem : doc->getChildren())
    {
        if (elem->isA<NodeDef>() || stage->findChildByName(RtToken(elem->getName())))
        {
            continue;
        }

        if (elem->isA<Node>())
        {
            readNode(elem->asA<Node>(), stage, stage);
        }
        else if (elem->isA<NodeGraph>())
        {
            readNodeGraph(elem->asA<NodeGraph>(), stage, stage);
        }
        else
        {
            readUnknown(elem, stage);
        }
    }
}

void RtFileIo::write(const FilePath& documentPath, const RtWriteOptions* options)
{
    PvtStage* stage = data()->asA<PvtStage>();

    DocumentPtr document = createDocument(); 
    writeDocument(document, stage, options);
    
    XmlWriteOptions xmlWriteOptions;
    if (options)
    {
        xmlWriteOptions.writeXIncludeEnable = options->writeIncludes;
    }
    writeToXmlFile(document, documentPath, &xmlWriteOptions);
}

void RtFileIo::write(std::ostream& stream, const RtWriteOptions* options)
{
    PvtStage* stage = data()->asA<PvtStage>();

    DocumentPtr document = createDocument();
    writeDocument(document, stage, options);

    XmlWriteOptions xmlWriteOptions;
    if (options)
    {
        xmlWriteOptions.writeXIncludeEnable = options->writeIncludes;
    }
    writeToXmlStream(document, stream, &xmlWriteOptions);
}

}
