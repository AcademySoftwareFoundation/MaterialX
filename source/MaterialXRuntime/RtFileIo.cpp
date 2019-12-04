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

    void readNodeDef(const NodeDefPtr& src, PvtStage* stage)
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
    }

    PvtNode* readNode(const NodePtr& src, PvtElement* parent, PvtStage* stage)
    {
        NodeDefPtr srcNodedef = src->getNodeDef();
        if (!srcNodedef)
        {
            throw ExceptionRuntimeError("No matching nodedef was found for node '" + src->getName() + "'");
        }

        const RtToken nodedefName(srcNodedef->getName());
        PvtDataHandle nodedefH = stage->findChildByName(nodedefName);
        if (!nodedefH)
        {
            // NodeDef is not loaded yet so create it now.
            readNodeDef(srcNodedef, stage);
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

        return node;
    }

    PvtNodeGraph* readNodeGraph(const NodeGraphPtr& src, PvtElement* parent, PvtStage* stage)
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
                PvtNode* node = readNode(srcNnode, nodegraph, stage);

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
                            PvtDataHandle socket = PvtPortDef::createNew(nodegraph, internalInputName, portType);
                            
                            const string& valueStr = elem->getValueString();
                            if (!valueStr.empty())
                            {
                                RtValue::fromString(portType, valueStr, socket->asA<PvtPortDef>()->getValue());
                            }
                        }
                        const RtToken inputName(elem->getName());
                        RtPort input = node->findPort(inputName);
                        PvtNode::connect(inputSocket, input);
                    }
                }
            }
        }

        // Create all connections.
        std::set<Edge> processedEdges;
        std::set<Element*> processedInterfaces;
        for (auto elem : src->getOutputs())
        {
            for (Edge edge : elem->traverseGraph())
            {
                if (processedEdges.count(edge))
                {
                    continue;
                }

                ElementPtr downstreamElem = edge.getDownstreamElement();
                ElementPtr connectingElem = edge.getConnectingElement();
                ElementPtr upstreamElem = edge.getUpstreamElement();

                if (upstreamElem->isA<Node>() && !processedInterfaces.count(upstreamElem.get()))
                {
                    const RtToken upstreamNodeName(upstreamElem->getName());
                    PvtNode* upstreamNode = nodegraph->findNode(upstreamNodeName);

                    if (downstreamElem->isA<Output>())
                    {
                        RtPort output = upstreamNode->getPort(0); // TODO: Fixme!
                        // Single outputs can have arbitrary names,
                        // so access by index in that case.
                        RtPort outputSocket = nodegraph->numOutputs() == 1 ?
                            nodegraph->getOutputSocket(0) :
                            nodegraph->findOutputSocket(RtToken(downstreamElem->getName()));
                        PvtNode::connect(output, outputSocket);
                    }
                    else
                    {
                        const RtToken downstreamNodeName(downstreamElem->getName());
                        const RtToken downstreamInputName(connectingElem->getName());
                        PvtNode* downstreamNode = nodegraph->findNode(downstreamNodeName);
                        RtPort input = downstreamNode->findPort(downstreamInputName);
                        RtPort output = upstreamNode->getPort(0); // TODO: Fixme!
                        PvtNode::connect(output, input);
                    }

                    processedInterfaces.insert(upstreamElem.get());
                }

                processedEdges.insert(edge);
            }
        }

        return nodegraph;
    }

    PvtUnknownElement* readUnknown(const ElementPtr& src, PvtElement* parent)
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

        return elem;
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
    void writeNode(const PvtNode* node, T dest)
    {
        const PvtNodeDef* nodedef = node->getNodeDef()->asA<PvtNodeDef>();

        const string type = nodedef->numOutputs() == 1 ? nodedef->getPort(0)->getType().str() : "multioutput";
        NodePtr destNode = dest->addNode(nodedef->getNodeName(), node->getName().str(), type);

        for (size_t i = 0; i < nodedef->numInputs(); ++i)
        {
            const PvtPortDef* inputDef = nodedef->getInput(i);
            RtPort input = const_cast<PvtNode*>(node)->findPort(inputDef->getName());
            if (input.isConnected() || input.getValue() != inputDef->getValue())
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
                            if (sourceNode->numOutputs() > 1)
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
        for (size_t i = 0; i < nodedef->numOutputs(); ++i)
        {
            const PvtPortDef* output = nodedef->getOutput(i);
            OutputPtr destOutput = destNode->addOutput(output->getName(), output->getType().str());
        }

        writeAttributes(node, destNode);
    }

    void writeNodeGraph(const PvtNodeGraph* nodegraph, DocumentPtr dest)
    {
        NodeGraphPtr destNodeGraph = dest->addNodeGraph(nodegraph->getName());
        writeAttributes(nodegraph, destNodeGraph);

        for (auto node : nodegraph->getChildren())
        {
            writeNode(node->asA<PvtNode>(), destNodeGraph);
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
                    writeNode(elem->asA<PvtNode>(), doc);
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
