//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtConnectableApi.h>
#include <MaterialXRuntime/Identifiers.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXCore/Element.h>

namespace MaterialX
{

namespace
{
    // TODO: We should derive this from a data driven XML schema.
    class PvtNodeGraphPrimSpec : public PvtPrimSpec
    {
    public:
        PvtNodeGraphPrimSpec()
        {
            addPrimAttribute(Identifiers::DOC, RtType::STRING);
            addPrimAttribute(Identifiers::XPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::YPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::WIDTH, RtType::INTEGER);
            addPrimAttribute(Identifiers::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Identifiers::UICOLOR, RtType::COLOR3);
            addPrimAttribute(Identifiers::UINAME, RtType::STRING);
            addPrimAttribute(Identifiers::VERSION, RtType::IDENTIFIER);
            addPrimAttribute(Identifiers::NAMESPACE, RtType::IDENTIFIER);
            addPrimAttribute(Identifiers::NODEDEF, RtType::IDENTIFIER);
            addPrimAttribute(Identifiers::COLORSPACE, RtType::IDENTIFIER);
            addPrimAttribute(Identifiers::FILEPREFIX, RtType::STRING);

            addInputAttribute(Identifiers::DOC, RtType::STRING);
            addInputAttribute(Identifiers::MEMBER, RtType::STRING);
            addInputAttribute(Identifiers::CHANNELS, RtType::STRING);
            addInputAttribute(Identifiers::UIADVANCED, RtType::BOOLEAN);
            addInputAttribute(Identifiers::UIVISIBLE, RtType::BOOLEAN);

            addInputAttributeByType(RtType::COLOR3, Identifiers::COLORSPACE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::COLOR4, Identifiers::COLORSPACE, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::FLOAT, Identifiers::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UNITTYPE, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::VECTOR2, Identifiers::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UNITTYPE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::DEFAULTGEOMPROP, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::VECTOR3, Identifiers::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UNITTYPE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::DEFAULTGEOMPROP, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::VECTOR4, Identifiers::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UNITTYPE, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::FILENAME, Identifiers::COLORSPACE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::FILENAME, Identifiers::FILEPREFIX, RtType::STRING);

            addOutputAttribute(Identifiers::DOC, RtType::STRING);
            addOutputAttribute(Identifiers::MEMBER, RtType::STRING);
            addOutputAttribute(Identifiers::WIDTH, RtType::INTEGER);
            addOutputAttribute(Identifiers::HEIGHT, RtType::INTEGER);
            addOutputAttribute(Identifiers::BITDEPTH, RtType::INTEGER);

            addOutputAttributeByType(RtType::COLOR3, Identifiers::COLORSPACE, RtType::IDENTIFIER);
            addOutputAttributeByType(RtType::COLOR4, Identifiers::COLORSPACE, RtType::IDENTIFIER);
        }
    };

}


// Private implementation of nodegraph prim.
class PvtNodeGraphPrim : public PvtPrim
{
public:
    PvtNodeGraphPrim(const RtTypeInfo* typeInfo, const RtIdentifier& name, PvtPrim* parent)
        : PvtPrim(typeInfo, name, parent)
    {}

    PvtOutput* createInputSocket(const RtIdentifier& name, const RtIdentifier& type, uint32_t flags)
    {
        PvtOutput* port = new PvtOutput(name, type, flags | RtPortFlag::SOCKET, this);
        _inputSockets.add(port);
        return port;
    }

    PvtInput* createOutputSocket(const RtIdentifier& name, const RtIdentifier& type, uint32_t flags)
    {
        PvtInput* port = new PvtInput(name, type, flags | RtPortFlag::SOCKET, this);
        _outputSockets.add(port);
        return port;
    }

    void removeInputSocket(const RtIdentifier& name)
    {
        PvtObjHandle hnd = _inputSockets.remove(name);
        hnd->asA<PvtPort>()->setDisposed(true);
    }

    void removeOutputSocket(const RtIdentifier& name)
    {
        PvtObjHandle hnd = _outputSockets.remove(name);
        hnd->asA<PvtPort>()->setDisposed(true);
    }

    void renameInputSocket(const RtIdentifier& name, const RtIdentifier& newName)
    {
        PvtObjHandle hnd = _inputSockets.remove(name);
        hnd->asA<PvtPort>()->setName(newName);
        _inputSockets.add(hnd.get());
    }

    void renameOutputSocket(const RtIdentifier& name, const RtIdentifier& newName)
    {
        PvtObjHandle hnd = _outputSockets.remove(name);
        hnd->asA<PvtPort>()->setName(newName);
        _outputSockets.add(hnd.get());
    }

    PvtOutput* getInputSocket(const RtIdentifier& name) const
    {
        PvtObject* obj = _inputSockets.find(name);
        return obj ? obj->asA<PvtOutput>() : nullptr;
    }

    PvtInput* getOutputSocket(const RtIdentifier& name) const
    {
        PvtObject* obj = _outputSockets.find(name);
        return obj ? obj->asA<PvtInput>() : nullptr;
    }

    PvtObjectList _inputSockets;
    PvtObjectList _outputSockets;
};


DEFINE_TYPED_SCHEMA(RtNodeGraph, "node:nodegraph");

RtPrim RtNodeGraph::createPrim(const RtIdentifier& typeName, const RtIdentifier& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name);

    static const RtIdentifier DEFAULT_NAME("nodegraph1");
    const RtIdentifier primName = name == EMPTY_IDENTIFIER ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew<PvtNodeGraphPrim>(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    return primH;
}

const RtPrimSpec& RtNodeGraph::getPrimSpec() const
{
    static const PvtNodeGraphPrimSpec s_primSpec;
    return s_primSpec;
}

RtInput RtNodeGraph::createInput(const RtIdentifier& name, const RtIdentifier& type, uint32_t flags)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    PvtInput* port = graph->createInput(name, type, flags);
    graph->createInputSocket(port->getName(), type, flags);
    return port->hnd();
}

void RtNodeGraph::removeInput(const RtIdentifier& name)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    graph->removeInputSocket(name);
    graph->removeInput(name);
}

RtIdentifier RtNodeGraph::renameInput(const RtIdentifier& name, const RtIdentifier& newName)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    RtIdentifier newPortName = graph->renameInput(name, newName);
    graph->renameInputSocket(name, newPortName);
    return newPortName;
}

RtOutput RtNodeGraph::createOutput(const RtIdentifier& name, const RtIdentifier& type, uint32_t flags)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    PvtOutput* port = graph->createOutput(name, type, flags);
    graph->createOutputSocket(port->getName(), type, flags);
    return port->hnd();
}

void RtNodeGraph::removeOutput(const RtIdentifier& name)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    graph->removeOutputSocket(name);
    graph->removeOutput(name);
}

RtIdentifier RtNodeGraph::renameOutput(const RtIdentifier& name, const RtIdentifier& newName)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    RtIdentifier newPortName = graph->renameOutput(name, newName);
    graph->renameOutputSocket(name, newPortName);
    return newPortName;
}

RtOutput RtNodeGraph::getInputSocket(const RtIdentifier& name) const
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    PvtOutput* socket = graph->getInputSocket(name);
    return socket ? socket->hnd() : RtOutput();
}

RtInput RtNodeGraph::getOutputSocket(const RtIdentifier& name) const
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    PvtInput* socket = graph->getOutputSocket(name);
    return socket ? socket->hnd() : RtInput();
}

RtNodeLayout RtNodeGraph::getNodeLayout()
{
    RtNodeLayout layout;
    for (PvtObject* input : prim()->getInputs())
    {
        layout.order.push_back(input->getName());
        RtTypedValue* attr = input->getAttribute(Identifiers::UIFOLDER, RtType::STRING);
        if (attr)
        {
            layout.uifolder[input->getName()] = attr->asString();
        }
    }
    return layout;
}

void RtNodeGraph::setNodeLayout(const RtNodeLayout& layout)
{
    PvtNodeGraphPrim* graph = prim()->asA<PvtNodeGraphPrim>();

    // Create new input/output lists with ports in the specifed order 
    RtIdentifierSet processed;
    PvtObjectList newInputList;
    PvtObjectList newOutputList;
    PvtObjectList newInputSocketList;
    PvtObjectList newOutputSocketList;
    for (const RtIdentifier& name : layout.order)
    {
        if (!processed.count(name))
        {
            PvtInput* input = graph->getInput(name);
            if (input)
            {
                PvtOutput* socket = graph->getInputSocket(name);
                newInputList.add(input);
                newInputSocketList.add(socket);
            }
            else
            {
                PvtOutput* output = graph->getOutput(name);
                if (output)
                {
                    PvtInput* socket = graph->getOutputSocket(name);
                    newOutputList.add(output);
                    newOutputSocketList.add(socket);
                }
            }
            processed.insert(name);
        }
    }

    // Move over any attributes that were not specified in the new order.
    for (PvtObject* input : graph->getInputs())
    {
        if (!processed.count(input->getName()))
        {
            PvtOutput* socket = graph->getInputSocket(input->getName());
            newInputList.add(input);
            newInputSocketList.add(socket);
            processed.insert(input->getName());
        }
    }
    for (PvtObject* output : graph->getOutputs())
    {
        if (!processed.count(output->getName()))
        {
            PvtInput* socket = graph->getOutputSocket(output->getName());
            newOutputList.add(output);
            newOutputSocketList.add(socket);
            processed.insert(output->getName());
        }
    }

    // Make sure all attributes were moved.
    if (newInputList.size() != graph->numInputs() || newOutputList.size() != graph->numOutputs())
    {
        throw ExceptionRuntimeError("Failed setting new node layout for '" + getName().str() + "'. Changing the port count is not allowed.");
    }

    // Switch to the new order.
    graph->_inputs = newInputList;
    graph->_outputs = newOutputList;
    graph->_inputSockets = newInputSocketList;
    graph->_outputSockets = newOutputSocketList;

    // Assign uifolder metadata.
    for (PvtObject* input: graph->getInputs())
    {
        auto it = layout.uifolder.find(input->getName());
        if (it != layout.uifolder.end() && !it->second.empty())
        {
            RtTypedValue* attr = input->createAttribute(Identifiers::UIFOLDER, RtType::STRING);
            attr->asString() = it->second;
        }
        else
        {
            input->removeAttribute(Identifiers::UIFOLDER);
        }
    }
}

RtPrim RtNodeGraph::getNode(const RtIdentifier& name) const
{
    PvtPrim* p = prim()->getChild(name);
    return p && p->getTypeInfo()->isCompatible(RtNode::typeName()) ? p->hnd() : RtPrim();
}

RtPrimIterator RtNodeGraph::getNodes() const
{
    RtSchemaPredicate<RtNode> predicate;
    return RtPrimIterator(hnd(), predicate);
}

void RtNodeGraph::setDefinition(const RtIdentifier& nodedef)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::NODEDEF, RtType::IDENTIFIER);
    attr->asIdentifier() = nodedef;
}

const RtIdentifier& RtNodeGraph::getDefinition() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::NODEDEF, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : EMPTY_IDENTIFIER;
}

void RtNodeGraph::setNamespace(const RtIdentifier& namespaceString)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::NAMESPACE, RtType::IDENTIFIER);
    attr->asIdentifier() = namespaceString;
}

const RtIdentifier& RtNodeGraph::getNamespace() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::NAMESPACE, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : EMPTY_IDENTIFIER;
}


string RtNodeGraph::asStringDot() const
{
    string dot = "digraph {\n";

    // Add input/output interface boxes.
    dot += "    \"inputs\" ";
    dot += "[shape=box];\n";
    dot += "    \"outputs\" ";
    dot += "[shape=box];\n";

    // Add all nodes.
    for (const RtPrim prim : getNodes())
    {
        dot += "    \"" + prim.getName().str() + "\" ";
        dot += "[shape=ellipse];\n";
    }

    // Add connections inbetween nodes
    // and between nodes and input interface.
    for (const RtPrim prim : getNodes())
    {
        const string dstName = prim.getName().str();
        for (size_t i = 0; i < prim.numInputs(); ++i)
        {
            RtInput input = prim.getInput(i);
            if (input.isConnected())
            {
                const RtOutput src = input.getConnection();
                const string srcName = src.isSocket() ? "inputs" : src.getParent().getName().str();
                dot += "    \"" + srcName;
                dot += "\" -> \"" + dstName;
                dot += "\" [label=\"" + input.getName().str() + "\"];\n";
            }
        }
    }

    // Add connections between nodes and output sockets.
    for (RtOutput output : getOutputs())
    {
        RtInput socket = getOutputSocket(output.getName());
        if (socket.isConnected())
        {
            const RtOutput src = socket.getConnection();
            const string srcName = src.isSocket() ? "inputs" : src.getParent().getName().str();
            dot += "    \"" + srcName;
            dot += "\" -> \"outputs";
            dot += "\" [label=\"" + socket.getName().str() + "\"];\n";
        }
    }

    dot += "}\n";

    return dot;
}

}
