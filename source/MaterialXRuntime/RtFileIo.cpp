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
#include <MaterialXRuntime/Tokens.h>
#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtTargetDef.h>
#include <MaterialXRuntime/Codegen/RtSourceCodeImpl.h>
#include <MaterialXRuntime/Codegen/RtSubGraphImpl.h>

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
    // Lists of attributes which are handled explicitly by read/write and should be ignored when reading/writing attributes.
    static const RtTokenSet nodedefIgnoreList    = { RtToken("name"), RtToken("type") };
    static const RtTokenSet portIgnoreList       = { RtToken("name"), RtToken("type"), RtToken("value"), RtToken("nodename"), RtToken("output"), RtToken("channels") };
    static const RtTokenSet inputIgnoreList      = { RtToken("name"), RtToken("type"), RtToken("value"), RtToken("nodename"), RtToken("output"), RtToken("channels"),
                                                     RtToken("nodegraph"), RtToken("interfacename") };
    static const RtTokenSet nodeIgnoreList       = { RtToken("name"), RtToken("type") };
    static const RtTokenSet nodegraphIgnoreList  = { RtToken("name") };
    static const RtTokenSet targetdefIgnoreList  = { RtToken("name"), RtToken("inherit") };
    static const RtTokenSet nodeimplIgnoreList   = { RtToken("name"), RtToken("file"), RtToken("sourcecode") };
    static const RtTokenSet lookIgnoreList       = { RtToken("name"), RtToken("inherit") };
    static const RtTokenSet lookGroupIgnoreList  = { RtToken("name"), RtToken("looks") };
    static const RtTokenSet mtrlAssignIgnoreList = { RtToken("name"), RtToken("geom"), RtToken("collection"), RtToken("material") };
    static const RtTokenSet collectionIgnoreList = { RtToken("name"), RtToken("includecollection") };
    static const RtTokenSet genericIgnoreList    = { RtToken("name"), RtToken("kind") };
    static const RtTokenSet stageIgnoreList      = {};

    static const RtToken MULTIOUTPUT("multioutput");
    static const RtToken SWIZZLE_INPUT("in");
    static const RtToken SWIZZLE_CHANNELS("channels");

    // Specification of attributes for the document root element.
    class PvtRootPrimSpec : public PvtPrimSpec
    {
    public:
        PvtRootPrimSpec()
        {
            // TODO: We should derive this from a data driven XML schema.
            addPrimAttribute(Tokens::DOC, RtType::STRING);
            addPrimAttribute(Tokens::COLORSPACE, RtType::TOKEN);
        }
    };

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

    void readAttributes(const ElementPtr& src, PvtObject* dest, const RtPrimSpec& primSpec, const RtTokenSet& ignoreList)
    {
        // Check if this is a port, we have a separate 
        // attribute spec call for ports below.
        const bool isAPort = dest->isA<PvtPort>();

        for (const string& nameStr : src->getAttributeNames())
        {
            const RtToken name(nameStr);
            if (!ignoreList.count(name))
            {
                // Check if this is an attribute defined by the spec.
                const RtAttributeSpec* attrSpec = isAPort ? primSpec.getPortAttribute(RtPort(dest->hnd()), name) : primSpec.getAttribute(name);
                if (attrSpec)
                {
                    // Create it according to the spec.
                    RtTypedValue* attr = dest->createAttribute(attrSpec->getName(), attrSpec->getType());
                    RtValue::fromString(attr->getType(), src->getAttribute(nameStr), attr->getValue());
                }
                else
                {
                    // Store all generic attributes as strings.
                    RtTypedValue* attr = dest->createAttribute(name, RtType::STRING);
                    attr->asString() = src->getAttribute(nameStr);
                }
            }
        }
    }

    void writeAttributes(const PvtObject* src, ElementPtr dest, const RtTokenSet& ignoreList, const RtWriteOptions* options)
    {
        for (const RtToken& name : src->getAttributeNames())
        {
            if (ignoreList.count(name) ||
                (name.str().size() > 0 && name.str().at(0) == '_')) // Attributes with "_" prefix are private
            {
                continue;
            }

            const RtTypedValue* attr = src->getAttribute(name);

            // Check filter if the attribute should be ignored
            if (options && options->attributeFilter && options->attributeFilter(src->hnd(), name, attr))
            {
                continue;
            }

            // Get the value as string to cover all attribute types.
            const string valueString = attr->getValueString();
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
            const RtToken portName(elem->getName());
            const RtToken portType(elem->getType());

            RtPort port;
            if (elem->isA<Output>())
            {
                port = schema.createOutput(portName, portType);
            }
            else if (elem->isA<Input>())
            {
                const uint32_t flags = elem->asA<Input>()->getIsUniform() ? RtPortFlag::UNIFORM : 0;
                port = schema.createInput(portName, portType, flags);
            }

            if (port)
            {
                const string& valueStr = elem->getValueString();
                if (!valueStr.empty())
                {
                    RtValue::fromString(portType, valueStr, port.getValue());
                }

                readAttributes(elem, PvtObject::ptr(port), schema.getPrimSpec(), portIgnoreList);
            }
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
            PvtOutput* out = swizzleNode->getOutput();
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

    void createNodeConnection(InterfaceElementPtr nodeElem, PvtPrim* parent, PvtStage* stage, const PvtRenamingMapper& mapper)
    {
        PvtPrim* node = findPrimOrThrow(RtToken(nodeElem->getName()), parent, mapper);
        for (InputPtr elemInput : nodeElem->getInputs())
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
                    RtOutput output = rtConnectedNode.getOutput();
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

    void createNodeConnections(const vector<NodePtr>& nodeElements, PvtPrim* parent, PvtStage* stage, const PvtRenamingMapper& mapper)
    {
        for (auto nodeElem : nodeElements)
        {
            createNodeConnection(nodeElem->asA<InterfaceElement>(), parent, stage, mapper);
        }
    }

    void createNodeGraphConnections(const vector<NodeGraphPtr>& nodeElements, PvtPrim* parent, PvtStage* stage, const PvtRenamingMapper& mapper)
    {
        for (auto nodeElem : nodeElements)
        {
            createNodeConnection(nodeElem->asA<InterfaceElement>(), parent, stage, mapper);
        }
    }

    PvtPrim* readNodeDef(const NodeDefPtr& src, PvtStage* stage)
    {
        const RtToken name(src->getName());
        PvtPrim* prim = stage->createPrim(stage->getPath(), name, RtNodeDef::typeName());

        RtNodeDef nodedef(prim->hnd());
        readAttributes(src, prim, nodedef.getPrimSpec(), nodedefIgnoreList);

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
        for (const ValueElementPtr& nodePort : nodePorts)
        {
            const RtToken name(nodePort->getName());
            const PvtPort* port = nodePort->isA<Input>() ? prim->getInput(name)->asA<PvtPort>() : prim->getOutput(name)->asA<PvtPort>();
            if (!port || port->getType().str() != nodePort->getType())
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

        // Third, try resolving among existing registered nodedefs.
        const RtToken nodeName(node->getCategory());
        const RtToken nodeType(node->getType());
        const vector<ValueElementPtr> nodePorts = node->getActiveValueElements();
        const size_t numNodeDefs = RtApi::get().numNodeDefs();
        for (size_t i=0; i<numNodeDefs; ++i)
        {
            RtPrim prim = RtApi::get().getNodeDef(i);
            RtNodeDef candidate(prim);
            if (candidate.getNamespacedNode() == nodeName && 
                matchingSignature(PvtObject::ptr<PvtPrim>(prim), nodeType, nodePorts))
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
        PvtPrim* prim = stage->createPrim(parent->getPath(), nodeName, nodedefName);
        mapper.addMapping(parent, nodeName, prim->getName());

        RtNode node(prim->hnd());
        readAttributes(src, prim, node.getPrimSpec(), nodeIgnoreList);

        // Copy input values.
        for (auto elem : src->getChildrenOfType<ValueElement>())
        {
            if (elem->isA<Output>())
            {
                continue;
            }
            const RtToken portName(elem->getName());
            PvtInput* input = prim->getInput(portName);
            if (!input)
            {
                throw ExceptionRuntimeError("No input named '" + elem->getName() + "' was found on runtime node '" + prim->getName().str() + "'");
            }
            const string& valueStr = elem->getValueString();
            if (!valueStr.empty())
            {
                const RtToken portType(elem->getType());
                RtValue::fromString(portType, valueStr, input->getValue());
            }
            readAttributes(elem, input, node.getPrimSpec(), portIgnoreList);
        }

        return prim;
    }

    PvtPrim* readNodeGraph(const NodeGraphPtr& src, PvtPrim* parent, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        const RtToken nodegraphName(src->getName());

        PvtPrim* prim = stage->createPrim(parent->getPath(), nodegraphName, RtNodeGraph::typeName());
        mapper.addMapping(parent, nodegraphName, prim->getName());

        RtNodeGraph nodegraph(prim->hnd());
        readAttributes(src, prim, nodegraph.getPrimSpec(), nodegraphIgnoreList);

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

        // Create all nodes and connections between node inputs and internal graph sockets.
        for (auto child : src->getChildren())
        {
            NodePtr srcNnode = child->asA<Node>();
            if (srcNnode)
            {
                PvtPrim* node = readNode(srcNnode, prim, stage, mapper);

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
                        RtOutput socket = nodegraph.getInputSocket(socketName);
                        if (!socket)
                        {
                            const RtToken inputType(elem->getType());
                            RtInput input = nodegraph.createInput(socketName, inputType);
                            socket = nodegraph.getInputSocket(input.getName());

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
        createNodeConnections(src->getNodes(), prim, stage, mapper);

        // Create connections between node outputs and internal graph sockets.
        for (const OutputPtr& elem : src->getOutputs())
        {
            const string& connectedNodeName = elem->getNodeName();
            if (!connectedNodeName.empty())
            {
                RtInput socket = nodegraph.getOutputSocket(RtToken(elem->getName()));
                if (!socket)
                {
                    PvtPath path(parent->getPath());
                    path.push(nodegraphName);
                    throw ExceptionRuntimeError("Output '" + elem->getName() + "' does not match an internal output socket on the nodegraph '" + path.asString() + "'");
                }

                PvtPrim* connectedNode = findPrimOrThrow(RtToken(connectedNodeName), prim, mapper);

                const RtToken outputName(elem->getOutputString());
                PvtOutput* output = findOutputOrThrow(outputName, connectedNode);
                PvtInput* input = PvtObject::ptr<PvtInput>(socket);
                const string& swizzle = elem->getChannels();

                createConnection(output, input, swizzle, stage);
            }
        }

        return prim;
    }

    PvtPrim* readGenericPrim(const ElementPtr& src, PvtPrim* parent, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        const RtToken name(src->getName());
        const RtToken category(src->getCategory());

        PvtPrim* prim = stage->createPrim(parent->getPath(), name, RtGeneric::typeName());
        mapper.addMapping(parent, name, prim->getName());

        RtGeneric generic(prim->hnd());
        generic.setKind(category);
        readAttributes(src, prim, generic.getPrimSpec(), genericIgnoreList);

        for (auto child : src->getChildren())
        {
            readGenericPrim(child, prim, stage, mapper);
        }

        return prim;
    }

    PvtPrim* readTargetDef(const TargetDefPtr& src, PvtPrim* parent, PvtStage* stage)
    {
        const RtToken name(src->getName());
        const RtToken inherit(src->getInheritString());

        PvtPrim* prim = stage->createPrim(parent->getPath(), name, RtTargetDef::typeName());

        RtTargetDef def(prim->hnd());
        if (inherit != EMPTY_TOKEN)
        {
            def.setInherit(inherit);
        }

        readAttributes(src, prim, def.getPrimSpec(), targetdefIgnoreList);

        return prim;
    }

    PvtPrim* readImplementation(const ImplementationPtr& src, PvtPrim* parent, PvtStage* stage)
    {
        const RtToken target(src->getAttribute(Tokens::TARGET.str()));

        // We are only interested in implementations for registered targets,
        // so if target is set make sure this target has been registered.
        if (target != EMPTY_TOKEN && !RtApi::get().hasTargetDef(target))
        {
            return nullptr;
        }

        const RtToken name(src->getName());

        const string& sourcecode = src->getAttribute(Tokens::SOURCECODE.str());
        const string& file = src->getAttribute(Tokens::FILE.str());

        PvtPrim* prim = nullptr;
        if (file.empty() && sourcecode.empty())
        {
            // Create a generic node implementation.
            prim = stage->createPrim(parent->getPath(), name, RtNodeImpl::typeName());
        }
        else
        {
            // Create a source code implementation.
            prim = stage->createPrim(parent->getPath(), name, RtSourceCodeImpl::typeName());

            RtSourceCodeImpl impl(prim->hnd());
            if (!sourcecode.empty())
            {
                impl.setSourceCode(sourcecode);
            }
            else
            {
                impl.setFile(file);
            }
        }

        RtNodeImpl impl(prim->hnd());
        readAttributes(src, prim, impl.getPrimSpec(), nodeimplIgnoreList);

        return prim;
    }

    // Note that this function reads in a single collection. After all required collections
    // have been read in, the makeCollectionIncludeConnections() function can be called
    // to create collection inclusion connections.
    PvtPrim* readCollection(const CollectionPtr& src, PvtPrim* parent, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        const RtToken name(src->getName());

        PvtPrim* prim = stage->createPrim(parent->getPath(), name, RtCollection::typeName());
        mapper.addMapping(parent, name, prim->getName());

        RtCollection collection(prim->hnd());
        readAttributes(src, prim, collection.getPrimSpec(), collectionIgnoreList);

        return prim;
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
    // read in then makeLookInheritConnections() can be called to create look inheritance
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
                rtMatAssign.getCollection().connect(collection->hnd());
            }

            if (!matAssign->getMaterial().empty())
            {
                PvtPrim* material = findPrimOrThrow(RtToken(matAssign->getMaterial()), parent, mapper);
                rtMatAssign.getMaterial().connect(material->prim().getOutput());
            }

            rtMatAssign.setGeom(matAssign->getActiveGeom());

            readAttributes(matAssign, assignPrim, rtMatAssign.getPrimSpec(), mtrlAssignIgnoreList);

            look.getMaterialAssigns().connect(assignPrim->hnd());
        }

        readAttributes(src, lookPrim, look.getPrimSpec(), lookIgnoreList);

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
                rtLook.getInherit().connect(parentLook->hnd());
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

        readAttributes(src, prim, lookGroup.getPrimSpec(), lookGroupIgnoreList);

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
        RtApi& api = RtApi::get();

        // Set the source location 
        const std::string& uri = doc->getSourceUri();
        stage->addSourceUri(RtToken(uri));

        // Read root document attributes.
        static PvtRootPrimSpec s_rootPrimSpec;
        readAttributes(doc, stage->getRootPrim(), s_rootPrimSpec, stageIgnoreList);

        RtReadOptions::ElementFilter filter = options ? options->elementFilter : nullptr;

        // Load and register all targetdefs.
        // Having these available is needed when implementations are loaded later.
        for (const TargetDefPtr& targetdef : doc->getTargetDefs())
        {
            if (!filter || filter(targetdef))
            {
                if (!api.hasTargetDef(RtToken(targetdef->getName())))
                {
                    PvtPrim* prim = readTargetDef(targetdef, stage->getRootPrim(), stage);
                    api.registerTargetDef(prim->hnd());
                }
            }
        }

        // Load and register all nodedefs.
        // Having these available is needed when node instances are loaded later.
        for (const NodeDefPtr& nodedef : doc->getNodeDefs())
        {
            if (!filter || filter(nodedef))
            {
                if (!api.hasNodeDef(RtToken(nodedef->getName())))
                {
                    PvtPrim* prim = readNodeDef(nodedef, stage);
                    api.registerNodeDef(prim->hnd());
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
                else if (elem->isA<Implementation>())
                {
                    PvtPrim* prim = readImplementation(elem->asA<Implementation>(), stage->getRootPrim(), stage);
                    if (prim)
                    {
                        api.registerNodeImpl(prim->hnd());
                    }
                }
                else
                {
                    const RtToken category(elem->getCategory());
                    if (category != RtLook::typeName() &&
                        category != RtLookGroup::typeName() &&
                        category != RtMaterialAssign::typeName() &&
                        category != RtCollection::typeName() &&
                        category != RtNodeDef::typeName())
                    {
                        readGenericPrim(elem, stage->getRootPrim(), stage, mapper);
                    }
                }
            }
        }

        // Create connections between all root level nodes.
        createNodeConnections(doc->getNodes(), stage->getRootPrim(), stage, mapper);

        // Create connections between all nodegraphs
        createNodeGraphConnections(doc->getNodeGraphs(), stage->getRootPrim(), stage, mapper);

        // Read look information
        if (!options || options->readLookInformation)
        {
            readLookInformation(doc, stage, options, mapper);
        }
    }

    void writeNodeDef(const PvtPrim* src, DocumentPtr dest, const RtWriteOptions* options)
    {
        RtNodeDef nodedef(src->hnd());
        NodeDefPtr destNodeDef = dest->addNodeDef(nodedef.getName().str(), EMPTY_STRING, nodedef.getNode().str());

        if (nodedef.getVersion() != EMPTY_TOKEN)
        {
            destNodeDef->setVersionString(nodedef.getVersion().str());
            if (nodedef.getIsDefaultVersion())
            {
                destNodeDef->setDefaultVersion(true);
            }
        }

        writeAttributes(src, destNodeDef, nodedefIgnoreList, options);

        for (PvtObject* obj : src->getInputs())
        {
            const PvtInput* input = obj->asA<PvtInput>();
            ValueElementPtr destPort = destNodeDef->addInput(input->getName().str(), input->getType().str());
            if (input->isUniform())
            {
                destPort->setIsUniform(true);
            }
            destPort->setValueString(input->getValueString());
            writeAttributes(input, destPort, portIgnoreList, options);
        }
        for (PvtObject* obj : src->getOutputs())
        {
            const PvtOutput* output = obj->asA<PvtOutput>();
            ValueElementPtr destPort = destNodeDef->addOutput(output->getName().str(), output->getType().str());
            destPort->setValueString(output->getValueString());
            writeAttributes(output, destPort, portIgnoreList, options);
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
        const size_t numOutputs = nodedef.getPrim().numOutputs();
        const string outputType = numOutputs > 1 ? "multioutput" : (numOutputs > 0 ? nodedef.getPrim().getOutput().getType().str() : EMPTY_STRING);

        NodePtr destNode = dest->addNode(nodedef.getNamespacedNode().str(), node.getName().str(), outputType);

        bool writeDefaultValues = options ? options->writeDefaultValues : false;

        for (size_t i = 0; i < nodedef.numInputs(); ++i)
        {
            RtInput nodedefInput = nodedef.getInput(i);
            RtInput input = node.getInput(i);
            if (input)
            {
                const RtTypedValue* uiVisible1 = input.getAttribute(Tokens::UIVISIBLE, RtType::BOOLEAN);
                const RtTypedValue* uiVisible2 = nodedefInput.getAttribute(Tokens::UIVISIBLE, RtType::BOOLEAN);
                const bool uiHidden1 = uiVisible1 && !uiVisible1->asBool();
                const bool uiHidden2 = uiVisible2 && !uiVisible2->asBool();
                const bool writeUiVisibleData = uiHidden1 != uiHidden2;

                // Write input if it's connected or different from default value.
                // Write input if the uivisible value differs in the input from the nodedef
                if (writeDefaultValues || writeUiVisibleData ||
                    input.isConnected() || !RtValue::compare(input.getType(), input.getValue(), nodedefInput.getValue()))
                {
                    ValueElementPtr valueElem;
                    if (input.isUniform())
                    {
                        valueElem = destNode->addInput(input.getName().str(), input.getType().str());
                        valueElem->setIsUniform(true);
                        if (input.isConnected())
                        {
                            RtOutput source = input.getConnection();
                            if (source.isSocket())
                            {
                                // This is a connection to the internal socket of a graph
                                valueElem->setInterfaceName(source.getName().str());
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
                        valueElem = destNode->addInput(input.getName().str(), input.getType().str());
                        if (input.isConnected())
                        {
                            RtOutput source = input.getConnection();
                            if (source.isSocket())
                            {
                                // This is a connection to the internal socket of a graph                                
                                valueElem->setInterfaceName(source.getName().str());
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
                                    inputElem->setNodeGraphString(sourcePrim.getName().str());
                                }
                                else
                                {
                                    inputElem->setNodeName(sourcePrim.getName().str());
                                }
                                if (sourcePrim.numOutputs() > 1)
                                {
                                    inputElem->setOutputString(source.getName().str());
                                }
                            }
                        }
                        else
                        {
                            valueElem->setValueString(input.getValueString());
                        }
                    }

                    writeAttributes(PvtObject::ptr(input), valueElem, inputIgnoreList, options);
                }
            }
        }

        if (numOutputs > 1)
        {
            for (size_t i = 0; i < nodedef.numOutputs(); ++i)
            {
                RtOutput nodedefOutput = nodedef.getOutput(i);
                destNode->addOutput(nodedefOutput.getName().str(), nodedefOutput.getType().str());
            }
        }

        writeAttributes(src, destNode, nodeIgnoreList, options);

        return destNode;
    }

    void writeNodeGraph(const PvtPrim* src, DocumentPtr dest, const RtWriteOptions* options)
    {
        NodeGraphPtr destNodeGraph = dest->addNodeGraph(src->getName().str());
        writeAttributes(src, destNodeGraph, nodegraphIgnoreList, options);

        RtNodeGraph nodegraph(src->hnd());

        if (!options || options->writeNodeGraphInputs)
        {
            // Write inputs.
            for (size_t i = 0; i < src->numInputs(); ++i)
            {
                PvtPort* port = src->getInput(i);

                RtInput nodegraphInput = nodegraph.getInput(port->getName());
                ValueElementPtr v = nullptr;
                if (nodegraphInput.isUniform())
                {
                    v = destNodeGraph->addInput(nodegraphInput.getName().str(), nodegraphInput.getType().str());
                    v->setIsUniform(true);
                }
                else
                {
                    InputPtr input = destNodeGraph->addInput(nodegraphInput.getName().str(), nodegraphInput.getType().str());
                    v = input->asA<ValueElement>();

                    if (nodegraphInput.isConnected())
                    {
                        // Write connections to upstream nodes.
                        RtOutput source = nodegraphInput.getConnection();
                        RtPrim sourcePrim = source.getParent();
                        if (sourcePrim.hasApi<RtNodeGraph>())
                        {
                            input->setNodeGraphString(sourcePrim.getName().str());
                        }
                        else
                        {
                            input->setNodeName(sourcePrim.getName().str());
                        }
                        if (sourcePrim.numOutputs() > 1)
                        {
                            input->setOutputString(source.getName().str());
                        }
                    }
                }
                if (v)
                {
                    v->setValueString(nodegraphInput.getValueString());
                    writeAttributes(port, v, inputIgnoreList, options);
                }
            }
        }

        // Write nodes.
        for (RtPrim node : nodegraph.getNodes())
        {
            writeNode(PvtObject::ptr<PvtPrim>(node), destNodeGraph, options);
        }

        // Write outputs.
        for (size_t i = 0; i < src->numOutputs(); ++i)
        {
            PvtPort* port = src->getOutput(i);

            RtInput nodegraphOutput = nodegraph.getOutputSocket(port->getName());
            OutputPtr output = destNodeGraph->addOutput(nodegraphOutput.getName().str(), nodegraphOutput.getType().str());
            if (nodegraphOutput.isConnected())
            {
                RtOutput source = nodegraphOutput.getConnection();
                if (source.isSocket())
                {
                    output->setInterfaceName(source.getName().str());
                }
                else
                {
                    RtNode sourceNode = source.getParent();
                    output->setNodeName(sourceNode.getName().str());
                    if (sourceNode.numOutputs() > 1)
                    {
                        output->setOutputString(source.getName().str());
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
            if (prim->hasApi<RtCollection>())
            {
                const string& name = prim->getName().str();
                if (dest.getCollection(name))
                {
                    continue;
                }

                CollectionPtr collection = dest.addCollection(name);

                RtCollection rtCollection(prim->hnd());
                RtRelationship rtIncludeCollection = rtCollection.getIncludeCollection();
                string includeList = rtIncludeCollection.getObjectNames();
                collection->setIncludeCollectionString(includeList);

                writeAttributes(prim, collection, collectionIgnoreList, options);
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
                const string name(prim->getName().str());

                if (dest.getCollection(name))
                {
                    continue;
                }

                LookPtr look = dest.addLook(name);

                // Add inherit
                const string inheritList = rtLook.getInherit().getObjectNames();
                if (!inheritList.empty())
                {
                    look->setInheritString(inheritList);
                }

                // Add in material assignments
                for (RtObject obj : rtLook.getMaterialAssigns().getConnections())
                {
                    PvtPrim* pprim = PvtObject::ptr<PvtPrim>(obj);
                    RtMaterialAssign rtMatAssign(pprim->hnd());
                    const string& assignName = rtMatAssign.getName().str();
                    if (look->getMaterialAssign(assignName))
                    {
                        continue;
                    }

                    MaterialAssignPtr massign = look->addMaterialAssign(assignName);
                    massign->setGeom(rtMatAssign.getGeom());

                    if (rtMatAssign.getCollection().hasConnections())
                    {
                        massign->setCollectionString(rtMatAssign.getCollection().getConnection().getName().str());
                    }

                    if (rtMatAssign.getMaterial().isConnected())
                    {
                        massign->setMaterial(rtMatAssign.getMaterial().getConnection().getParent().getName().str());
                    }

                    writeAttributes(pprim, massign, mtrlAssignIgnoreList, options);
                }

                writeAttributes(prim, look, lookIgnoreList, options);
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
                const string name(rtLookGroup.getName().str());

                if (dest.getLookGroup(name))
                {
                    continue;
                }

                LookGroupPtr lookGroup = dest.addLookGroup(name);

                const string lookList = rtLookGroup.getLooks().getObjectNames();
                lookGroup->setLooks(lookList);

                writeAttributes(prim, lookGroup, lookGroupIgnoreList, options);
            }
        }
    }

    void writeGenericPrim(const PvtPrim* src, ElementPtr dest, const RtWriteOptions* options)
    {
        RtGeneric generic(src->hnd());

        ElementPtr elem = dest->addChildOfCategory(generic.getKind().str(), generic.getName().str());
        writeAttributes(src, elem, genericIgnoreList, options);

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

    void writePrimData(DocumentPtr& doc, const RtPrim& prim, const RtWriteOptions* options)
    {
        const PvtPrim* p = PvtObject::ptr<PvtPrim>(prim);
        const RtToken typeName = prim.getTypeInfo()->getShortTypeName();
        if (typeName == RtNodeDef::typeName())
        {
            writeNodeDef(p, doc, options);
        }
        else if (typeName == RtNode::typeName())
        {
            writeNode(p, doc, options);
        }
        else if (typeName == RtNodeGraph::typeName())
        {
            writeNodeGraph(p, doc, options);
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
            writeGenericPrim(p, doc->asA<Element>(), options);
        }
    }

    void writeDocument(DocumentPtr& doc, PvtStage* stage, const RtWriteOptions* options)
    {
        writeAttributes(stage->getRootPrim(), doc, RtTokenSet(), options);

        // Write out any dependent includes
        if (options && options->writeIncludes)
        {
            writeSourceUris(stage, doc);
        }

        std::vector<NodePtr> materialElements;
        for (RtPrim child : stage->getRootPrim()->getChildren(options ? options->objectFilter : nullptr))
        {
            writePrimData(doc, child, options);
        }

        // Write the existing look information
        writeCollections(stage, *doc, options);
        writeLooks(stage, *doc, options);
        writeLookGroups(stage, *doc, options);
    }

    void readUnitDefinitions(DocumentPtr doc)
    {
        UnitConverterRegistryPtr unitDefinitions = RtApi::get().getUnitDefinitions();
        for (UnitTypeDefPtr unitTypeDef : doc->getUnitTypeDefs())
        {
            LinearUnitConverterPtr unitConvert = LinearUnitConverter::create(unitTypeDef);
            unitDefinitions->addUnitConverter(unitTypeDef, unitConvert);
        }
    }

    void writeNodeDefAndImplementation(DocumentPtr document, PvtStage* stage, PvtPrim* prim, const RtWriteOptions* options)
    {
        if (!prim || prim->isDisposed())
        {
            throw ExceptionRuntimeError("Trying to write invalid nodedef" +  (prim ? (": '" + prim->getName().str() + "'") :  EMPTY_STRING));
        }

        // Write the definition
        writeNodeDef(prim, document, options);

        // Write the corresponding nodegraph implementation if any.
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
        RtApi& api = RtApi::get();
        if (names.empty())
        {
            // Write all nodedefs.
            const size_t numNodeDefs = RtApi::get().numNodeDefs();
            for (size_t i = 0; i < numNodeDefs; ++i)
            {
                RtPrim prim = api.getNodeDef(i);
                writeNodeDefAndImplementation(document, stage, PvtObject::ptr<PvtPrim>(prim), options);
            }
        }
        else
        {
            // Write only the specified nodedefs.
            for (const RtToken& name : names)
            {
                RtPrim prim = api.getNodeDef(name);
                if (prim)
                {
                    writeNodeDefAndImplementation(document, stage, PvtObject::ptr<PvtPrim>(prim), options);
                }
            }
        }      
    }

} // end anonymous namespace

RtReadOptions::RtReadOptions() :
    elementFilter(nullptr),
    readLookInformation(false)
{
}

RtWriteOptions::RtWriteOptions() :
    writeIncludes(true),
    writeNodeGraphInputs(true),
    writeDefaultValues(false),
    objectFilter(nullptr),
    attributeFilter(nullptr),
    desiredMajorVersion(MATERIALX_MAJOR_VERSION),
    desiredMinorVersion(MATERIALX_MINOR_VERSION)
{
}

void RtFileIo::read(const FilePath& documentPath, const FileSearchPath& searchPaths, const RtReadOptions* options)
{
    try
    {
        DocumentPtr document = createDocument();
        readFromXmlFile(document, documentPath, searchPaths);

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
        readFromXmlStream(document, stream);

        PvtStage* stage = PvtStage::ptr(_stage);
        readDocument(document, stage, options);
    }
    catch (Exception& e)
    {
        throw ExceptionRuntimeError(string("Could not read from stream. Error: ") + e.what());
    }
}

void RtFileIo::readLibraries(const FilePathVec& libraryPaths, const FileSearchPath& searchPaths, const RtReadOptions& /*options*/)
{
    RtApi& api = RtApi::get();
    PvtStage* stage = PvtStage::ptr(_stage);

    // Load all content into a document.
    DocumentPtr doc = createDocument();
    MaterialX::loadLibraries(libraryPaths, searchPaths, doc);

    StringSet uris = doc->getReferencedSourceUris();
    for (const string& uri : uris)
    {
        stage->addSourceUri(RtToken(uri));
    }

    // Load and register all targetdefs. Having these available is needed
    // when implementations are loaded later.
    for (const TargetDefPtr& targetdef : doc->getTargetDefs())
    {
        if (!api.hasTargetDef(RtToken(targetdef->getName())))
        {
            PvtPrim* prim = readTargetDef(targetdef, stage->getRootPrim(), stage);
            api.registerTargetDef(prim->hnd());
        }
    }

    // Update any units found
    readUnitDefinitions(doc);

    // Load and register all nodedefs. Having these available is needed
    // when node instances are loaded later.
    for (const NodeDefPtr& nodedef : doc->getNodeDefs())
    {
        if (!api.hasNodeDef(RtToken(nodedef->getName())))
        {
            PvtPrim* prim = readNodeDef(nodedef, stage);
            api.registerNodeDef(prim->hnd());
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
            else if (elem->isA<Implementation>())
            {
                PvtPrim* prim = readImplementation(elem->asA<Implementation>(), stage->getRootPrim(), stage);
                if (prim)
                {
                    api.registerNodeImpl(prim->hnd());
                }
            }
            else
            {
                readGenericPrim(elem, stage->getRootPrim(), stage, mapper);
            }
        }
        catch(const ExceptionRuntimeError &e)
        {
            api.log(RtLogger::ERROR, e.what());
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

RtPrim RtFileIo::readPrim(std::istream& stream, const RtPath& parentPrimPath, std::string& outOriginalPrimName, const RtReadOptions* options)
{
    try
    {
        PvtPath parentPath(parentPrimPath.asString());
        DocumentPtr document = createDocument();
        XmlReadOptions xmlReadOptions;
        readFromXmlStream(document, stream);

        PvtStage* stage = PvtStage::ptr(_stage);

        RtReadOptions::ElementFilter filter = options ? options->elementFilter : nullptr;

        // Keep track of renamed nodes:
        ElementPtr elem = document->getChildren().size() > 0 ? document->getChildren()[0] : nullptr;
        if (!elem)
        {
            return RtPrim();
        }
        outOriginalPrimName = elem->getName();
        PvtRenamingMapper mapper;
        if (!filter || filter(elem))
        {
            if (elem->isA<NodeDef>())
            {
                PvtPrim* p = readNodeDef(elem->asA<NodeDef>(), stage);
                return p ? p->prim() : RtPrim();
            }
            else if (elem->isA<Node>())
            {
                PvtPrim* p = readNode(elem->asA<Node>(), stage->getPrimAtPath(parentPath), stage, mapper);
                return p ? p->prim() : RtPrim();
            }
            else if (elem->isA<NodeGraph>())
            {
                if (parentPrimPath.asString() != "/")
                {
                    throw ExceptionRuntimeError("Cannot create nested graphs.");
                }
                // Always skip if the nodegraph implements a nodedef
                PvtPath path(PvtPath::ROOT_NAME.str() + elem->getName());
                if (stage->getPrimAtPath(path) && elem->asA<NodeGraph>()->getNodeDef())
                {
                    throw ExceptionRuntimeError("Cannot read node graphs that implement a nodedef.");
                }
                PvtPrim* p = readNodeGraph(elem->asA<NodeGraph>(), stage->getPrimAtPath(parentPath), stage, mapper);
                return p ? p->prim() : RtPrim();
            }
            else if (elem->isA<Backdrop>())
            {
                // TODO: Do something here!
                return RtPrim();
            }
            else
            {
                const RtToken category(elem->getCategory());
                if (category != RtLook::typeName() &&
                    category != RtLookGroup::typeName() &&
                    category != RtMaterialAssign::typeName() &&
                    category != RtCollection::typeName() &&
                    category != RtNodeDef::typeName()) {
                    PvtPrim* p = readGenericPrim(elem, stage->getRootPrim(), stage, mapper);
                    return p ? p->prim() : RtPrim();
                }
            }
        }
    }
    catch (Exception& e)
    {
        throw ExceptionRuntimeError(string("Could not read from stream. Error: ") + e.what());
    }
    return RtPrim();
}

void RtFileIo::writePrim(std::ostream& stream, const RtPath& primPath, const RtWriteOptions* options)
{
    RtPrim prim = _stage->getPrimAtPath(primPath);
    if (!prim)
    {
        throw ExceptionRuntimeError("Can't find prim for path: '" + primPath.asString() + "' in stage: '" + _stage->getName().str() + "'");
    }
    DocumentPtr document = createDocument();
    writePrimData(document, prim, options);
    writeToXmlStream(document, stream);
}

}

