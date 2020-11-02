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
#include <MaterialXRuntime/RtLook.h>
#include <MaterialXRuntime/RtCollection.h>
#include <MaterialXRuntime/RtGeneric.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtTraversal.h>
#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtLogger.h>

#include <MaterialXRuntime/Private/PvtStage.h>

#include <MaterialXCore/Types.h>

#include <MaterialXFormat/Util.h>
#include <sstream>
#include <fstream>
#include <map>

namespace MaterialX
{

namespace
{
    // Lists of known metadata which are handled explicitly by import/export.
    static const RtTokenSet nodedefMetadata     = { RtToken("name"), RtToken("type"), RtToken("node") };
    static const RtTokenSet attrMetadata        = { RtToken("name"), RtToken("type"), RtToken("value"), RtToken("nodename"), RtToken("output"), RtToken("channels") };
    static const RtTokenSet inputMetadata       = { RtToken("name"), RtToken("type"), RtToken("value"), RtToken("nodename"), RtToken("output"), RtToken("channels"), 
                                                    RtToken("nodegraph"), RtToken("interfacename") };
    static const RtTokenSet nodeMetadata        = { RtToken("name"), RtToken("type"), RtToken("node") };
    static const RtTokenSet nodegraphMetadata   = { RtToken("name") };
    static const RtTokenSet lookMetadata        = { RtToken("name"), RtToken("inherit") };
    static const RtTokenSet lookGroupMetadata   = { RtToken("name"), RtToken("looks"), RtToken("default") };
    static const RtTokenSet mtrlAssignMetadata  = { RtToken("name"), RtToken("geom"), RtToken("collection"), RtToken("material"), RtToken("exclusive") };
    static const RtTokenSet collectionMetadata  = { RtToken("name"), RtToken("includegeom"), RtToken("includecollection"), RtToken("excludegeom") };
    static const RtTokenSet genericMetadata     = { RtToken("name"), RtToken("kind") };
    static const RtTokenSet stageMetadata       = {};

    static const RtToken DEFAULT_OUTPUT("out");
    static const RtToken OUTPUT_ELEMENT_PREFIX("OUT_");
    static const RtToken MULTIOUTPUT("multioutput");
    static const RtToken UI_VISIBLE("uivisible");
    static const RtToken SWIZZLE_INPUT("in");
    static const RtToken SWIZZLE_CHANNELS("channels");
    static const RtToken XPOS("xpos");
    static const RtToken YPOS("ypos");

    class PvtRenamingMapper
    {
        typedef RtTokenMap<RtToken> TokenToToken;
        typedef std::map<PvtPrim*, TokenToToken> PerPrimMap;

        PerPrimMap _map;
    public:
        void addMapping(PvtPrim* parent, const RtToken& originalName, const RtToken& finalName) {
            if (originalName != finalName) {
                _map[parent][originalName] = finalName;
            }
        }

        const RtToken& getFinalName(PvtPrim* parent, const RtToken& originalName) const {
            PerPrimMap::const_iterator primTarget = _map.find(parent);
            if (primTarget != _map.cend()) {
                const TokenToToken& nameMap = primTarget->second;
                TokenToToken::const_iterator nameTarget = nameMap.find(originalName);
                if (nameTarget != nameMap.cend()) {
                    return nameTarget->second;
                }
            }
            return originalName;
        }
    };

    PvtPrim* findPrimOrThrow(const RtToken& name, PvtPrim* parent, const PvtRenamingMapper& mapper)
    {
        PvtPrim* prim = parent->getChild(mapper.getFinalName(parent, name));
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
        PvtOutput* output = name.str().empty() ? prim->getOutput() : prim->getOutput(name);
        if (!output)
        {
            throw ExceptionRuntimeError("Node '" + prim->getName().str() + "' has no output named '" + name.str() + "'");
        }
        return output;
    }

    void readMetadata(const ElementPtr& src, PvtObject* dest, const RtTokenSet& ignoreList)
    {
        // Read in all metadata so we can export the element again
        // without loosing data.
        for (const string& name : src->getAttributeNames())
        {
            const RtToken mdName(name);
            if (!ignoreList.count(mdName))
            {
                // Store all generic metadata as strings.
                RtTypedValue* md = dest->addMetadata(mdName, RtType::STRING);
                md->getValue().asString() = src->getAttribute(name);
            }
        }
    }

    void writeMetadata(const PvtObject* src, ElementPtr dest, const RtTokenSet& ignoreList, const RtWriteOptions* options)
    {
        for (const RtToken name : src->getMetadataOrder())
        {
            if (ignoreList.count(name) ||
                (name.str().size() > 0 && name.str().at(0) == '_')) // Metadata with "_" prefix are private
            {
                continue;
            }
            const RtTypedValue* md = src->getMetadata(name);

            // Check filter if the metadata should be ignored
            if (options && options->metadataFilter && options->metadataFilter(src->hnd(), name, md))
            {
                continue;
            }

            std::string valueString = md->getValueString();
            if (!valueString.empty())
            {
                dest->setAttribute(name.str(), valueString);
            }
        }
    }

    template<class T>
    void createInterface(const ElementPtr& src, T schema)
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
                const uint32_t flags = elem->asA<Input>()->getIsUniform() ? RtAttrFlag::UNIFORM : 0;
                attr = schema.createInput(attrName, attrType, flags);
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

            readMetadata(elem, PvtObject::ptr<PvtObject>(attr), attrMetadata);
        }
    }

    void createConnection(PvtOutput* output, PvtInput* input, const string& swizzle, PvtStage* stage)
    {
        // Check if a swizzle node should be used in the connection.
        if (!swizzle.empty())
        {
            const RtToken swizzleNodeDefName("ND_swizzle_" + output->getType().str() + "_" + input->getType().str());
            const RtToken swizzleNodeName("swizzle_" + input->getParent()->getName().str() + "_" + input->getName().str());

            PvtPrim* parent = input->getParent()->getParent();
            PvtPrim* swizzleNode = stage->createPrim(parent->getPath(), swizzleNodeName, swizzleNodeDefName);

            PvtInput* in = swizzleNode->getInput(SWIZZLE_INPUT);
            PvtInput* ch = swizzleNode->getInput(SWIZZLE_CHANNELS);
            PvtOutput* out = swizzleNode->getOutput(DEFAULT_OUTPUT);
            if (in && ch && out)
            {
                ch->getValue().asString() = swizzle;
                output->connect(in);
                output = out;
            }
        }

        // Make the connection
        output->connect(input);
    }

    void createNodeConnections(const vector<NodePtr>& nodeElements, PvtPrim* parent, PvtStage* stage, const PvtRenamingMapper& mapper)
    {
        for (const NodePtr& nodeElem : nodeElements)
        {
            PvtPrim* node = findPrimOrThrow(RtToken(nodeElem->getName()), parent, mapper);
            for (const InputPtr& elemInput : nodeElem->getInputs())
            {
                PvtInput* input = findInputOrThrow(RtToken(elemInput->getName()), node);
                string connectedNodeName = elemInput->getNodeName();
                if (connectedNodeName.empty())
                {
                    connectedNodeName = elemInput->getNodeGraphString();
                }
                if (!connectedNodeName.empty())
                {
                    PvtPrim* connectedNode = findPrimOrThrow(RtToken(connectedNodeName), parent, mapper);
                    RtToken outputName(elemInput->getOutputString());
                    if (outputName == EMPTY_TOKEN && connectedNode)
                    {
                        RtNode rtConnectedNode(connectedNode->hnd());
                        auto output = rtConnectedNode.getOutput();
                        if (output)
                        {
                            outputName = output.getName();
                        }
                    }
                    PvtOutput* output = findOutputOrThrow(outputName, connectedNode);

                    createConnection(output, input, elemInput->getChannels(), stage);
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
            PvtOutput* out = prim->getOutput();
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
            if (candidate.getNamespacedNode() == nodeName && 
                matchingSignature(PvtObject::ptr<PvtPrim>(masterPrim), nodeType, nodePorts))
            {
                return candidate.getName();
            }
        }

        return EMPTY_TOKEN;
    }

    PvtPrim* readNode(const NodePtr& src, PvtPrim* parent, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        const RtToken nodedefName = resolveNodeDefName(src);
        if (nodedefName == EMPTY_TOKEN)
        {
            throw ExceptionRuntimeError("No matching nodedef was found for node '" + src->getName() + "'");
        }

        const RtToken nodeName(src->getName());
        PvtPrim* node = stage->createPrim(parent->getPath(), nodeName, nodedefName);
        mapper.addMapping(parent, nodeName, node->getName());

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
            readMetadata(elem, input, attrMetadata);
        }

        return node;
    }

    PvtPrim* readNodeGraph(const NodeGraphPtr& src, PvtPrim* parent, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        const RtToken nodegraphName(src->getName());

        PvtPrim* nodegraph = stage->createPrim(parent->getPath(), nodegraphName, RtNodeGraph::typeName());
        mapper.addMapping(parent, nodegraphName, nodegraph->getName());
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

        // Create all nodes and connections between node inputs and internal graph sockets.
        for (auto child : src->getChildren())
        {
            NodePtr srcNnode = child->asA<Node>();
            if (srcNnode)
            {
                PvtPrim* node = readNode(srcNnode, nodegraph, stage, mapper);

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
                        const RtToken socketName(interfaceName);
                        RtOutput socket = schema.getInputSocket(socketName);
                        if (!socket)
                        {
                            const RtToken inputType(elem->getType());
                            RtInput input = schema.createInput(socketName, inputType);
                            socket = schema.getInputSocket(input.getName());

                            // Set the input value
                            const string& valueStr = elem->getValueString();
                            if (!valueStr.empty())
                            {
                                const RtToken portType(elem->getType());
                                RtValue::fromString(portType, valueStr, input.getValue());
                            }
                        }

                        PvtOutput* output = PvtObject::ptr<PvtOutput>(socket);
                        const RtToken inputName(elem->getName());
                        PvtInput* input = findInputOrThrow(inputName, node);
                        const string& swizzle = elem->isA<Input>() ? elem->asA<Input>()->getChannels() : EMPTY_STRING;

                        createConnection(output, input, swizzle, stage);
                    }
                }
            }
        }

        // Create connections between all nodes.
        createNodeConnections(src->getNodes(), nodegraph, stage, mapper);

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

                PvtPrim* connectedNode = findPrimOrThrow(RtToken(connectedNodeName), nodegraph, mapper);

                const RtToken outputName(elem->getOutputString());
                PvtOutput* output = findOutputOrThrow(outputName, connectedNode);
                PvtInput* input = PvtObject::ptr<PvtInput>(socket);
                const string& swizzle = elem->getChannels();

                createConnection(output, input, swizzle, stage);
            }
        }

        return nodegraph;
    }

    PvtPrim* readGenericPrim(const ElementPtr& src, PvtPrim* parent, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        const RtToken name(src->getName());
        const RtToken category(src->getCategory());

        PvtPrim* prim = stage->createPrim(parent->getPath(), name, RtGeneric::typeName());
        mapper.addMapping(parent, name, prim->getName());
        RtGeneric generic(prim->hnd());
        generic.setKind(category);

        readMetadata(src, prim, genericMetadata);

        for (auto child : src->getChildren())
        {
            readGenericPrim(child, prim, stage, mapper);
        }

        return prim;
    }

    // Note that this function reads in a single collection. After all required collections
    // have been read in, the createCollectionConnections() function can be called
    // to create collection inclusion connections.
    PvtPrim* readCollection(const CollectionPtr& src, PvtPrim* parent, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        const RtToken name(src->getName());

        PvtPrim* collectionPrim = stage->createPrim(parent->getPath(), name, RtCollection::typeName());
        mapper.addMapping(parent, name, collectionPrim->getName());
        RtCollection collection(collectionPrim->hnd());
        collection.getIncludeGeom().setValueString(src->getIncludeGeom());
        collection.getExcludeGeom().setValueString(src->getExcludeGeom());

        readMetadata(src, collectionPrim, collectionMetadata);

        return collectionPrim;
    }

    // Create collection include connections assuming that all referenced
    // looks exist.
    void makeCollectionIncludeConnections(const vector<CollectionPtr>& collectionElements, PvtPrim* parent, const PvtRenamingMapper& mapper)
    {
        for (const CollectionPtr& colElement : collectionElements)
        {
            PvtPrim* parentCollection = findPrimOrThrow(RtToken(colElement->getName()), parent, mapper);
            for (const CollectionPtr& includeCollection : colElement->getIncludeCollections())
            {
                PvtPrim* childCollection = findPrimOrThrow(RtToken(includeCollection->getName()), parent, mapper);
                RtCollection rtCollection(parentCollection->hnd());
                rtCollection.addCollection(childCollection->hnd());
            }
        }
    }

    // Note that this function reads in a single look. After all required looks have been
    // read in then createLookConnections() can be called to create look inheritance
    // connections.
    PvtPrim* readLook(const LookPtr& src, PvtPrim* parent, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        const RtToken name(src->getName());

        PvtPrim* lookPrim = stage->createPrim(parent->getPath(), name, RtLook::typeName());
        mapper.addMapping(parent, name, lookPrim->getName());
        RtLook look(lookPrim->hnd());

        // Create material assignments
        for (const MaterialAssignPtr matAssign : src->getMaterialAssigns())
        {
            const RtToken matAssignName(matAssign->getName());
            PvtPrim* assignPrim = stage->createPrim(parent->getPath(), matAssignName, RtMaterialAssign::typeName());
            mapper.addMapping(parent, matAssignName, assignPrim->getName());
            RtMaterialAssign rtMatAssign(assignPrim->hnd());
            
            if (!matAssign->getCollectionString().empty())
            {
                PvtPrim* collection = findPrimOrThrow(RtToken(matAssign->getCollectionString()), parent, mapper);
                rtMatAssign.getCollection().addTarget(collection->hnd());
            }

            if (!matAssign->getMaterial().empty())
            {
                PvtPrim* material = findPrimOrThrow(RtToken(matAssign->getMaterial()), parent, mapper);
                rtMatAssign.getMaterial().connect(material->prim().getOutput());
            }

            if (matAssign->hasAttribute(MaterialAssign::EXCLUSIVE_ATTRIBUTE)) {
                rtMatAssign.getExclusive().getValue().asBool() = matAssign->getExclusive();
            } else {
                rtMatAssign.getExclusive().getValue().asBool() = true; // default
            }

            rtMatAssign.getGeom().getValue().asString() = matAssign->getActiveGeom();

            readMetadata(matAssign, assignPrim, mtrlAssignMetadata);

            look.getMaterialAssigns().addTarget(assignPrim->hnd());
        }

        readMetadata(src, lookPrim, lookMetadata);

        return lookPrim;
    }

    // Create look inheritance connections assuming that all referenced
    // looks exist.
    void makeLookInheritConnections(const vector<LookPtr>& lookElements, PvtPrim* parent, const PvtRenamingMapper& mapper)
    {
        for (const LookPtr& lookElem : lookElements)
        {
            PvtPrim* childLook = findPrimOrThrow(RtToken(lookElem->getName()), parent, mapper);
            const string& inheritString = lookElem->getInheritString();
            if (!inheritString.empty())
            {
                PvtPrim* parentLook = findPrimOrThrow(RtToken(inheritString), parent, mapper);
                RtLook rtLook(childLook->hnd());
                rtLook.getInherit().addTarget(parentLook->hnd());
            }
        }
    }

    // Read in a look group. This assumes that all referenced looks have
    // already been created.
    PvtPrim* readLookGroup(const LookGroupPtr& src, PvtPrim* parent, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        const string LIST_SEPARATOR(",");

        const RtToken name(src->getName());
        PvtPrim* prim = stage->createPrim(parent->getPath(), name, RtLookGroup::typeName());
        mapper.addMapping(parent, name, prim->getName());
        RtLookGroup lookGroup(prim->hnd());

        // Link to looks
        const string& lookNamesString = src->getLooks();
        StringVec lookNamesList  = splitString(lookNamesString, LIST_SEPARATOR);
        for (auto lookName : lookNamesList)
        {
            if (!lookName.empty())
            {
                PvtPrim* lookPrim = findPrimOrThrow(RtToken(lookName), parent, mapper);
                lookGroup.addLook(lookPrim->hnd());
            }
        }
        const string& activeLook = src->getActiveLook();
        lookGroup.getActiveLook().setValueString(activeLook);

        readMetadata(src, prim, lookGroupMetadata);

        return prim;
    }

    // Read in all look information from a document. Collections, looks and
    // look groups are read in first. Then relationship linkages are made.
    void readLookInformation(const DocumentPtr& doc, PvtStage* stage, const RtReadOptions* options, PvtRenamingMapper& mapper)
    {
        RtReadOptions::ElementFilter filter = options ? options->elementFilter : nullptr;

        PvtPrim* rootPrim = stage->getRootPrim();

        // Read collections
        for (const ElementPtr& elem : doc->getCollections())
        {
            if (!filter || filter(elem))
            {
                readCollection(elem->asA<Collection>(), rootPrim, stage, mapper);
            }
        }

        // Read looks
        for (const LookPtr& elem : doc->getLooks())
        {
            if (!filter || filter(elem))
            {
                readLook(elem, rootPrim, stage, mapper);
            }
        }

        // Read look groups
        for (const LookGroupPtr& elem : doc->getLookGroups())
        {
            if (!filter || filter(elem))
            {
                readLookGroup(elem, rootPrim, stage, mapper);
            }
        }

        // Create additional connections
        makeCollectionIncludeConnections(doc->getCollections(), rootPrim, mapper);
        makeLookInheritConnections(doc->getLooks(), rootPrim, mapper);
    }

    void validateNodesHaveNodedefs(DocumentPtr doc)
    {
        for (auto elem : doc->getChildren())
        {
            if (elem->isA<Node>())
            {
                NodePtr node = elem->asA<Node>();
                const RtToken nodedefName = resolveNodeDefName(node);
                if (nodedefName == EMPTY_TOKEN)
                {
                    throw ExceptionRuntimeError("No matching nodedef was found for node '" + node->getName() + "'");
                }
            }
        }
    }

    void readDocument(const DocumentPtr& doc, PvtStage* stage, const RtReadOptions* options)
    {
        // Set the source location 
        const std::string& uri = doc->getSourceUri();
        stage->addSourceUri(RtToken(uri));

        readMetadata(doc, stage->getRootPrim(), stageMetadata);

        RtReadOptions::ElementFilter filter = options ? options->elementFilter : nullptr;

        // First, load and register all nodedefs.
        // Having these available is needed when node instances are loaded later.
        for (const NodeDefPtr& nodedef : doc->getNodeDefs())
        {
            if (!filter || filter(nodedef))
            {
                if (!RtApi::get().hasMasterPrim(RtToken(nodedef->getName())))
                {
                    PvtPrim* prim = readNodeDef(nodedef, stage);
                    RtNodeDef(prim->hnd()).registerMasterPrim();
                }
            }
        }

        validateNodesHaveNodedefs(doc);

        // Keep track of renamed nodes:
        PvtRenamingMapper mapper;

        // Load all other elements.
        for (auto elem : doc->getChildren())
        {
            if (!filter || filter(elem))
            {
                if (elem->isA<Node>())
                {
                    readNode(elem->asA<Node>(), stage->getRootPrim(), stage, mapper);
                }
                else if (elem->isA<NodeGraph>())
                {
                    // Always skip if the nodegraph implements a nodedef
                    PvtPath path(PvtPath::ROOT_NAME.str() + elem->getName());
                    if (stage->getPrimAtPath(path) && elem->asA<NodeGraph>()->getNodeDef())
                    {
                        continue;
                    }
                    readNodeGraph(elem->asA<NodeGraph>(), stage->getRootPrim(), stage, mapper);
                }
                else
                {
                    const RtToken category(elem->getCategory());
                    if (category != RtLook::typeName() &&
                        category != RtLookGroup::typeName() &&
                        category != RtMaterialAssign::typeName() &&
                        category != RtCollection::typeName() &&
                        category != RtNodeDef::typeName()) {
                        readGenericPrim(elem, stage->getRootPrim(), stage, mapper);
                    }
                }
            }
        }

        // Create connections between all root level nodes.
        createNodeConnections(doc->getNodes(), stage->getRootPrim(), stage, mapper);

        // Read look information
        if (!options || options->readLookInformation)
        {
            readLookInformation(doc, stage, options, mapper);
        }
    }

    void writeNodeDef(const PvtPrim* src, DocumentPtr dest, const RtWriteOptions* options)
    {
        RtNodeDef nodedef(src->hnd());

        NodeDefPtr destNodeDef = dest->addNodeDef(nodedef.getName(), EMPTY_STRING, nodedef.getNode());
        writeMetadata(src, destNodeDef, nodedefMetadata, options);

        bool writeUniformsAsParameters = options ? options->writeUniformsAsParameters : false;
        for (const PvtDataHandle attrH : src->getAllAttributes())
        {
            const PvtAttribute* attr = attrH->asA<PvtAttribute>();

            ValueElementPtr destPort;
            if (attr->isA<PvtInput>())
            {
                const PvtInput* input = attr->asA<PvtInput>();
                if (input->isUniform())
                {
                    if (writeUniformsAsParameters)
                    {
                        destPort = destNodeDef->addParameter(attr->getName(), attr->getType().str());
                    }
                    else
                    {
                        destPort = destNodeDef->addInput(attr->getName(), attr->getType().str());
                        destPort->setIsUniform(true);
                    }
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
            writeMetadata(attr, destPort, attrMetadata, options);
        }
    }

    NodePtr writeNode(const PvtPrim* src, GraphElementPtr dest, const RtWriteOptions* options)
    {
        RtNode node(src->hnd());
        RtNodeDef nodedef(node.getNodeDef());
        if (!nodedef)
        {
            throw ExceptionRuntimeError("Prim '" + src->getName().str() + "' is not a node with a valid nodedef");
        }

        // Count output and get output type
        size_t numOutputs = 0;
        string outputType;
        RtObjTypePredicate<RtOutput> outputFilter;
        for (RtAttribute attr : nodedef.getPrim().getAttributes(outputFilter))
        {
            numOutputs++;
            outputType = attr.getType();
        }

        bool writeDefaultValues = options ? options->writeDefaultValues : false;
        bool writeUniformsAsParameters = options ? options->writeUniformsAsParameters : false;

        NodePtr destNode = dest->addNode(nodedef.getNamespacedNode(), node.getName(), numOutputs > 1 ? "multioutput" : outputType);

        for (RtAttribute attrDef : nodedef.getPrim().getAttributes())
        {
            RtAttribute attr = node.getPrim().getAttribute(attrDef.getName());
            RtInput input = attr.asA<RtInput>();
            if (input)
            {
                const RtTypedValue* uiVisible1 = input.getMetadata(UI_VISIBLE);
                const RtTypedValue* uiVisible2 = attrDef.getMetadata(UI_VISIBLE);
                const bool uiHidden1 = uiVisible1 && (uiVisible1->getValueString() == VALUE_STRING_FALSE);
                const bool uiHidden2 = uiVisible2 && (uiVisible2->getValueString() == VALUE_STRING_FALSE);
                const bool writeUiVisibleData = uiHidden1 != uiHidden2;

                // Write input if it's connected or different from default value.
                // Write input if the uivisible value differs in the input from the nodedef
                if (writeDefaultValues || writeUiVisibleData ||
                    input.isConnected() || !RtValue::compare(input.getType(), input.getValue(), attrDef.getValue()))
                {
                    ValueElementPtr valueElem;
                    if (input.isUniform())
                    {
                        if (writeUniformsAsParameters)
                        {
                            valueElem = destNode->addParameter(input.getName(), input.getType());
                        }
                        else
                        {
                            valueElem = destNode->addInput(input.getName(), input.getType());
                            valueElem->setIsUniform(true);
                        }
                        if (input.isConnected())
                        {
                            RtOutput source = input.getConnection();
                            if (source.isSocket())
                            {
                                // This is a connection to the internal socket of a graph
                                valueElem->setInterfaceName(source.getName());
                            }
                        }
                        const string& inputValueString = input.getValueString(); 
                        if (!inputValueString.empty())
                        {
                            valueElem->setValueString(inputValueString);
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
                                const string& inputValueString = input.getValueString();
                                if (!inputValueString.empty())
                                {
                                    valueElem->setValueString(inputValueString);
                                }
                            }
                            else
                            {
                                RtPrim sourcePrim = source.getParent();
                                InputPtr inputElem = valueElem->asA<Input>();
                                if (sourcePrim.hasApi<RtNodeGraph>())
                                {
                                    inputElem->setNodeGraphString(sourcePrim.getName());
                                }
                                else
                                {
                                    inputElem->setNodeName(sourcePrim.getName());
                                }
                                if (sourcePrim.numOutputs() > 1)
                                {
                                    inputElem->setOutputString(source.getName());
                                }
                            }
                        }
                        else
                        {
                            valueElem->setValueString(input.getValueString());
                        }
                    }

                    writeMetadata(PvtObject::ptr<PvtObject>(attr), valueElem, inputMetadata, options);
                }
            }
            else if(numOutputs > 1)
            {
                destNode->addOutput(attr.getName(), attr.getType());
            }
        }

        writeMetadata(src, destNode, nodeMetadata, options);

        return destNode;
    }

    void writeNodeGraph(const PvtPrim* src, DocumentPtr dest, const RtWriteOptions* options)
    {
        NodeGraphPtr destNodeGraph = dest->addNodeGraph(src->getName());
        writeMetadata(src, destNodeGraph, nodegraphMetadata, options);

        RtNodeGraph nodegraph(src->hnd());

        if (!options || options->writeNodeGraphInputs)
        {
            bool writeUniformsAsParameters = options ? options->writeUniformsAsParameters : false;

            // Write inputs/parameters.
            RtObjTypePredicate<RtInput> inputsFilter;
            for (RtAttribute attr : src->getAttributes(inputsFilter))
            {
                RtInput nodegraphInput = nodegraph.getInput(attr.getName());
                ValueElementPtr v = nullptr;
                if (nodegraphInput.isUniform())
                {
                    if (writeUniformsAsParameters)
                    {
                        v = destNodeGraph->addParameter(nodegraphInput.getName(), nodegraphInput.getType());
                    }
                    else
                    {
                        v = destNodeGraph->addInput(nodegraphInput.getName(), nodegraphInput.getType());
                        v->setIsUniform(true);
                    }
                }
                else
                {
                    InputPtr input = destNodeGraph->addInput(nodegraphInput.getName(), nodegraphInput.getType());
                    v = input->asA<ValueElement>();

                    if (nodegraphInput.isConnected())
                    {
                        // Write connections to upstream nodes.
                        RtOutput source = nodegraphInput.getConnection();
                        RtNode sourceNode = source.getParent();
                        input->setNodeName(sourceNode.getName());
                        if (sourceNode.numOutputs() > 1)
                        {
                            input->setOutputString(source.getName());
                        }
                    }
                }
                if (v)
                {
                    v->setValueString(nodegraphInput.getValueString());
                    writeMetadata(PvtObject::ptr<PvtObject>(attr), v, inputMetadata, options);
                }
            }
        }

        // Write nodes.
        for (RtPrim node : nodegraph.getNodes())
        {
            writeNode(PvtObject::ptr<PvtPrim>(node), destNodeGraph, options);
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
                    RtNode sourceNode = source.getParent();
                    output->setNodeName(sourceNode.getName());
                    if (sourceNode.numOutputs() > 1)
                    {
                        output->setOutputString(source.getName());
                    }
                }
            }
        }
    }

    void writeCollections(PvtStage* stage, Document& dest, const RtWriteOptions* options)
    {
        for (RtPrim child : stage->getRootPrim()->getChildren(options ? options->objectFilter : nullptr))
        {
            const PvtPrim* prim = PvtObject::ptr<PvtPrim>(child);
            const RtToken typeName = child.getTypeInfo()->getShortTypeName();
            if (typeName == RtCollection::typeName())
            {
                RtCollection rtCollection(prim->hnd());
                const string name(prim->getName());

                if (dest.getCollection(name))
                {
                    continue;
                }

                CollectionPtr collection = dest.addCollection(name);

                collection->setExcludeGeom(rtCollection.getExcludeGeom().getValueString());
                collection->setIncludeGeom(rtCollection.getIncludeGeom().getValueString());

                RtRelationship rtIncludeCollection = rtCollection.getIncludeCollection();
                string includeList = rtIncludeCollection.getTargetsAsString();                
                collection->setIncludeCollectionString(includeList);

                writeMetadata(prim, collection, collectionMetadata, options);
            }
        }
    }

    void writeLooks(PvtStage* stage, Document& dest, const RtWriteOptions* options)
    {
        for (RtPrim child : stage->getRootPrim()->getChildren(options ? options->objectFilter : nullptr))
        {
            const PvtPrim* prim = PvtObject::ptr<PvtPrim>(child);
            const RtToken typeName = child.getTypeInfo()->getShortTypeName();
            if (typeName == RtLook::typeName())
            {
                RtLook rtLook(prim->hnd());
                const string name(prim->getName());

                if (dest.getCollection(name))
                {
                    continue;
                }

                LookPtr look = dest.addLook(name);

                // Add inherit
                if (!rtLook.getInherit().getTargetsAsString().empty())
                {
                    look->setInheritString(rtLook.getInherit().getTargetsAsString());
                }

                // Add in material assignments
                for (const RtObject& obj : rtLook.getMaterialAssigns().getTargets())
                {
                    PvtPrim* pprim = PvtObject::ptr<PvtPrim>(obj);
                    RtMaterialAssign rtMatAssign(pprim->hnd());
                    const string& assignName = rtMatAssign.getName();
                    if (look->getMaterialAssign(assignName))
                    {
                        continue;
                    }

                    MaterialAssignPtr massign = look->addMaterialAssign(assignName);
                    massign->setExclusive(rtMatAssign.getExclusive().getValue().asBool());
                    massign->setGeom(rtMatAssign.getGeom().getValueString());

                    auto iter = rtMatAssign.getCollection().getTargets();
                    if (!iter.isDone())
                    {
                        massign->setCollectionString((*iter).getName());
                    }

                    if (rtMatAssign.getMaterial().isConnected())
                    {
                        massign->setMaterial(rtMatAssign.getMaterial().getConnection().getParent().getName());
                    }

                    writeMetadata(pprim, massign, mtrlAssignMetadata, options);
                }

                writeMetadata(prim, look, lookMetadata, options);
            }
        }
    }

    void writeLookGroups(PvtStage* stage, Document& dest, const RtWriteOptions* options)
    {
        for (RtPrim child : stage->getRootPrim()->getChildren(options ? options->objectFilter : nullptr))
        {
            const PvtPrim* prim = PvtObject::ptr<PvtPrim>(child);
            const RtToken typeName = child.getTypeInfo()->getShortTypeName();
            if (typeName == RtLookGroup::typeName())
            {
                RtLookGroup rtLookGroup(prim->hnd());
                const string name(rtLookGroup.getName());

                if (dest.getLookGroup(name))
                {
                    continue;
                }

                LookGroupPtr lookGroup = dest.addLookGroup(name);

                string lookList = rtLookGroup.getLooks().getTargetsAsString();
                lookGroup->setLooks(lookList);
                lookGroup->setActiveLook(rtLookGroup.getActiveLook().getValueString());

                writeMetadata(prim, lookGroup, lookGroupMetadata, options);
            }
        }
    }

    void writeGenericPrim(const PvtPrim* src, ElementPtr dest, const RtWriteOptions* options)
    {
        RtGeneric generic(src->hnd());

        ElementPtr elem = dest->addChildOfCategory(generic.getKind(), generic.getName());
        writeMetadata(src, elem, genericMetadata, options);

        for (auto child : src->getChildren())
        {
            writeGenericPrim(PvtObject::ptr<PvtPrim>(child), elem, options);
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
            const RtTokenVec& uris = ref->getSourceUri();
            if (!uris.empty())
            {
                for (const RtToken& uri : uris)
                {
                    prependXInclude(doc, uri.str());
                }
            }
        }
    }

    void writeDocument(DocumentPtr& doc, PvtStage* stage, const RtWriteOptions* options)
    {
        writeMetadata(stage->getRootPrim(), doc, RtTokenSet(), options);

        // Write out any dependent includes
        if (options && options->writeIncludes)
        {
            writeSourceUris(stage, doc);
        }

        std::vector<NodePtr> materialElements;
        for (RtPrim child : stage->getRootPrim()->getChildren(options ? options->objectFilter : nullptr))
        {
            const PvtPrim* prim = PvtObject::ptr<PvtPrim>(child);
            const RtToken typeName = child.getTypeInfo()->getShortTypeName();
            if (typeName == RtNodeDef::typeName())
            {
                writeNodeDef(prim, doc, options);
            }
            else if (typeName == RtNode::typeName())
            {
                writeNode(prim, doc, options);
            }
            else if (typeName == RtNodeGraph::typeName())
            {
                writeNodeGraph(prim, doc, options);
            }
            else if (typeName == RtBackdrop::typeName())
            {
                //writeBackdrop(prim, doc)
            }
            else if (typeName != RtLook::typeName() &&
                     typeName != RtLookGroup::typeName() &&
                     typeName != RtMaterialAssign::typeName() &&
                     typeName != RtCollection::typeName())
            {
                writeGenericPrim(prim, doc->asA<Element>(), options);
            }
        }

        // Write the existing look information
        writeCollections(stage, *doc, options);
        writeLooks(stage, *doc, options);
        writeLookGroups(stage, *doc, options);
    }

    void readUnitDefinitions(DocumentPtr doc)
    {
        RtApi& api = RtApi::get();
        UnitConverterRegistryPtr unitDefinitions = api.getUnitDefinitions();
        for (UnitTypeDefPtr unitTypeDef : doc->getUnitTypeDefs())
        {
            LinearUnitConverterPtr unitConvert = LinearUnitConverter::create(unitTypeDef);
            unitDefinitions->addUnitConverter(unitTypeDef, unitConvert);
        }
    }

    void writeMasterPrim(DocumentPtr document, PvtStage* stage, PvtPrim* prim, const RtWriteOptions* options)
    {
        if (!prim || prim->isDisposed())
        {
            throw ExceptionRuntimeError("Trying to write invalid definition" +  (prim ? (": '" + prim->getName().str() + "'") :  EMPTY_STRING));
        }

        // Write the definition
        writeNodeDef(prim, document, options);

        // Write the corresponding nodegraph if any.
        // Currently there is no "implementation" association kept other than
        // on the node graph referencing the definition it represents.
        //
        // TODO: Want to change this to keep this in <implementation>
        // elements but requires a spec change plus support in the runtime
        // for implementation associations.
        RtNodeDef nodedef(prim->hnd());
        RtToken nodeDefName = prim->getName();
        RtSchemaPredicate<RtNodeGraph> filter;
        for (RtPrim child : stage->getRootPrim()->getChildren(filter))
        {
            // The association between a nodedef and a nodegraph is by name. No
            // version check is required as nodegraphs are not versioned.
            RtNodeGraph nodeGraph(child);
            if (nodeGraph.getDefinition() == nodeDefName)
            {
                PvtPrim* graphPrim = PvtObject::ptr<PvtPrim>(child);
                writeNodeGraph(graphPrim, document, options);
                break;
            }
        }
    }

    void writeNodeDefs(DocumentPtr document, PvtStage* stage, const RtTokenVec& names, const RtWriteOptions* options)
    {
        // Write all definitions if no names provided
        RtApi& rtApi = RtApi::get();
        if (names.empty())
        {
            RtSchemaPredicate<RtNodeDef> nodedefFilter;
            for (RtPrim masterPrim : rtApi.getMasterPrims(nodedefFilter))
            {
                PvtPrim* prim = PvtObject::ptr<PvtPrim>(masterPrim);
                writeMasterPrim(document, stage, prim, options);
            }
        }
        else
        {
            for (const RtToken& name : names)
            {
                RtPrim masterPrim = rtApi.getMasterPrim(name);
                PvtPrim* prim = PvtObject::ptr<PvtPrim>(masterPrim);
                writeMasterPrim(document, stage, prim, options);
            }
        }      
    }

} // end anonymous namespace

RtReadOptions::RtReadOptions() :
    elementFilter(nullptr),
    readLookInformation(false),
    applyFutureUpdates(true)
{
}

RtWriteOptions::RtWriteOptions() :
    writeIncludes(true),
    writeNodeGraphInputs(true),
    writeDefaultValues(false),
    objectFilter(nullptr),
    metadataFilter(nullptr),
    desiredMajorVersion(MATERIALX_MAJOR_VERSION),
    desiredMinorVersion(MATERIALX_MINOR_VERSION),
    writeUniformsAsParameters(false)
{
}

void RtFileIo::read(const FilePath& documentPath, const FileSearchPath& searchPaths, const RtReadOptions* options)
{
    try
    {
        DocumentPtr document = createDocument();
        XmlReadOptions xmlReadOptions;
        if (options)
        {
            xmlReadOptions.applyFutureUpdates = options->applyFutureUpdates;
        }
        readFromXmlFile(document, documentPath, searchPaths, &xmlReadOptions);

        PvtStage* stage = PvtStage::ptr(_stage);
        readDocument(document, stage, options);
    }
    catch (Exception& e)
    {
        throw ExceptionRuntimeError("Could not read file: " + documentPath.asString() + ". Error: " + e.what());
    }
}

void RtFileIo::read(std::istream& stream, const RtReadOptions* options)
{
    try
    {
        DocumentPtr document = createDocument();
        XmlReadOptions xmlReadOptions;
        if (options)
        {
            xmlReadOptions.applyFutureUpdates = options->applyFutureUpdates;
        }
        readFromXmlStream(document, stream, &xmlReadOptions);

        PvtStage* stage = PvtStage::ptr(_stage);
        readDocument(document, stage, options);
    }
    catch (Exception& e)
    {
        throw ExceptionRuntimeError(string("Could not read from stream. Error: ") + e.what());
    }
}

void RtFileIo::readLibraries(const FilePathVec& libraryPaths, const FileSearchPath& searchPaths, const RtReadOptions& options)
{
    PvtStage* stage = PvtStage::ptr(_stage);

    // Load all content into a document.
    DocumentPtr doc = createDocument();
    MaterialX::XmlReadOptions readOptions;
    readOptions.applyFutureUpdates = options.applyFutureUpdates;
    MaterialX::loadLibraries(libraryPaths, searchPaths, doc, MaterialX::StringSet(), &readOptions);

    StringSet uris = doc->getReferencedSourceUris();
    for (const string& uri : uris)
    {
        stage->addSourceUri(RtToken(uri));
    }

    // Update any units found
    readUnitDefinitions(doc);

    // First, load all nodedefs. Having these available is needed
    // when node instances are loaded later.
    for (const NodeDefPtr& nodedef : doc->getNodeDefs())
    {
        if (!RtApi::get().hasMasterPrim(RtToken(nodedef->getName())))
        {
            PvtPrim* prim = readNodeDef(nodedef, stage);
            RtNodeDef(prim->hnd()).registerMasterPrim();
        }
    }

    validateNodesHaveNodedefs(doc);

    // We were already renaming on conflict here. Keep track of the new names.
    PvtRenamingMapper mapper;

    // Second, load all other elements.
    for (auto elem : doc->getChildren())
    {
        PvtPath path(stage->getPath());
        path.push(RtToken(elem->getName()));

        if (elem->isA<NodeDef>() || stage->getPrimAtPath(path))
        {
            continue;
        }

        try
        {
            if (elem->isA<Node>())
            {
                readNode(elem->asA<Node>(), stage->getRootPrim(), stage, mapper); 
            }
            else if (elem->isA<NodeGraph>())
            {
                readNodeGraph(elem->asA<NodeGraph>(), stage->getRootPrim(), stage, mapper);
            }
            else
            {
                readGenericPrim(elem, stage->getRootPrim(), stage, mapper);
            }
        }
        catch(const ExceptionRuntimeError &e)
        {
            RtApi::get().log(RtLogger::ERROR, e.what());
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
        //document->setVersionString(makeVersionString(options->desiredMajorVersion, options->desiredMinorVersion));
    }
    else
    {
        //document->setVersionString(makeVersionString(MATERIALX_MAJOR_VERSION, MATERIALX_MINOR_VERSION + 1));
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
        //document->setVersionString(makeVersionString(options->desiredMajorVersion, options->desiredMinorVersion));
    }
    else
    {
        //document->setVersionString(makeVersionString(MATERIALX_MAJOR_VERSION, MATERIALX_MINOR_VERSION + 1));
    }
    writeToXmlStream(document, stream, &xmlWriteOptions);
}

void RtFileIo::writeDefinitions(std::ostream& stream, const RtTokenVec& names, const RtWriteOptions* options)
{
    DocumentPtr document = createDocument();
    PvtStage* stage = PvtStage::ptr(_stage);
    writeNodeDefs(document, stage, names, options);
    writeToXmlStream(document, stream);
}

void RtFileIo::writeDefinitions(const FilePath& documentPath, const RtTokenVec& names, const RtWriteOptions* options)
{
    std::ofstream ofs(documentPath.asString());
    writeDefinitions(ofs, names, options);
}

}
