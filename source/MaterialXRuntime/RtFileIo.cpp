//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtFileIo.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtBackdrop.h>
#include <MaterialXRuntime/RtGeneric.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtTraversal.h>
#include <MaterialXRuntime/RtApi.h>

#include <MaterialXRuntime/Private/PvtStage.h>

#include <MaterialXCore/Types.h>

#include <MaterialXGenShader/Util.h>
#include <sstream>

namespace MaterialX
{

namespace
{
    // Lists of known metadata which are handled explicitly by import/export.
    static const RtTokenSet nodedefMetadata     = { RtToken("name"), RtToken("type"), RtToken("node") };
    static const RtTokenSet attrMetadata        = { RtToken("name"), RtToken("type"), RtToken("value"), RtToken("nodename"), RtToken("output"), RtToken("colorspace"), RtToken("unit") };
    static const RtTokenSet nodeMetadata        = { RtToken("name"), RtToken("type"), RtToken("node") };
    static const RtTokenSet nodegraphMetadata   = { RtToken("name") };
    static const RtTokenSet genericMetadata     = { RtToken("name"), RtToken("kind") };
    static const RtTokenSet stageMetadata       = {};

    static const RtToken MULTIOUTPUT("multioutput");

    PvtPrim* findPrimOrThrow(const RtToken& name, PvtPrim* parent)
    {
        PvtPrim* prim = parent->getChild(name);
        if (!prim)
        {
            throw ExceptionRuntimeError("Can't find node '" + name.str() + "' in '" + parent->getName().str() + "'");
        }
        return prim;
    }

    PvtInput* findInputOrThrow(const RtToken& name, PvtPrim* prim)
    {
        PvtInput* input = prim->getInput(name);
        if (!input)
        {
            throw ExceptionRuntimeError("Node '" + prim->getName().str() + "' has no input named '" + name.str() + "'");
        }
        return input;
    }

    PvtOutput* findOutputOrThrow(const RtToken& name, PvtPrim* prim)
    {
        PvtOutput* output = prim->getOutput(name);
        if (!output)
        {
            throw ExceptionRuntimeError("Node '" + prim->getName().str() + "' has no output named '" + name.str() + "'");
        }
        return output;
    }

    void readMetadata(const ElementPtr src, PvtObject* dest, const RtTokenSet& ignoreList)
    {
        // Read in all metadata so we can export the element again
        // without loosing data.
        for (const string& name : src->getAttributeNames())
        {
            const RtToken mdName(name);
            if (!ignoreList.count(mdName))
            {
                // Store all custom attributes as string tokens.
                RtTypedValue* md = dest->addMetadata(mdName, RtType::TOKEN);
                md->getValue().asToken() = src->getAttribute(name);
            }
        }
    }

    void writeMetadata(const PvtObject* src, ElementPtr dest, const RtTokenSet& ignoreList)
    {
        for (const RtToken name : src->getMetadataOrder())
        {
            if (ignoreList.count(name) ||
                (name.str().size() > 0 && name.str().at(0) == '_')) // Metadata with "_" prefix are private
            {
                continue;
            }
            const RtTypedValue* md = src->getMetadata(name);
            dest->setAttribute(name.str(), md->getValueString());
        }
    }

    template<class T>
    void createInterface(const ElementPtr src, T schema)
    {
        for (auto elem : src->getChildrenOfType<ValueElement>())
        {
            const RtToken attrName(elem->getName());
            const RtToken attrType(elem->getType());

            RtAttribute attr;
            if (elem->isA<Output>())
            {
                attr = schema.createOutput(attrName, attrType);
            }
            else if (elem->isA<Input>())
            {
                attr = schema.createInput(attrName, attrType);
            }
            else
            {
                attr = schema.createInput(attrName, attrType, RtAttrFlag::UNIFORM);
            }

            const string& valueStr = elem->getValueString();
            if (!valueStr.empty())
            {
                RtValue::fromString(attrType, valueStr, attr.getValue());
            }
            if (elem->hasColorSpace())
            {
                attr.setColorSpace(RtToken(elem->getColorSpace()));
            }
            // TODO: fix when units are implemented in core
            // input->setUnit(RtToken(elem->getUnit()));

            readMetadata(elem, PvtObject::ptr<PvtObject>(attr), attrMetadata);
        }
    }

    void createNodeConnections(const vector<NodePtr>& nodeElements, PvtPrim* parent)
    {
        for (const NodePtr& nodeElem : nodeElements)
        {
            PvtPrim* node = findPrimOrThrow(RtToken(nodeElem->getName()), parent);
            for (const InputPtr& elemInput : nodeElem->getInputs())
            {
                PvtInput* input = findInputOrThrow(RtToken(elemInput->getName()), node);
                const string& connectedNodeName = elemInput->getNodeName();
                if (!connectedNodeName.empty())
                {
                    PvtPrim* connectedNode = findPrimOrThrow(RtToken(connectedNodeName), parent);
                    const RtToken outputName(elemInput->getOutputString());
                    PvtOutput* output = findOutputOrThrow(outputName != EMPTY_TOKEN ? outputName : PvtAttribute::DEFAULT_OUTPUT_NAME, connectedNode);
                    output->connect(input);
                }
            }
        }
    }

    PvtPrim* readNodeDef(const NodeDefPtr& src, PvtStage* stage)
    {
        const RtToken name(src->getName());
        PvtPrim* prim = stage->createPrim(stage->getPath(), name, RtNodeDef::typeName());

        const RtToken nodeName(src->getNodeString());
        RtNodeDef nodedef(prim->hnd());
        nodedef.setNode(nodeName);

        readMetadata(src, prim, nodedefMetadata);

        // Create the interface.
        createInterface(src, nodedef);

        return prim;
    }

    bool matchingSignature(const PvtPrim* prim, const RtToken& nodeType, const vector<ValueElementPtr>& nodePorts)
    {
        if (nodeType != MULTIOUTPUT)
        {
            // For single output nodes we can match the output directly.
            PvtOutput* out = prim->getOutput(PvtAttribute::DEFAULT_OUTPUT_NAME);
            if (!out || out->getType() != nodeType)
            {
                return false;
            }
        }

        // Check all ports.
        // TODO: Do we need to match port type as well (input/output/parameter)?
        for (const ValueElementPtr& nodePort : nodePorts)
        {
            const PvtAttribute* attr = prim->getAttribute(RtToken(nodePort->getName()));
            if (!attr || attr->getType().str() != nodePort->getType())
            {
                return false;
            }
        }
        return true;
    }

    RtToken resolveNodeDefName(const NodePtr& node)
    {
        // First, see if a nodedef is specified on the node.
        const string& nodedefString = node->getNodeDefString();
        if (!nodedefString.empty())
        {
            return RtToken(nodedefString);
        }

        // Second, try resolving a nodedef from content in the current document.
        NodeDefPtr srcNodedef = node->getNodeDef();
        if (srcNodedef)
        {
            const RtToken nodedefName(srcNodedef->getName());
            return nodedefName;
        }

        // Third, try resolving among existing registered master prim nodedefs.
        const RtToken nodeName(node->getCategory());
        const RtToken nodeType(node->getType());
        const vector<ValueElementPtr> nodePorts = node->getActiveValueElements();
        RtSchemaPredicate<RtNodeDef> nodedefFilter;
        for (RtPrim masterPrim : RtApi::get().getMasterPrims(nodedefFilter))
        {
            RtNodeDef candidate(masterPrim);
            if (candidate.getNode() == nodeName &&
                matchingSignature(PvtObject::ptr<PvtPrim>(masterPrim), nodeType, nodePorts))
            {
                return candidate.getName();
            }
        }

        return EMPTY_TOKEN;
    }

    PvtPrim* readNode(const NodePtr& src, PvtPrim* parent, PvtStage* stage)
    {
        const RtToken nodedefName = resolveNodeDefName(src);
        if (nodedefName == EMPTY_TOKEN)
        {
            throw ExceptionRuntimeError("No matching nodedef was found for node '" + src->getName() + "'");
        }

        const RtToken nodeName(src->getName());
        PvtPrim* node = stage->createPrim(parent->getPath(), nodeName, nodedefName);

        readMetadata(src, node, attrMetadata);

        // Copy input values.
        for (auto elem : src->getChildrenOfType<ValueElement>())
        {
            if (elem->isA<Output>())
            {
                continue;
            }
            const RtToken portName(elem->getName());
            PvtAttribute* input = node->getInput(portName);
            if (!input)
            {
                throw ExceptionRuntimeError("No input named '" + elem->getName() + "' was found on runtime node '" + node->getName().str() + "'");
            }
            const string& valueStr = elem->getValueString();
            if (!valueStr.empty())
            {
                const RtToken portType(elem->getType());
                RtValue::fromString(portType, valueStr, input->getValue());
            }
        }

        return node;
    }

    PvtPrim* readNodeGraph(const NodeGraphPtr& src, PvtPrim* parent, PvtStage* stage)
    {
        const RtToken nodegraphName(src->getName());

        PvtPrim* nodegraph = stage->createPrim(parent->getPath(), nodegraphName, RtNodeGraph::typeName());
        RtNodeGraph schema(nodegraph->hnd());

        readMetadata(src, nodegraph, nodegraphMetadata);

        // Create the interface either from a nodedef if given
        // otherwise from the graph itself.
        const NodeDefPtr srcNodeDef = src->getNodeDef();
        if (srcNodeDef)
        {
            createInterface(srcNodeDef, schema);
        }
        else
        {
            createInterface(src, schema);
        }

        // Create all nodes and connection between node inputs and internal graph sockets.
        for (auto child : src->getChildren())
        {
            NodePtr srcNnode = child->asA<Node>();
            if (srcNnode)
            {
                PvtPrim* node = readNode(srcNnode, nodegraph, stage);

                // Check for connections to the internal graph sockets
                for (auto elem : srcNnode->getChildrenOfType<ValueElement>())
                {
                    if (elem->isA<Output>())
                    {
                        continue;
                    }
                    const string& interfaceName = elem->getInterfaceName();
                    if (!interfaceName.empty())
                    {
                        const RtToken inputName(elem->getName());
                        const RtToken socketName(interfaceName);
                        RtOutput socket = schema.getInputSocket(socketName);
                        if (!socket)
                        {
                            const RtToken inputType(elem->getType());
                            RtInput input = schema.createInput(socketName, inputType);
                            socket = schema.getInputSocket(input.getName());
                        }

                        RtInput input(findInputOrThrow(inputName, node)->hnd());
                        socket.connect(input);
                    }
                }
            }
        }

        // Create connections between all nodes.
        createNodeConnections(src->getNodes(), nodegraph);

        // Create connections between node outputs and internal graph sockets.
        for (const OutputPtr& elem : src->getOutputs())
        {
            const string& connectedNodeName = elem->getNodeName();
            if (!connectedNodeName.empty())
            {
                RtInput socket = schema.getOutputSocket(RtToken(elem->getName()));
                if (!socket)
                {
                    PvtPath path(parent->getPath());
                    path.push(nodegraphName);
                    throw ExceptionRuntimeError("Output '" + elem->getName() + "' does not match an internal output socket on the nodegraph '" + path.asString() + "'");
                }

                PvtPrim* connectedNode = findPrimOrThrow(RtToken(connectedNodeName), nodegraph);

                const RtToken outputName(elem->getOutputString());
                RtOutput output(findOutputOrThrow(outputName != EMPTY_TOKEN ? outputName : PvtAttribute::DEFAULT_OUTPUT_NAME, connectedNode)->hnd());
                output.connect(socket);
            }
        }

        return nodegraph;
    }

    PvtPrim* readGenericPrim(const ElementPtr& src, PvtPrim* parent, PvtStage* stage)
    {
        const RtToken name(src->getName());
        const RtToken category(src->getCategory());

        PvtPrim* prim = stage->createPrim(parent->getPath(), name, RtGeneric::typeName());
        RtGeneric generic(prim->hnd());
        generic.setKind(category);

        readMetadata(src, prim, genericMetadata);

        for (auto child : src->getChildren())
        {
            readGenericPrim(child, prim, stage);
        }

        return prim;
    }

    void readDocument(const DocumentPtr& doc, PvtStage* stage, const RtReadOptions* readOptions)
    {
        // Set the source location 
        const std::string& uri = doc->getSourceUri();
        stage->addSourceUri(RtToken(uri));

        readMetadata(doc, stage->getRootPrim(), stageMetadata);

        RtReadOptions::ReadFilter filter = readOptions ? readOptions->readFilter : nullptr;

        // First, load and register all nodedefs.
        // Having these available is needed when node instances are loaded later.
        for (const NodeDefPtr& nodedef : doc->getNodeDefs())
        {
            if (!filter || filter(nodedef))
            {
                PvtPath path("/" + nodedef->getName());
                if (!stage->getPrimAtPath(path))
                {
                    PvtPrim* prim = readNodeDef(nodedef, stage);
                    RtNodeDef(prim->hnd()).registerMasterPrim();
                }
            }
        }

        // Load all other elements.
        for (const ElementPtr& elem : doc->getChildren())
        {
            if (!filter || filter(elem))
            {
                // Make sure the element has not been loaded already.
                PvtPath path("/" + elem->getName());
                if (stage->getPrimAtPath(path))
                {
                    continue;
                }

                if (elem->isA<Node>())
                {
                    readNode(elem->asA<Node>(), stage->getRootPrim(), stage);
                }
                else if (elem->isA<NodeGraph>())
                {
                    readNodeGraph(elem->asA<NodeGraph>(), stage->getRootPrim(), stage);
                }
                else
                {
                    readGenericPrim(elem, stage->getRootPrim(), stage);
                }
            }
        }

        // Create connections between all root level nodes.
        createNodeConnections(doc->getNodes(), stage->getRootPrim());
    }

    void writeNodeDef(const PvtPrim* src, DocumentPtr dest)
    {
        RtNodeDef nodedef(src->hnd());

        NodeDefPtr destNodeDef = dest->addNodeDef(nodedef.getName(), EMPTY_STRING, nodedef.getNode());
        writeMetadata(src, destNodeDef, nodedefMetadata);

        for (const PvtDataHandle attrH : src->getAllAttributes())
        {
            const PvtAttribute* attr = attrH->asA<PvtAttribute>();

            ValueElementPtr destPort;
            if (attr->isA<PvtInput>())
            {
                const PvtInput* input = attr->asA<PvtInput>();
                if (input->isUniform())
                {
                    destPort = destNodeDef->addParameter(attr->getName(), attr->getType().str());
                }
                else
                {
                    destPort = destNodeDef->addInput(attr->getName(), attr->getType().str());
                }
            }
            else
            {
                destPort = destNodeDef->addOutput(attr->getName(), attr->getType().str());
            }

            destPort->setValueString(attr->getValueString());

            if (attr->getColorSpace())
            {
                destPort->setColorSpace(attr->getColorSpace().str());
            }
            // TODO: fix when units are implemented in core.
            //if (attr->getUnit())
            //{
            //    destInput->setUnit(input->getUnit().str());
            //}

            writeMetadata(attr, destPort, attrMetadata);
        }
    }

    NodePtr writeNode(const PvtPrim* src, GraphElementPtr dest)
    {
        RtNode node(src->hnd());
        RtNodeDef nodedef(node.getNodeDef());

        // Count output and get output type
        size_t numOutputs = 0;
        string outputType;
        RtObjTypePredicate<RtOutput> outputFilter;
        for (RtAttribute attr : nodedef.getPrim().getAttributes(outputFilter))
        {
            numOutputs++;
            outputType = attr.getType();
        }

        NodePtr destNode = dest->addNode(nodedef.getNode(), node.getName(), numOutputs > 1 ? "multioutput" : outputType);

        for (RtAttribute attrDef : nodedef.getPrim().getAttributes())
        {
            RtAttribute attr = node.getPrim().getAttribute(attrDef.getName());
            RtInput input = attr.asA<RtInput>();
            if (input)
            {
                // Write input if it's connected or different from default value.
                if (input.isConnected() || !RtValue::compare(input.getType(), input.getValue(), attrDef.getValue()))
                {
                    ValueElementPtr valueElem;
                    if (input.isUniform())
                    {
                        valueElem = destNode->addParameter(input.getName(), input.getType());
                        if (input.isConnected())
                        {
                            RtOutput source = input.getConnection();
                            if (source.isSocket())
                            {
                                // This is a connection to the internal socket of a graph
                                valueElem->setInterfaceName(source.getName());
                            }
                        }
                        else
                        {
                            valueElem->setValueString(input.getValueString());
                        }
                    }
                    else
                    {
                        valueElem = destNode->addInput(input.getName(), input.getType());
                        if (input.isConnected())
                        {
                            RtOutput source = input.getConnection();
                            if (source.isSocket())
                            {
                                // This is a connection to the internal socket of a graph
                                valueElem->setInterfaceName(source.getName());
                            }
                            else
                            {
                                RtPrim sourceNode = source.getParent();
                                InputPtr inputElem = valueElem->asA<Input>();
                                inputElem->setNodeName(sourceNode.getName());
                                inputElem->setOutputString(source.getName());
                            }
                        }
                        else
                        {
                            valueElem->setValueString(input.getValueString());
                        }
                    }

                    const RtToken colorspace = input.getColorSpace();
                    if (colorspace != EMPTY_TOKEN)
                    {
                        valueElem->setColorSpace(colorspace.str());
                    }
                    //if (input.getUnit())
                    //{
                    //    TODO: fix when units are implemented in core.
                    //    valueElem->setUnit(input->getUnit().str());
                    //}
                }
                else if(numOutputs > 1)
                {
                    destNode->addOutput(attr.getName(), attr.getType());
                }
            }
        }

        writeMetadata(src, destNode, nodeMetadata);

        return destNode;
    }

    void createMaterialNode(const PvtPrim* prim, NodePtr mxNode, DocumentPtr doc)
    {
        // Check to see if the surfaceshader node is already connected to a material node
        bool isConnectedToMaterialNode = false;
        const PvtOutput* output = prim->getOutput(PvtAttribute::DEFAULT_OUTPUT_NAME);
        if (output && output->getType() == RtType::SURFACESHADER && output->isConnected())
        {
            for (const RtObject& dest : output->getConnections())
            {
                RtInput destInput = dest.asA<RtInput>();
                RtPrim destNode = destInput.getParent();
                RtOutput destOutput = destNode.getOutput(PvtAttribute::DEFAULT_OUTPUT_NAME);
                if (destOutput.getType() == RtType::MATERIAL)
                {
                    isConnectedToMaterialNode = true;
                    break;
                }
            }
        }

        if (!isConnectedToMaterialNode)
        {
            NodePtr materialNode = doc->addNode(SURFACE_MATERIAL_NODE_STRING, mxNode->getName() + "_SurfaceMaterial", MATERIAL_TYPE_STRING);
            materialNode->setConnectedNode(SURFACE_SHADER_TYPE_STRING, mxNode);
        }
    }

    void writeMaterialElementsHelper(const PvtPrim* prim, NodePtr mxNode, const string& materialBaseName, DocumentPtr doc, const RtWriteOptions* writeOptions)
    {
        MaterialPtr material = doc->addMaterial(materialBaseName + "_Material");
        ShaderRefPtr shaderRef = material->addShaderRef("sref", mxNode->getName());

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
            doc->removeChild(prim->getName());
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

    void writeMaterialElements(const PvtPrim* prim, NodePtr mxNode, DocumentPtr doc, const RtWriteOptions* writeOptions)
    {
        if (writeOptions->materialWriteOp & RtWriteOptions::MaterialWriteOp::ADD_MATERIAL_NODES_FOR_SHADERS)
        {
            // Get the connected material nodes and create material elements from them (using their names)
            const PvtOutput* output = prim->getOutput(PvtAttribute::DEFAULT_OUTPUT_NAME);
            if (output->isConnected())
            {
                for (const RtObject& dest : output->getConnections())
                {
                    RtInput destInput = dest.asA<RtInput>();
                    RtPrim destNode = destInput.getParent();
                    RtOutput destOutput = destNode.getOutput(PvtAttribute::DEFAULT_OUTPUT_NAME);
                    if (destOutput.getType() == RtType::MATERIAL)
                    {
                        writeMaterialElementsHelper(prim, mxNode, destNode.getName(), doc, writeOptions);
                    }
                }
            }
        }
        else
        {
            writeMaterialElementsHelper(prim, mxNode, mxNode->getName(), doc, writeOptions);
        }
    }

    void writeNodeGraph(const PvtPrim * src, DocumentPtr dest)
    {
        NodeGraphPtr destNodeGraph = dest->addNodeGraph(src->getName());
        writeMetadata(src, destNodeGraph, nodegraphMetadata);

        RtNodeGraph nodegraph(src->hnd());

        // Write nodes.
        for (RtPrim node : nodegraph.getNodes())
        {
            writeNode(PvtObject::ptr<PvtPrim>(node), destNodeGraph);
        }

        // Write outputs.
        RtObjTypePredicate<RtOutput> outputsFilter;
        for (RtAttribute attr : src->getAttributes(outputsFilter))
        {
            RtInput nodegraphOutput = nodegraph.getOutputSocket(attr.getName());
            OutputPtr output = destNodeGraph->addOutput(nodegraphOutput.getName(), nodegraphOutput.getType());
            if (nodegraphOutput.isConnected())
            {
                RtOutput source = nodegraphOutput.getConnection();
                if (source.isSocket())
                {
                    output->setInterfaceName(source.getName());
                }
                else
                {
                    RtPrim sourceNode = source.getParent();
                    output->setNodeName(sourceNode.getName());
                    output->setOutputString(source.getName());
                }
            }
        }
    }

    void writeGenericPrim(const PvtPrim* src, ElementPtr dest)
    {
        RtGeneric generic(src->hnd());

        ElementPtr elem = dest->addChildOfCategory(generic.getKind(), generic.getName());
        writeMetadata(src, elem, genericMetadata);

        for (auto child : src->getChildren())
        {
            writeGenericPrim(PvtObject::ptr<PvtPrim>(child), elem);
        }
    }

    void writeSourceUris(const PvtStage* stage, DocumentPtr doc)
    {
        for (const RtStagePtr& refPtr : stage->getAllReferences())
        {
            const PvtStage* ref = PvtStage::ptr(refPtr);
            if (ref->getAllReferences().size() > 0)
            {
                writeSourceUris(ref, doc);
            }
            const RtTokenList& uris = ref->getSourceUri();
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
        writeMetadata(stage->getRootPrim(), doc, RtTokenSet());

        // Write out any dependent includes
        if (writeOptions && writeOptions->writeIncludes)
        {
            writeSourceUris(stage, doc);
        }

        RtWriteOptions::WriteFilter filter = writeOptions ? writeOptions->writeFilter : nullptr;
        for (RtPrim child : stage->getRootPrim()->getChildren(filter))
        {
            const PvtPrim* prim = PvtObject::ptr<PvtPrim>(child);
            const RtToken typeName = child.getTypeName();
            if (typeName == RtNodeDef::typeName())
            {
                writeNodeDef(prim, doc);
            }
            else if (typeName == RtNode::typeName())
            {
                NodePtr mxNode = writeNode(prim, doc);

                const PvtOutput* output = prim->getOutput(PvtAttribute::DEFAULT_OUTPUT_NAME);
                if (output && output->getType() == RtType::SURFACESHADER)
                {
                    if (writeOptions->materialWriteOp & RtWriteOptions::MaterialWriteOp::ADD_MATERIAL_NODES_FOR_SHADERS)
                    {
                        createMaterialNode(prim, mxNode, doc);
                    }
                    if (writeOptions->materialWriteOp & RtWriteOptions::MaterialWriteOp::WRITE_MATERIALS_AS_ELEMENTS)
                    {
                        writeMaterialElements(prim, mxNode, doc, writeOptions);
                    }
                }
            }
            else if (typeName == RtNodeGraph::typeName())
            {
                writeNodeGraph(prim, doc);
            }
            else
            {
                writeGenericPrim(prim, doc->asA<Element>());
            }
        }
    }

} // end anonymous namespace

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

        PvtStage* stage = PvtStage::ptr(_stage);
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

        PvtStage* stage = PvtStage::ptr(_stage);
        readDocument(document, stage, readOptions);
    }
    catch (Exception& e)
    {
        throw ExceptionRuntimeError(string("Could not read from stream. Error: ") + e.what());
    }
}

void RtFileIo::readLibraries(const StringVec& libraryPaths, const FileSearchPath& searchPaths)
{
    PvtStage* stage = PvtStage::ptr(_stage);

    // Load all content into a document.
    DocumentPtr doc = createDocument();
    MaterialX::loadLibraries(libraryPaths, searchPaths, doc);

    StringSet uris = doc->getReferencedSourceUris();
    for (const string& uri : uris)
    {
        stage->addSourceUri(RtToken(uri));
    }

    // First, load all nodedefs. Having these available is needed
    // when node instances are loaded later.
    for (const NodeDefPtr& nodedef : doc->getNodeDefs())
    {
        PvtPath path(stage->getPath());
        path.push(RtToken(nodedef->getName()));
        if (!stage->getPrimAtPath(path))
        {
            PvtPrim* prim = readNodeDef(nodedef, stage);
            RtNodeDef(prim->hnd()).registerMasterPrim();
        }
    }

    // Second, load all other elements.
    for (const ElementPtr& elem : doc->getChildren())
    {
        PvtPath path(stage->getPath());
        path.push(RtToken(elem->getName()));

        if (elem->isA<NodeDef>() || stage->getPrimAtPath(path))
        {
            continue;
        }

        if (elem->isA<Node>())
        {
            readNode(elem->asA<Node>(), stage->getRootPrim(), stage);
        }
        else if (elem->isA<NodeGraph>())
        {
            readNodeGraph(elem->asA<NodeGraph>(), stage->getRootPrim(), stage);
        }
        else
        {
            readGenericPrim(elem, stage->getRootPrim(), stage);
        }
    }
}

void RtFileIo::write(const FilePath& documentPath, const RtWriteOptions* options)
{
    PvtStage* stage = PvtStage::ptr(_stage);

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
    PvtStage* stage = PvtStage::ptr(_stage);

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
