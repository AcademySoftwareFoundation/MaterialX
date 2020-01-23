//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtFileIo.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXRuntime/Private/PvtStage.h>
#include <MaterialXRuntime/Private/PvtNodeDef.h>
#include <MaterialXRuntime/Private/PvtNode.h>
#include <MaterialXRuntime/Private/PvtNodeGraph.h>

#include <MaterialXGenShader/Util.h>
#include <sstream>

namespace MaterialX
{

namespace
{
    // Lists of known attributes which are handled explicitly by import/export.
    static const RtTokenSet nodedefAttrs    = { RtToken("name"), RtToken("type"), RtToken("node") };
    static const RtTokenSet portdefAttrs    = { RtToken("name"), RtToken("type"), RtToken("value"), RtToken("nodename"), RtToken("output"), RtToken("colorspace"), RtToken("unit") };
    static const RtTokenSet nodeAttrs       = { RtToken("name"), RtToken("type"), RtToken("node") };
    static const RtTokenSet nodegraphAttrs  = { RtToken("name") };
    static const RtTokenSet genericAttrs    = { RtToken("name") };
    static const RtTokenSet stageAttrs      = {};

    PvtNode* findNodeOrThrow(const RtToken& name, PvtPrim* parent)
    {
        PvtPrim* node = parent->getChild(name);
        if (!(node && node->hasApi(RtApiType::NODE)))
        {
            throw ExceptionRuntimeError("Prim '" + name.str() + "' is not a node in '" + parent->getName().str() + "'");
        }
        return node->asA<PvtNode>();
    }

    PvtAttribute* findAttrOrThrow(const RtToken& name, PvtNode* node)
    {
        PvtAttribute* port = node->getAttribute(name);
        if (!port)
        {
            throw ExceptionRuntimeError("Node '" + node->getName().str() + "' has no attribute named '" + name.str() + "'");
        }
        return port;
    }

    void readCustomMetadata(const ElementPtr src, PvtPathItem* dest, const RtTokenSet& knownAttrNames)
    {
        // Read in all custom metadata so we can export the element again
        // without loosing data.
        for (const string& name : src->getAttributeNames())
        {
            const RtToken mdName(name);
            if (!knownAttrNames.count(mdName))
            {
                // Store all custom attributes as string tokens.
                RtTypedValue* md = dest->addMetadata(mdName, RtType::TOKEN);
                md->getValue().asToken() = src->getAttribute(name);
            }
        }
    }

    void createInterface(const ElementPtr src, PvtPrim* dest)
    {
        for (auto elem : src->getChildrenOfType<ValueElement>())
        {
            const RtToken attrName(elem->getName());
            const RtToken attrType(elem->getType());
            uint32_t attrFlags = 0;
            if (elem->isA<Output>())
            {
                attrFlags |= RtAttrFlag::OUTPUT;
            }
            else if (elem->isA<Parameter>())
            {
                attrFlags |= RtAttrFlag::UNIFORM;
            }

            PvtAttribute* attr = dest->createAttribute(attrName, attrType, attrFlags);

            const string& valueStr = elem->getValueString();
            if (!valueStr.empty())
            {
                RtValue::fromString(attrType, valueStr, attr->getValue());
            }
            if (elem->hasColorSpace())
            {
                attr->setColorSpace(RtToken(elem->getColorSpace()));
            }
            // TODO: fix when units are implemented in core
            // input->setUnit(RtToken(elem->getUnit()));

            readCustomMetadata(elem, attr, portdefAttrs);
        }
    }

    void createNodeConnections(const vector<NodePtr>& nodeElements, PvtPrim* parent)
    {
        for (const NodePtr& nodeElem : nodeElements)
        {
            PvtNode* node = findNodeOrThrow(RtToken(nodeElem->getName()), parent);
            for (const InputPtr& elemInput : nodeElem->getInputs())
            {
                PvtAttribute* input = findAttrOrThrow(RtToken(elemInput->getName()), node);
                const string& connectedNodeName = elemInput->getNodeName();
                if (!connectedNodeName.empty())
                {
                    PvtNode* connectedNode = findNodeOrThrow(RtToken(connectedNodeName), parent);
                    const RtToken outputName(elemInput->getOutputString());
                    PvtAttribute* output = findAttrOrThrow(outputName != EMPTY_TOKEN ? outputName : PvtAttribute::DEFAULT_OUTPUT_NAME, connectedNode);
                    PvtAttribute::connect(output, input);
                }
            }
        }
    }

    void writeMetadata(const PvtPathItem* src, ElementPtr dest)
    {
        for (const RtToken name : src->getMetadataOrder())
        {
            // Ignore metadata with "_" prefix as these are private.
            if (name.str().size() > 0 && name.str().at(0) == '_')
            {
                continue;
            }
            const RtTypedValue* md = src->getMetadata(name);
            dest->setAttribute(name.str(), md->getValueString());
        }
    }

    PvtNodeDef* readNodeDef(const NodeDefPtr& src, PvtStage* stage)
    {
        const RtToken name(src->getName());
        PvtPrim* prim = stage->createPrim(stage->getPath(), name, PvtNodeDef::typeName());

        const RtToken nodeTypeName(src->getNodeString());
        PvtNodeDef* nodedef = prim->asA<PvtNodeDef>();
        nodedef->setNodeTypeName(nodeTypeName);

        readCustomMetadata(src, nodedef, nodedefAttrs);

        // Create the interface.
        createInterface(src, nodedef);

        return nodedef;
    }

    bool matchingSignature(const PvtNodeDef* nodedef, const vector<ValueElementPtr>& nodePorts)
    {
        // Check all ports.
        // TODO: Do we need to match port type as well (input/output/parameter)?
        for (const ValueElementPtr& nodePort : nodePorts)
        {
            const PvtAttribute* attr = nodedef->getAttribute(RtToken(nodePort->getName()));
            if (!attr || attr->getType().str() != nodePort->getType())
            {
                return false;
            }
        }
        return true;
    }

    PvtNodeDef* resolveNodeDef(const NodePtr& node, PvtStage* stage)
    {
        PvtNodeDef* nodedef = 0;

        // First try resolving a nodedef from content in the current document.
        NodeDefPtr srcNodedef = node->getNodeDef();
        if (srcNodedef)
        {
            const RtToken nodedefName(srcNodedef->getName());

            PvtPath path(stage->getPath());
            path.push(nodedefName);

            PvtPrim* prim = stage->getPrimAtPath(path);
            if (prim)
            {
                if (!prim->hasApi(RtApiType::NODEDEF))
                {
                    throw ExceptionRuntimeError("Prim at path '" + path.asString() + "' is not a nodedef");
                }
                nodedef = prim->asA<PvtNodeDef>();
            }
            else
            {
                // NodeDef is not loaded yet so create it now.
                nodedef = readNodeDef(srcNodedef, stage);
            }
        }

        if (!nodedef)
        {
            // Try resolving among existing nodedefs in the stage
            // by matching signatures.

            const RtToken nodeTypeName(node->getCategory());
            const vector<ValueElementPtr> nodePorts = node->getActiveValueElements();

            RtObjTypePredicate<RtObjType::NODEDEF> nodedefFilter;
            for (const PvtDataHandle& hnd : stage->traverse(nodedefFilter))
            {
                PvtNodeDef* candidate = hnd->asA<PvtNodeDef>();
                if (candidate->getNodeTypeName() == nodeTypeName &&
                    matchingSignature(candidate, nodePorts))
                {
                    return candidate;
                }
            }
        }

        return nodedef;
    }

    PvtNode* readNode(const NodePtr& src, PvtPrim* parent, PvtStage* stage)
    {
        PvtNodeDef* nodedef = resolveNodeDef(src, stage);
        if (!nodedef)
        {

            nodedef = resolveNodeDef(src, stage);

            throw ExceptionRuntimeError("No matching nodedef was found for node '" + src->getName() + "'");
        }

        const RtToken nodeName(src->getName());
        PvtPrim* prim = stage->createPrim(parent->getPath(), nodeName, PvtNode::typeName(), nodedef);
        PvtNode* node = prim->asA<PvtNode>();

        readCustomMetadata(src, node, nodeAttrs);

        // Copy input values.
        for (auto elem : src->getChildrenOfType<ValueElement>())
        {
            const RtToken portName(elem->getName());
            PvtAttribute* attr = node->getAttribute(portName);
            if (!attr)
            {
                throw ExceptionRuntimeError("No attribute named '" + elem->getName() + "' was found on runtime node '" + node->getName().str() + "'");
            }
            const string& valueStr = elem->getValueString();
            if (!valueStr.empty())
            {
                const RtToken portType(elem->getType());
                RtValue::fromString(portType, valueStr, attr->getValue());
            }
        }

        return node;
    }

    PvtNodeGraph* readNodeGraph(const NodeGraphPtr& src, PvtPrim* parent, PvtStage* stage)
    {
        const RtToken nodegraphName(src->getName());

        PvtPrim* prim = stage->createPrim(parent->getPath(), nodegraphName, PvtNodeGraph::typeName());
        PvtNodeGraph* nodegraph = prim->asA<PvtNodeGraph>();

        readCustomMetadata(src, nodegraph, nodegraphAttrs);

        // Create the interface either from a nodedef if given
        // otherwise from the graph itself.
        const NodeDefPtr srcNodeDef = src->getNodeDef();
        if (srcNodeDef)
        {
            createInterface(srcNodeDef, nodegraph);
        }
        else
        {
            createInterface(src, nodegraph);
        }

        // Create all nodes and connection between node inputs and internal graph sockets.
        for (auto child : src->getChildren())
        {
            NodePtr srcNnode = child->asA<Node>();
            if (srcNnode)
            {
                PvtNode* node = readNode(srcNnode, nodegraph, stage);

                // Check for connections to the internal graph sockets
                for (auto elem : srcNnode->getChildrenOfType<ValueElement>())
                {
                    const string& interfaceName = elem->getInterfaceName();
                    if (!interfaceName.empty())
                    {
                        const RtToken inputName(elem->getName());
                        const RtToken socketName(interfaceName);
                        PvtAttribute* socket = nodegraph->getInputSocket(socketName);
                        if (!socket)
                        {
                            const RtToken inputType(elem->getType());
                            PvtAttribute* attr = nodegraph->createAttribute(socketName, inputType);
                            socket = nodegraph->getInputSocket(attr->getName());
                        }

                        PvtAttribute* input = findAttrOrThrow(inputName, node);
                        PvtAttribute::connect(socket, input);
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
                PvtAttribute* socket = nodegraph->getOutputSocket(RtToken(elem->getName()));
                if (!socket)
                {
                    PvtPath path(parent->getPath());
                    path.push(nodegraphName);
                    throw ExceptionRuntimeError("Output '" + elem->getName() + "' does not match an internal output socket on the nodegraph '" + path.asString() + "'");
                }

                PvtNode* connectedNode = findNodeOrThrow(RtToken(connectedNodeName), nodegraph);

                const RtToken outputName(elem->getOutputString());
                PvtAttribute* output = findAttrOrThrow(outputName != EMPTY_TOKEN ? outputName : PvtAttribute::DEFAULT_OUTPUT_NAME, connectedNode);

                PvtAttribute::connect(output, socket);
            }
        }

        return nodegraph;
    }

    PvtPrim* readGenericPrim(const ElementPtr& src, PvtPrim* parent, PvtStage* stage)
    {
        const RtToken name(src->getName());
        const RtToken category(src->getCategory());

        PvtPrim* prim = stage->createPrim(parent->getPath(), name, PvtPrim::typeName());
        prim->setPrimTypeName(category);

        readCustomMetadata(src, prim, genericAttrs);

        for (auto child : src->getChildren())
        {
            readGenericPrim(child, prim, stage);
        }

        return prim;
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

        readCustomMetadata(doc, stage->getRootPrim(), stageAttrs);

        RtReadOptions::ReadFilter filter = readOptions ? readOptions->readFilter : nullptr;

        // First, load all nodedefs. Having these available is needed
        // when node instances are loaded later.
        for (const NodeDefPtr& nodedef : doc->getNodeDefs())
        {
            if (!filter || filter(nodedef))
            {
                PvtPath path("/" + nodedef->getName());
                if (!stage->getPrimAtPath(path))
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

    void writeNodeDef(const PvtNodeDef* nodedef, DocumentPtr dest)
    {
        NodeDefPtr destNodeDef = dest->addNodeDef(nodedef->getName(), EMPTY_STRING, nodedef->getNodeTypeName());
        writeMetadata(nodedef, destNodeDef);

        for (const PvtDataHandle attrH : nodedef->getAllAttributes())
        {
            const PvtAttribute* attr = attrH->asA<PvtAttribute>();

            ValueElementPtr destPort;
            if (attr->isInput())
            {
                if (attr->isUniform())
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

            writeMetadata(attr, destPort);
        }
    }

    template<typename T>
    NodePtr writeNode(const PvtNode* node, T dest)
    {
        const PvtNodeDef* nodedef = node->getNodeDef()->asA<PvtNodeDef>();

        size_t numOutputs = 0;
        string outputType;
        for (const PvtDataHandle attrH : nodedef->getAllAttributes())
        {
            const PvtAttribute* attr = attrH->asA<PvtAttribute>();
            if (attr->isOutput())
            {
                numOutputs++;
                outputType = attr->getType();
            }
        }

        NodePtr destNode = dest->addNode(nodedef->getNodeTypeName(), node->getName().str(), numOutputs > 1 ? "multioutput" : outputType);

        for (const PvtDataHandle attrH : nodedef->getAllAttributes())
        {
            const PvtAttribute* attrDef = attrH->asA<PvtAttribute>();
            const PvtAttribute* attr = node->getAttribute(attrDef->getName());

            if (attr->isOutput())
            {
                if (numOutputs > 1)
                {
                    destNode->addOutput(attr->getName(), attr->getType().str());
                }
            }
            else if (attr->isConnected() || !RtValue::compare(attr->getType(), attr->getValue(), attrDef->getValue()))
            {
                ValueElementPtr valueElem;
                if (attr->isUniform())
                {
                    valueElem = destNode->addParameter(attr->getName().str(), attr->getType().str());
                    if (attr->isConnected())
                    {
                        PvtAttribute* source = attr->getConnection();
                        if (source->isSocket())
                        {
                            // This is a connection to the internal socket of a graph
                            valueElem->setInterfaceName(source->getName());
                        }
                    }
                    else
                    {
                        valueElem->setValueString(attr->getValueString());
                    }
                }
                else
                {
                    valueElem = destNode->addInput(attr->getName().str(), attr->getType().str());
                    if (attr->isConnected())
                    {
                        PvtAttribute* source = attr->getConnection();
                        if (source->isSocket())
                        {
                            // This is a connection to the internal socket of a graph
                            valueElem->setInterfaceName(source->getName());
                        }
                        else
                        {
                            const PvtPrim* sourceNode = source->getParent();
                            InputPtr inputElem = valueElem->asA<Input>();
                            inputElem->setNodeName(sourceNode->getName());
                            inputElem->setOutputString(source->getName());
                        }
                    }
                    else
                    {
                        valueElem->setValueString(attr->getValueString());
                    }
                }

                const RtToken colorspace = attr->getColorSpace();
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
        }

        writeMetadata(node, destNode);

        return destNode;
    }

    void writeMaterialElements(const PvtNode* node, NodePtr mxNode, const RtToken& nodeTypeName, DocumentPtr doc, const RtWriteOptions* writeOptions)
    {
        MaterialPtr material = doc->addMaterial(mxNode->getName() + "_Material");
        ShaderRefPtr shaderRef = material->addShaderRef("sref", nodeTypeName.str());
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

    void writeNodeGraph(const PvtNodeGraph* nodegraph, DocumentPtr dest)
    {
        NodeGraphPtr destNodeGraph = dest->addNodeGraph(nodegraph->getName());
        writeMetadata(nodegraph, destNodeGraph);

        // Write nodes.
        for (const RtObject child : nodegraph->getChildren())
        {
            if (child.hasApi(RtApiType::NODE))
            {
                writeNode(PvtObject::ptr<PvtNode>(child), destNodeGraph);
            }
        }

        // Write outputs.
        for (const PvtDataHandle attrH : nodegraph->getAllAttributes())
        {
            const PvtAttribute* attr = attrH->asA<PvtAttribute>();

            if (attr->isOutput())
            {
                const PvtAttribute* nodegraphOutput = nodegraph->getOutputSocket(attr->getName());

                OutputPtr output = destNodeGraph->addOutput(nodegraphOutput->getName(), nodegraphOutput->getType().str());

                if (nodegraphOutput->isConnected())
                {
                    const PvtAttribute* sourcePort = nodegraphOutput->getConnection();
                    if (sourcePort->isSocket())
                    {
                        output->setInterfaceName(sourcePort->getName());
                    }
                    else
                    {
                        const PvtPrim* sourceNode = sourcePort->getParent();
                        output->setNodeName(sourceNode->getName());
                        output->setOutputString(sourcePort->getName());
                    }
                }
            }
        }
    }

    void writeGenericPrim(const PvtPrim* prim, ElementPtr dest)
    {
        ElementPtr elem = dest->addChildOfCategory(prim->getPrimTypeName(), prim->getName());
        writeMetadata(prim, elem);

        for (auto child : prim->getChildren())
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
        writeMetadata(stage->getRootPrim(), doc);

        // Write out any dependent includes
        if (writeOptions && writeOptions->writeIncludes)
        {
            writeSourceUris(stage, doc);
        }

        RtWriteOptions::WriteFilter filter = writeOptions ? writeOptions->writeFilter : nullptr;
        for (auto prim : stage->getRootPrim()->getAllChildren())
        {
            if (!filter || filter(prim->obj()))
            {
                if (prim->getObjType() == PvtNodeDef::typeId())
                {
                    writeNodeDef(prim->asA<PvtNodeDef>(), doc);
                }
                else if (prim->getObjType() == PvtNode::typeId())
                {
                    PvtNode* node = prim->asA<PvtNode>();
                    NodePtr mxNode = writeNode(node, doc);
                    if (writeOptions && writeOptions->materialWriteOp & RtWriteOptions::MaterialWriteOp::WRITE)
                    {
                        const PvtAttribute* output = node->getAttribute(PvtAttribute::DEFAULT_OUTPUT_NAME);
                        if (output && output->getType() == RtType::SURFACESHADER)
                        {
                            writeMaterialElements(node, mxNode, node->getPrimTypeName(), doc, writeOptions);
                        }
                    }
                }
                else if (prim->getObjType() == PvtNodeGraph::typeId())
                {
                    writeNodeGraph(prim->asA<PvtNodeGraph>(), doc);
                }
                else
                {
                    writeGenericPrim(prim->asA<PvtPrim>(), doc->asA<Element>());
                }
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
    readSourceUri(doc, stage);

    // First, load all nodedefs. Having these available is needed
    // when node instances are loaded later.
    for (const NodeDefPtr& nodedef : doc->getNodeDefs())
    {
        PvtPath path(stage->getPath());
        path.push(RtToken(nodedef->getName()));
        if (!stage->getPrimAtPath(path))
        {
            readNodeDef(nodedef, stage);
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
