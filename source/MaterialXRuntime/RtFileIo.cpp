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
#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtTargetDef.h>
#include <MaterialXRuntime/Codegen/RtSourceCodeImpl.h>
#include <MaterialXRuntime/Codegen/RtSubGraphImpl.h>

#include <MaterialXRuntime/Private/PvtStage.h>
#include <MaterialXRuntime/Private/PvtApi.h>

#include <MaterialXCore/Types.h>

#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/XmlExport.h>

#include <sstream>
#include <fstream>
#include <map>

namespace MaterialX
{

namespace
{
    // Lists of attributes which are handled explicitly by read/write and should be ignored when reading/writing attributes.
    static const RtStringSet nodedefIgnoreList    = { RtString("name"), RtString("type") };
    static const RtStringSet portIgnoreList       = { RtString("name"), RtString("type"), RtString("value"), RtString("nodename"), RtString("output"), RtString("channels") };
    static const RtStringSet inputIgnoreList      = { RtString("name"), RtString("type"), RtString("value"), RtString("nodename"), RtString("output"), RtString("channels"),
                                                     RtString("nodegraph"), RtString("interfacename") };
    static const RtStringSet nodeIgnoreList       = { RtString("name"), RtString("type") };
    static const RtStringSet nodegraphIgnoreList  = { RtString("name") };
    static const RtStringSet targetdefIgnoreList  = { RtString("name"), RtString("inherit") };
    static const RtStringSet nodeimplIgnoreList   = { RtString("name"), RtString("file"), RtString("sourcecode") };
    static const RtStringSet lookIgnoreList       = { RtString("name"), RtString("inherit") };
    static const RtStringSet mtrlAssignIgnoreList = { RtString("name"), RtString("geom"), RtString("collection"), RtString("material") };
    static const RtStringSet collectionIgnoreList = { RtString("name"), RtString("includecollection") };
    static const RtStringSet genericIgnoreList    = { RtString("name"), RtString("kind") };
    static const RtStringSet stageIgnoreList      = {};

    static const RtString MULTIOUTPUT("multioutput");
    static const RtString SWIZZLE_INPUT("in");
    static const RtString SWIZZLE_CHANNELS("channels");

    // Specification of attributes for the document root element.
    class PvtRootPrimSpec : public PvtPrimSpec
    {
    public:
        PvtRootPrimSpec()
        {
            // TODO: We should derive this from a data driven XML schema.
            addPrimAttribute(RtString::DOC, RtType::STRING);
            addPrimAttribute(RtString::COLORSPACE, RtType::INTERNSTRING);
        }
    };

    class PvtRenamingMapper
    {
        typedef RtStringMap<RtString> IdToIdMap;
        typedef std::map<PvtPrim*, IdToIdMap> PerPrimMap;

        PerPrimMap _map;
    public:
        void addMapping(PvtPrim* parent, const RtString& originalName, const RtString& finalName) {
            if (originalName != finalName) {
                _map[parent][originalName] = finalName;
            }
        }

        const RtString& getFinalName(PvtPrim* parent, const RtString& originalName) const {
            PerPrimMap::const_iterator primTarget = _map.find(parent);
            if (primTarget != _map.cend()) {
                const IdToIdMap& nameMap = primTarget->second;
                IdToIdMap::const_iterator nameTarget = nameMap.find(originalName);
                if (nameTarget != nameMap.cend()) {
                    return nameTarget->second;
                }
            }
            return originalName;
        }
    };

    PvtPrim* findPrimOrThrow(const RtString& name, PvtPrim* parent, const PvtRenamingMapper& mapper)
    {
        PvtPrim* prim = parent->getChild(mapper.getFinalName(parent, name));
        if (!prim)
        {
            throw ExceptionRuntimeError("Can't find node '" + name.str() + "' in '" + parent->getName().str() + "'");
        }
        return prim;
    }

    PvtInput* findInputOrThrow(const RtString& name, PvtPrim* prim)
    {
        PvtInput* input = prim->getInput(name);
        if (!input)
        {
            throw ExceptionRuntimeError("Node '" + prim->getName().str() + "' has no input named '" + name.str() + "'");
        }
        return input;
    }

    PvtOutput* findOutputOrThrow(const RtString& name, PvtPrim* prim)
    {
        PvtOutput* output = name.str().empty() ? prim->getOutput() : prim->getOutput(name);
        if (!output)
        {
            throw ExceptionRuntimeError("Node '" + prim->getName().str() + "' has no output named '" + name.str() + "'");
        }
        return output;
    }

    void readAttributes(const ElementPtr& src, PvtObject* dest, const RtPrimSpec& primSpec, const RtStringSet& ignoreList)
    {
        // Check if this is a port, we have a separate 
        // attribute spec call for ports below.
        const bool isAPort = dest->isA<PvtPort>();

        for (const string& nameStr : src->getAttributeNames())
        {
            const RtString name(nameStr);
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

    void writeAttributes(const PvtObject* src, ElementPtr dest, const RtStringSet& ignoreList, const RtWriteOptions* options)
    {
        for (const RtString& name : src->getAttributeNames())
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

    void writeTargetDef(const PvtPrim* src, DocumentPtr dest, const RtWriteOptions* options)
    {
        RtTargetDef targetdef(src->hnd());
        TargetDefPtr destTargetDef = dest->addTargetDef(targetdef.getName().str());

        const RtString& inherit = targetdef.getInherit();
        if (!inherit.empty())
        {
            destTargetDef->setInheritString(inherit.str());
        }

        writeAttributes(src, destTargetDef, targetdefIgnoreList, options);
    }

    void writeNodeDef(const PvtPrim* src, DocumentPtr dest, const RtWriteOptions* options)
    {
        RtNodeDef nodedef(src->hnd());
        string nodedefName = nodedef.getName().str();

        // Strip out any extraneous namespace prefix in the name
        string namespaceString = nodedef.getNamespace().str();
        if (!namespaceString.empty())
        {
            namespaceString += NAME_PREFIX_SEPARATOR;
            auto location = nodedefName.find(namespaceString);
            if (location != string::npos)
            {
                nodedefName.erase(location, namespaceString.length());
            }
        }

        NodeDefPtr destNodeDef = dest->addNodeDef(nodedefName, EMPTY_STRING, nodedef.getNode().str());

        const RtString& version = nodedef.getVersion();
        if (!version.empty())
        {
            destNodeDef->setVersionString(version.str());
            if (nodedef.getIsDefaultVersion())
            {
                destNodeDef->setDefaultVersion(true);
            }
        }

        writeAttributes(src, destNodeDef, nodedefIgnoreList, options);

        for (PvtObject* obj : src->getInputs())
        {
            const PvtInput* input = obj->asA<PvtInput>();
            ValueElementPtr destPort = nullptr;
            if (input->isToken())
            {
                destPort = destNodeDef->addToken(input->getName().str());
                destPort->setType(input->getType().str());
            }
            else
            {
                destPort = destNodeDef->addInput(input->getName().str(), input->getType().str());
            }
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

        // Always write out the associated nodedef to avoid ambiguity.
        destNode->setNodeDefString(nodedef.getName().str());

        bool writeDefaultValues = options ? options->writeDefaultValues : false;

        for (size_t i = 0; i < nodedef.numInputs(); ++i)
        {
            RtInput nodedefInput = nodedef.getInput(i);
            RtInput input = node.getInput(i);
            if (input)
            {
                const bool uiHidden1 = input.isUIVisible();
                const bool uiHidden2 = nodedefInput.isUIVisible();
                const bool writeUiVisibleData = uiHidden1 != uiHidden2;

                // Write input if it's connected or different from default value.
                // Write input if the uivisible value differs in the input from the nodedef
                if (writeDefaultValues || writeUiVisibleData ||
                    input.isConnected() || !RtValue::compare(input.getType(), input.getValue(), nodedefInput.getValue()))
                {
                    ValueElementPtr valueElem;
                    if (input.isUniform())
                    {
                        if (input.isToken())
                        {
                            valueElem = destNode->addToken(input.getName().str());
                            valueElem->setType(input.getType().str());
                        }
                        else
                        {
                            valueElem = destNode->addInput(input.getName().str(), input.getType().str());
                        }
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

                    writeAttributes(PvtObject::cast(input), valueElem, inputIgnoreList, options);
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
                    if (nodegraphInput.isToken())
                    {
                        v = destNodeGraph->addToken(nodegraphInput.getName().str());
                        v->setType(nodegraphInput.getType().str());
                    }
                    else
                    {
                        v = destNodeGraph->addInput(nodegraphInput.getName().str(), nodegraphInput.getType().str());
                    }
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
            writeNode(PvtObject::cast<PvtPrim>(node), destNodeGraph, options);
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

    void writeImplementation(const PvtPrim* src, DocumentPtr dest, const RtWriteOptions* options)
    {
        RtNodeImpl impl(src->hnd());
        ImplementationPtr destImpl = dest->addImplementation(impl.getName().str());

        if (src->hasApi<RtSourceCodeImpl>())
        {
            RtSourceCodeImpl sourceImpl(src->hnd());
            const string& file = sourceImpl.getFile();
            const string& source = sourceImpl.getSourceCode();
            if (!file.empty())
            {
                destImpl->setFile(file);
            }
            if (!source.empty())
            {
                destImpl->setAttribute(RtString::SOURCECODE.str(), source);
            }
        }

        writeAttributes(src, destImpl, nodeimplIgnoreList, options);
    }

    void writeCollections(PvtStage* stage, Document& dest, const RtWriteOptions* options)
    {
        for (RtPrim child : stage->getRootPrim()->getChildren(options ? options->objectFilter : nullptr))
        {
            const PvtPrim* prim = PvtObject::cast<PvtPrim>(child);
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
            const PvtPrim* prim = PvtObject::cast<PvtPrim>(child);
            const RtString typeName = child.getTypeInfo()->getShortTypeName();
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
                    PvtPrim* pprim = PvtObject::cast<PvtPrim>(obj);
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

    void writeGenericPrim(const PvtPrim* src, ElementPtr dest, const RtWriteOptions* options)
    {
        RtGeneric generic(src->hnd());

        ElementPtr elem = dest->addChildOfCategory(generic.getKind().str(), generic.getName().str());
        writeAttributes(src, elem, genericIgnoreList, options);

        for (auto child : src->getChildren())
        {
            writeGenericPrim(PvtObject::cast<PvtPrim>(child), elem, options);
        }
    }

    void writeSourceUris(const PvtStage* stage, DocumentPtr doc)
    {
        for (const RtStagePtr& refPtr : stage->getAllReferences())
        {
            const PvtStage* ref = PvtStage::cast(refPtr);
            if (ref->getAllReferences().size() > 0)
            {
                writeSourceUris(ref, doc);
            }
            const FilePathVec& uris = ref->getSourceUri();
            if (!uris.empty())
            {
                for (const FilePath& uri : uris)
                {
                    prependXInclude(doc, uri);
                }
            }
        }
    }

    void writePrimData(DocumentPtr& doc, const RtPrim& prim, const RtWriteOptions* options)
    {
        const PvtPrim* p = PvtObject::cast<PvtPrim>(prim);
        if (p->hasApi<RtNodeDef>())
        {
            writeNodeDef(p, doc, options);
        }
        else if (p->hasApi<RtNode>())
        {
            if (p->hasApi<RtNodeGraph>())
            {
                writeNodeGraph(p, doc, options);
            }
            else
            {
                writeNode(p, doc, options);
            }
        }
        else if (p->hasApi<RtNodeImpl>())
        {
            writeImplementation(p, doc, options);
        }
        else if (p->hasApi<RtTargetDef>())
        {
            writeTargetDef(p, doc, options);
        }
        else if (p->hasApi<RtBackdrop>())
        {
            //writeBackdrop(prim, doc)
        }
        else if (!p->hasApi<RtBindElement>())
        {
            writeGenericPrim(p, doc->asA<Element>(), options);
        }
    }

    void writeDocument(DocumentPtr& doc, PvtStage* stage, const RtWriteOptions* options)
    {
        writeAttributes(stage->getRootPrim(), doc, RtStringSet(), options);

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
        RtString nodeDefName = prim->getName();
        RtString defNamespace = nodedef.getNamespace();
        RtString qualifiedName = nodeDefName;
        //The Node Definition name can already have a namespace prefix attached at the front, so don't bother doing it again.
        if(!defNamespace.empty() && qualifiedName.str().rfind(defNamespace.str() + NAME_PREFIX_SEPARATOR , 0) != 0) {
            qualifiedName = RtString(defNamespace.str() + NAME_PREFIX_SEPARATOR  + nodeDefName.str());
        }
        RtSchemaPredicate<RtNodeGraph> filter;
        for (RtPrim child : stage->getRootPrim()->getChildren(filter))
        {
            // The association between a nodedef and a nodegraph is by name. No
            // version check is required as nodegraphs are not versioned.
            RtNodeGraph nodeGraph(child);
            if (nodeGraph.getDefinition() == qualifiedName)
            {
                PvtPrim* graphPrim = PvtObject::cast<PvtPrim>(child);
                writeNodeGraph(graphPrim, document, options);
                break;
            }
        }
    }

    void writeNodeDefs(DocumentPtr document, PvtStage* stage, const RtStringVec& names, const RtWriteOptions* options)
    {
        PvtApi* api = PvtApi::cast(RtApi::get());
        if (names.empty())
        {
            // Write all nodedefs.
            const size_t numNodeDefs = api->numNodeDefs();
            for (size_t i = 0; i < numNodeDefs; ++i)
            {
                PvtPrim* prim = api->getNodeDef(i)->asA<PvtPrim>();
                writeNodeDefAndImplementation(document, stage, prim, options);
            }
        }
        else
        {
            // Write only the specified nodedefs.
            for (const RtString& name : names)
            {
                PvtPrim* prim = api->getNodeDef(name)->asA<PvtPrim>();
                if (prim)
                {
                    writeNodeDefAndImplementation(document, stage, prim, options);
                }
            }
        }      
    }

    template<class T>
    void createInterface(const ElementPtr& src, T schema)
    {
        StringSet elementsAdded;
        for (ConstElementPtr interface : src->traverseInheritance())
        {
            for (auto elem : interface->getChildrenOfType<ValueElement>())
            {
                if (elementsAdded.insert(elem->getName()).second)
                {
                    const RtString portName(elem->getName());
                    const RtString portType(elem->getType());

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
                    else if (elem->isA<Token>())
                    {
                        port = schema.createInput(portName, portType, RtPortFlag::TOKEN);
                    }

                    if (port)
                    {
                        const string& valueStr = elem->getValueString();
                        if (!valueStr.empty())
                        {
                            RtValue::fromString(portType, valueStr, port.getValue());
                        }

                        readAttributes(elem, PvtObject::cast(port), schema.getPrimSpec(), portIgnoreList);
                    }
                }
            }
        }
    }

    void createConnection(PvtOutput* output, PvtInput* input, const string& swizzle, PvtStage* stage)
    {
        // Check if a swizzle node should be used in the connection.
        if (!swizzle.empty())
        {
            const RtString swizzleNodeDefName("ND_swizzle_" + output->getType().str() + "_" + input->getType().str());
            const RtString swizzleNodeName("swizzle_" + input->getParent()->getName().str() + "_" + input->getName().str());

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
        PvtPrim* node = findPrimOrThrow(RtString(nodeElem->getName()), parent, mapper);
        for (InputPtr elemInput : nodeElem->getInputs())
        {
            PvtInput* input = findInputOrThrow(RtString(elemInput->getName()), node);
            string connectedNodeName = elemInput->getNodeName();
            if (connectedNodeName.empty())
            {
                connectedNodeName = elemInput->getNodeGraphString();
            }
            if (!connectedNodeName.empty())
            {
                PvtPrim* connectedNode = findPrimOrThrow(RtString(connectedNodeName), parent, mapper);
                RtString outputName(elemInput->getOutputString());
                if (outputName.empty() && connectedNode)
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
            // Only process compound nodegraph, since functional nodegraph are not part of the stage.
            NodeGraphPtr nodegraph = nodeElem->asA<NodeGraph>();
            if (!nodegraph || nodegraph->getNodeDefString().empty())
            {
                createNodeConnection(nodeElem->asA<InterfaceElement>(), parent, stage, mapper);
            }
        }
    }

    PvtPrim* readNodeDef(const NodeDefPtr& src, PvtStage* stage)
    {
        const RtString name(src->getName());
        PvtPrim* prim = stage->createPrim(stage->getPath(), name, RtNodeDef::typeName());

        RtNodeDef nodedef(prim->hnd());
        readAttributes(src, prim, nodedef.getPrimSpec(), nodedefIgnoreList);

        // Create the interface.
        createInterface(src, nodedef);

        return prim;
    }

    bool matchingSignature(const PvtPrim* prim, const RtString& nodeType, const vector<ValueElementPtr>& nodePorts)
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
            const RtString name(nodePort->getName());
            const PvtPort* port = nodePort->isA<Input>() ? prim->getInput(name)->asA<PvtPort>() : prim->getOutput(name)->asA<PvtPort>();
            if (!port || port->getType().str() != nodePort->getType())
            {
                return false;
            }
        }

        return true;
    }

    PvtPrim* resolveNodeDef(const NodePtr& node, PvtStage* stage)
    {
        PvtApi* api = PvtApi::cast(RtApi::get());

        // First, see if a nodedef string is specified on the node.
        const string& nodedefNameString = node->getNodeDefString();
        if (!nodedefNameString.empty())
        {
            // Find this nodedef among the available ones.
            const RtString nodedefName(nodedefNameString);

            // Search the globally registered nodedefs.
            PvtObject* obj = api->getNodeDef(nodedefName);
            if (!obj)
            {
                // Search the current stage.
                PvtPath path(nodedefName);
                obj = stage->getPrimAtPath(path);
            }
            return obj ? obj->asA<PvtPrim>() : nullptr;
        }

        // Second, try resolving among the available nodedefs.
        const RtString nodeName(node->getCategory());
        const RtString nodeType(node->getType());
        const vector<ValueElementPtr> nodePorts = node->getActiveValueElements();

        // Search the globally registered nodedefs.
        const size_t numNodeDefs = api->numNodeDefs();
        for (size_t i = 0; i < numNodeDefs; ++i)
        {
            PvtPrim* prim = api->getNodeDef(i)->asA<PvtPrim>();
            RtNodeDef candidate(prim->hnd());
            if (candidate.getNamespacedNode() == nodeName && 
                matchingSignature(prim, nodeType, nodePorts))
            {
                return prim;
            }
        }

        // Search the current stage.
        RtSchemaPredicate<RtNodeDef> filter;
        for (auto it : stage->getPrims(filter))
        {
            PvtPrim* prim = PvtObject::cast<PvtPrim>(it);
            RtNodeDef candidate(prim->hnd());
            if (candidate.getNamespacedNode() == nodeName &&
                matchingSignature(prim, nodeType, nodePorts))
            {
                return prim;
            }
        }

        return nullptr;
    }

    PvtPrim* resolveTargetDef(const RtString target, PvtStage* stage)
    {
        PvtApi* api = PvtApi::cast(RtApi::get());

        PvtObject* obj = api->getTargetDef(target);
        if (obj)
        {
            return obj->asA<PvtPrim>();
        }

        PvtPath path(target);
        return stage->getPrimAtPath(path);
    }

    PvtPrim* readNode(const NodePtr& src, PvtPrim* parent, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        const PvtPrim* nodedef = resolveNodeDef(src, stage);
        if (!nodedef)
        {
            throw ExceptionRuntimeError("No matching nodedef was found for node '" + src->getName() + "'");
        }

        const RtString nodeName(src->getName());
        PvtPrim* prim = stage->createPrim(parent->getPath(), nodeName, nodedef->getName());
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
            const RtString portName(elem->getName());
            PvtInput* input = prim->getInput(portName);
            if (!input)
            {
                throw ExceptionRuntimeError("No input named '" + elem->getName() + "' was found on runtime node '" + prim->getName().str() + "'");
            }
            const string& valueStr = elem->getValueString();
            if (!valueStr.empty())
            {
                const RtString portType(elem->getType());
                RtValue::fromString(portType, valueStr, input->getValue());
            }
            readAttributes(elem, input, node.getPrimSpec(), portIgnoreList);
        }

        return prim;
    }

    PvtPrim* readNodeGraph(const NodeGraphPtr& src, PvtPrim* parent, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        const RtString nodegraphName(src->getName());

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
                        const RtString inputName(elem->getName());
                        PvtInput* input = findInputOrThrow(inputName, node);

                        const RtString socketName(interfaceName);
                        RtOutput socket = nodegraph.getInputSocket(socketName);
                        if (!socket)
                        {
                            const RtString inputType(elem->getType());
                            RtInput graphInput = nodegraph.createInput(socketName, inputType, input->getFlags());
                            socket = nodegraph.getInputSocket(graphInput.getName());

                            // Set the input value
                            const string& valueStr = elem->getValueString();
                            if (!valueStr.empty())
                            {
                                RtValue::fromString(inputType, valueStr, graphInput.getValue());
                            }
                        }

                        PvtOutput* output = PvtObject::cast<PvtOutput>(socket);
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
                RtInput socket = nodegraph.getOutputSocket(RtString(elem->getName()));
                if (!socket)
                {
                    PvtPath path(parent->getPath());
                    path.push(nodegraphName);
                    throw ExceptionRuntimeError("Output '" + elem->getName() + "' does not match an internal output socket on the nodegraph '" + path.asString() + "'");
                }

                PvtPrim* connectedNode = findPrimOrThrow(RtString(connectedNodeName), prim, mapper);

                const RtString outputName(elem->getOutputString());
                PvtOutput* output = findOutputOrThrow(outputName, connectedNode);
                PvtInput* input = PvtObject::cast<PvtInput>(socket);
                const string& swizzle = elem->getChannels();

                createConnection(output, input, swizzle, stage);
            }
        }

        return prim;
    }

    PvtPrim* readGenericPrim(const ElementPtr& src, PvtPrim* parent, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        const RtString name(src->getName());
        const RtString category(src->getCategory());

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

    PvtPrim* readTargetDef(const TargetDefPtr& src, PvtStage* stage)
    {
        PvtPrim* parent = stage->getRootPrim();
        const RtString name(src->getName());
        const RtString inherit(src->getInheritString());

        PvtPrim* prim = stage->createPrim(parent->getPath(), name, RtTargetDef::typeName());

        RtTargetDef def(prim->hnd());
        if (inherit)
        {
            def.setInherit(inherit);
        }

        readAttributes(src, prim, def.getPrimSpec(), targetdefIgnoreList);

        return prim;
    }

    PvtPrim* readImplementation(const ImplementationPtr& src, PvtStage* stage)
    {
        PvtPrim* parent = stage->getRootPrim();
        const RtString target(src->getAttribute(RtString::TARGET.str()));

        // We are only interested in implementations for loaded targets,
        // so if target is set make sure this target has been loaded.
        if (target && !resolveTargetDef(target, stage))
        {
            return nullptr;
        }

        const RtString name(src->getName());

        const string& sourcecode = src->getAttribute(RtString::SOURCECODE.str());
        const string& file = src->getAttribute(RtString::FILE.str());

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
    PvtPrim* readCollection(const CollectionPtr& src, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        PvtPrim* parent = stage->getRootPrim();
        const RtString name(src->getName());

        PvtPrim* prim = stage->createPrim(parent->getPath(), name, RtCollection::typeName());
        mapper.addMapping(stage->getRootPrim(), name, prim->getName());

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
            PvtPrim* parentCollection = findPrimOrThrow(RtString(colElement->getName()), parent, mapper);
            for (const CollectionPtr& includeCollection : colElement->getIncludeCollections())
            {
                PvtPrim* childCollection = findPrimOrThrow(RtString(includeCollection->getName()), parent, mapper);
                RtCollection rtCollection(parentCollection->hnd());
                rtCollection.addCollection(childCollection->hnd());
            }
        }
    }

    // Note that this function reads in a single look. After all required looks have been
    // read in then makeLookInheritConnections() can be called to create look inheritance
    // connections.
    PvtPrim* readLook(const LookPtr& src, PvtStage* stage, PvtRenamingMapper& mapper)
    {
        PvtPrim* parent = stage->getRootPrim();
        const RtString name(src->getName());

        PvtPrim* lookPrim = stage->createPrim(parent->getPath(), name, RtLook::typeName());
        mapper.addMapping(parent, name, lookPrim->getName());
        RtLook look(lookPrim->hnd());

        // Create material assignments
        for (const MaterialAssignPtr& matAssign : src->getMaterialAssigns())
        {
            const RtString matAssignName(matAssign->getName());
            PvtPrim* assignPrim = stage->createPrim(parent->getPath(), matAssignName, RtMaterialAssign::typeName());
            mapper.addMapping(parent, matAssignName, assignPrim->getName());
            RtMaterialAssign rtMatAssign(assignPrim->hnd());
            
            if (!matAssign->getCollectionString().empty())
            {
                PvtPrim* collection = findPrimOrThrow(RtString(matAssign->getCollectionString()), parent, mapper);
                rtMatAssign.getCollection().connect(collection->hnd());
            }

            if (!matAssign->getMaterial().empty())
            {
                PvtPrim* material = findPrimOrThrow(RtString(matAssign->getMaterial()), parent, mapper);
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
            PvtPrim* childLook = findPrimOrThrow(RtString(lookElem->getName()), parent, mapper);
            const string& inheritString = lookElem->getInheritString();
            if (!inheritString.empty())
            {
                PvtPrim* parentLook = findPrimOrThrow(RtString(inheritString), parent, mapper);
                RtLook rtLook(childLook->hnd());
                rtLook.getInherit().connect(parentLook->hnd());
            }
        }
    }

    // Read in all look information from a document. Collections, looks and
    // look groups are read in first. Then relationship linkages are made.
    void readLookInformation(const DocumentPtr& doc, PvtStage* stage, const RtReadOptions* options, PvtRenamingMapper& mapper)
    {
        // Create the mergedDoc
        DocumentPtr mergedDoc = doc;

        PvtPrim* parent = stage->getRootPrim();
        RtReadOptions::ElementFilter filter = options ? options->elementFilter : nullptr;

        // Read collections
        for (const CollectionPtr& elem : mergedDoc->getCollections())
        {
            if (!filter || filter(elem))
            {
                readCollection(elem->asA<Collection>(), stage, mapper);
            }
        }

        // Read looks
        for (const LookPtr& elem : mergedDoc->getLooks())
        {
            if (!filter || filter(elem))
            {
                readLook(elem, stage, mapper);
            }
        }

        // Create additional connections
        makeCollectionIncludeConnections(mergedDoc->getCollections(), parent, mapper);
        makeLookInheritConnections(mergedDoc->getLooks(), parent, mapper);
    }

    void readDocument(const DocumentPtr& doc, PvtStage* stage, const RtReadOptions* options)
    {
        // Set the source location.
        const string& uri = doc->getSourceUri();
        if (!uri.empty())
        {
            stage->addSourceUri(uri);
        }

        // Read root document attributes.
        static PvtRootPrimSpec s_rootPrimSpec;
        readAttributes(doc, stage->getRootPrim(), s_rootPrimSpec, stageIgnoreList);

        RtReadOptions::ElementFilter filter = options ? options->elementFilter : nullptr;

        // First, read all definition elements.
        for (auto elem : doc->getChildren())
        {
            if (!filter || filter(elem))
            {
                if (elem->isA<NodeDef>())
                {
                    readNodeDef(elem->asA<NodeDef>(), stage);
                }
                else if (elem->isA<TargetDef>())
                {
                    readTargetDef(elem->asA<TargetDef>(), stage);
                }
                else if (elem->isA<UnitTypeDef>())
                {
                    UnitTypeDefPtr unitTypeDef = elem->asA<UnitTypeDef>();
                    RtApi::get().getUnitDefinitions()->addUnitConverter(unitTypeDef, LinearUnitConverter::create(unitTypeDef));
                }
            }
        }

        // Second, read all other elements.
        PvtRenamingMapper mapper;
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
                    readNodeGraph(elem->asA<NodeGraph>(), stage->getRootPrim(), stage, mapper);
                }
                else if (elem->isA<Implementation>())
                {
                    readImplementation(elem->asA<Implementation>(), stage);
                }
                else if (!(elem->isA<Look>() ||
                           elem->isA<MaterialAssign>() ||
                           elem->isA<Collection>() ||
                           elem->isA<NodeDef>() ||
                           elem->isA<TargetDef>()))
                {
                    // Read all unknown elements as generic prims into the stage.
                    readGenericPrim(elem, stage->getRootPrim(), stage, mapper);
                }
            }
        }

        // Create connections between all root level nodes in the stage.
        createNodeConnections(doc->getNodes(), stage->getRootPrim(), stage, mapper);

        // Create connections between all nodegraphs in the stage.
        createNodeGraphConnections(doc->getNodeGraphs(), stage->getRootPrim(), stage, mapper);

        // Read look information into the stage.
        if (!options || options->readLookInformation)
        {
            readLookInformation(doc, stage, options, mapper);
        }
    }

    void setupNodeDefs(RtStagePtr stage)
    {
        PvtApi* api = PvtApi::cast(RtApi::get());
        api->registerPrims(stage);
        api->setupNodeImplRelationships();
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

RtExportOptions::RtExportOptions() :
    flattenFilenames(true)
{
}

void RtFileIo::read(const FilePath& documentPath, const FileSearchPath& searchPaths, const RtReadOptions* options)
{
    try
    {
        DocumentPtr document = createDocument();
        readFromXmlFile(document, documentPath, searchPaths);

        PvtStage* stage = PvtStage::cast(_stage.get());
        readDocument(document, stage, options);

        setupNodeDefs(_stage);
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

        PvtStage* stage = PvtStage::cast(_stage.get());
        readDocument(document, stage, options);

        setupNodeDefs(_stage);
    }
    catch (Exception& e)
    {
        throw ExceptionRuntimeError(string("Could not read from stream. Error: ") + e.what());
    }
}

StringSet RtFileIo::readLibrary(const FilePathVec& libraryPaths, const FileSearchPath& searchPaths, const RtReadOptions* options)
{
    PvtStage* stage = PvtStage::cast(_stage.get());

    // Load all content into a core document.
    DocumentPtr doc = createDocument();
    StringSet loadedFiles = MaterialX::loadLibraries(libraryPaths, searchPaths, doc);

    // Read this document.
    readDocument(doc, stage, options);

    return loadedFiles;
}

void RtFileIo::write(const FilePath& documentPath, const RtWriteOptions* options)
{
    PvtStage* stage = PvtStage::cast(_stage.get());

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
    PvtStage* stage = PvtStage::cast(_stage.get());

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

void RtFileIo::writeDefinitions(std::ostream& stream, const RtStringVec& names, const RtWriteOptions* options)
{
    DocumentPtr document = createDocument();
    PvtStage* stage = PvtStage::cast(_stage.get());
    writeNodeDefs(document, stage, names, options);
    writeToXmlStream(document, stream);
}

void RtFileIo::writeDefinitions(const FilePath& documentPath, const RtStringVec& names, const RtWriteOptions* options)
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

        PvtStage* stage = PvtStage::cast(_stage.get());

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
                PvtPath path(RtString(elem->getName()));
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
                const RtString category(elem->getCategory());
                if (category != RtLook::typeName() &&
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

void RtFileIo::exportDocument(std::ostream& stream, const RtExportOptions* options)
{
    PvtStage* stage = PvtStage::cast(_stage.get());

    DocumentPtr document = createDocument();
    writeDocument(document, stage, options);

    XmlExportOptions xmlExportOptions;
    if (options)
    {
        xmlExportOptions.writeXIncludeEnable = options->writeIncludes;
        xmlExportOptions.flattenFilenames = options->flattenFilenames;
        xmlExportOptions.resolvedTexturePath = options->resolvedTexturePath;
        xmlExportOptions.stringResolver = options->stringResolver;
        xmlExportOptions.exportResolvers = options->exportResolvers;
        xmlExportOptions.libraries = options->libraries;
        xmlExportOptions.skipFlattening = options->skipFlattening;
    }

    xmlExportOptions.modifyInPlace = true;
    exportToXmlStream(document, stream, &xmlExportOptions);
}

void RtFileIo::exportDocument(const FilePath& documentPath, const RtExportOptions* options)
{
    PvtStage* stage = PvtStage::cast(_stage.get());

    DocumentPtr document = createDocument();
    writeDocument(document, stage, options);

    XmlExportOptions xmlExportOptions;
    if (options)
    {
        xmlExportOptions.writeXIncludeEnable = options->writeIncludes;
        xmlExportOptions.flattenFilenames = options->flattenFilenames;
        xmlExportOptions.resolvedTexturePath = options->resolvedTexturePath;
        xmlExportOptions.stringResolver = options->stringResolver;
        xmlExportOptions.exportResolvers = options->exportResolvers;
        xmlExportOptions.libraries = options->libraries;
        xmlExportOptions.skipFlattening = options->skipFlattening;
    }
    xmlExportOptions.modifyInPlace = true;
    exportToXmlFile(document, documentPath, &xmlExportOptions);
}

}
