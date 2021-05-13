//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtConnectableApi.h>

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
            addPrimAttribute(RtString::DOC, RtType::STRING);
            addPrimAttribute(RtString::XPOS, RtType::FLOAT);
            addPrimAttribute(RtString::YPOS, RtType::FLOAT);
            addPrimAttribute(RtString::WIDTH, RtType::INTEGER);
            addPrimAttribute(RtString::HEIGHT, RtType::INTEGER);
            addPrimAttribute(RtString::UICOLOR, RtType::COLOR3);
            addPrimAttribute(RtString::UINAME, RtType::STRING);
            addPrimAttribute(RtString::VERSION, RtType::INTERNSTRING);
            addPrimAttribute(RtString::NAMESPACE, RtType::INTERNSTRING);
            addPrimAttribute(RtString::NODEDEF, RtType::INTERNSTRING);
            addPrimAttribute(RtString::COLORSPACE, RtType::INTERNSTRING);
            addPrimAttribute(RtString::FILEPREFIX, RtType::STRING);

            addInputAttribute(RtString::DOC, RtType::STRING);
            addInputAttribute(RtString::MEMBER, RtType::STRING);
            addInputAttribute(RtString::CHANNELS, RtType::STRING);
            addInputAttribute(RtString::UIADVANCED, RtType::BOOLEAN);
            addInputAttribute(RtString::UIVISIBLE, RtType::BOOLEAN);

            addInputAttributeByType(RtType::COLOR3, RtString::COLORSPACE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::COLOR4, RtString::COLORSPACE, RtType::INTERNSTRING);

            addInputAttributeByType(RtType::FLOAT, RtString::UNIT, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::FLOAT, RtString::UNITTYPE, RtType::INTERNSTRING);

            addInputAttributeByType(RtType::VECTOR2, RtString::UNIT, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR2, RtString::UNITTYPE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR2, RtString::DEFAULTGEOMPROP, RtType::INTERNSTRING);

            addInputAttributeByType(RtType::VECTOR3, RtString::UNIT, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR3, RtString::UNITTYPE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR3, RtString::DEFAULTGEOMPROP, RtType::INTERNSTRING);

            addInputAttributeByType(RtType::VECTOR4, RtString::UNIT, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR4, RtString::UNITTYPE, RtType::INTERNSTRING);

            addInputAttributeByType(RtType::FILENAME, RtString::COLORSPACE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::FILENAME, RtString::FILEPREFIX, RtType::STRING);

            addOutputAttribute(RtString::DOC, RtType::STRING);
            addOutputAttribute(RtString::MEMBER, RtType::STRING);
            addOutputAttribute(RtString::WIDTH, RtType::INTEGER);
            addOutputAttribute(RtString::HEIGHT, RtType::INTEGER);
            addOutputAttribute(RtString::BITDEPTH, RtType::INTEGER);

            addOutputAttributeByType(RtType::COLOR3, RtString::COLORSPACE, RtType::INTERNSTRING);
            addOutputAttributeByType(RtType::COLOR4, RtString::COLORSPACE, RtType::INTERNSTRING);
        }
    };

}


// Private implementation of nodegraph prim.
class PvtNodeGraphPrim : public PvtPrim
{
public:
    PvtNodeGraphPrim(const RtTypeInfo* typeInfo, const RtString& name, PvtPrim* parent)
        : PvtPrim(typeInfo, name, parent)
    {}

    PvtOutput* createInputSocket(const RtString& name, const RtString& type, uint32_t flags)
    {
        PvtOutput* port = new PvtOutput(name, type, flags | RtPortFlag::SOCKET, this);
        _inputSockets.add(port);
        return port;
    }

    PvtInput* createOutputSocket(const RtString& name, const RtString& type, uint32_t flags)
    {
        PvtInput* port = new PvtInput(name, type, flags | RtPortFlag::SOCKET, this);
        _outputSockets.add(port);
        return port;
    }

    void removeInputSocket(const RtString& name)
    {
        PvtObjHandle hnd = _inputSockets.remove(name);
        hnd->asA<PvtPort>()->setDisposed(true);
    }

    void removeOutputSocket(const RtString& name)
    {
        PvtObjHandle hnd = _outputSockets.remove(name);
        hnd->asA<PvtPort>()->setDisposed(true);
    }

    void renameInputSocket(const RtString& name, const RtString& newName)
    {
        PvtObjHandle hnd = _inputSockets.remove(name);
        hnd->asA<PvtPort>()->setName(newName);
        _inputSockets.add(hnd.get());
    }

    void renameOutputSocket(const RtString& name, const RtString& newName)
    {
        PvtObjHandle hnd = _outputSockets.remove(name);
        hnd->asA<PvtPort>()->setName(newName);
        _outputSockets.add(hnd.get());
    }

    PvtOutput* getInputSocket(const RtString& name) const
    {
        PvtObject* obj = _inputSockets.find(name);
        return obj ? obj->asA<PvtOutput>() : nullptr;
    }

    PvtInput* getOutputSocket(const RtString& name) const
    {
        PvtObject* obj = _outputSockets.find(name);
        return obj ? obj->asA<PvtInput>() : nullptr;
    }

    PvtObjectList _inputSockets;
    PvtObjectList _outputSockets;
};


DEFINE_TYPED_SCHEMA(RtNodeGraph, "node:nodegraph");

RtPrim RtNodeGraph::createPrim(const RtString& typeName, const RtString& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name);

    static const RtString DEFAULT_NAME("nodegraph1");
    const RtString primName = name.empty() ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew<PvtNodeGraphPrim>(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    return primH;
}

const RtPrimSpec& RtNodeGraph::getPrimSpec() const
{
    static const PvtNodeGraphPrimSpec s_primSpec;
    return s_primSpec;
}

RtInput RtNodeGraph::createInput(const RtString& name, const RtString& type, uint32_t flags)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    PvtInput* port = graph->createInput(name, type, flags);
    graph->createInputSocket(port->getName(), type, flags);
    return port->hnd();
}

void RtNodeGraph::removeInput(const RtString& name)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    graph->removeInputSocket(name);
    graph->removeInput(name);
}

RtString RtNodeGraph::renameInput(const RtString& name, const RtString& newName)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    RtString newPortName = graph->renameInput(name, newName);
    graph->renameInputSocket(name, newPortName);
    return newPortName;
}

RtOutput RtNodeGraph::createOutput(const RtString& name, const RtString& type, uint32_t flags)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    PvtOutput* port = graph->createOutput(name, type, flags);
    graph->createOutputSocket(port->getName(), type, flags);
    return port->hnd();
}

void RtNodeGraph::removeOutput(const RtString& name)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    graph->removeOutputSocket(name);
    graph->removeOutput(name);
}

RtString RtNodeGraph::renameOutput(const RtString& name, const RtString& newName)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    RtString newPortName = graph->renameOutput(name, newName);
    graph->renameOutputSocket(name, newPortName);
    return newPortName;
}

RtOutput RtNodeGraph::getInputSocket(const RtString& name) const
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    PvtOutput* socket = graph->getInputSocket(name);
    return socket ? socket->hnd() : RtOutput();
}

RtInput RtNodeGraph::getOutputSocket(const RtString& name) const
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
        RtTypedValue* attr = input->getAttribute(RtString::UIFOLDER, RtType::STRING);
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
    RtStringSet processed;
    PvtObjectList newInputList;
    PvtObjectList newOutputList;
    PvtObjectList newInputSocketList;
    PvtObjectList newOutputSocketList;
    for (const RtString& name : layout.order)
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
            RtTypedValue* attr = input->createAttribute(RtString::UIFOLDER, RtType::STRING);
            attr->asString() = it->second;
        }
        else
        {
            input->removeAttribute(RtString::UIFOLDER);
        }
    }
}

RtPrim RtNodeGraph::getNode(const RtString& name) const
{
    PvtPrim* p = prim()->getChild(name);
    return p && p->getTypeInfo()->isCompatible(RtNode::typeName()) ? p->hnd() : RtPrim();
}

RtPrimIterator RtNodeGraph::getNodes() const
{
    RtSchemaPredicate<RtNode> predicate;
    return RtPrimIterator(hnd(), predicate);
}

void RtNodeGraph::setDefinition(const RtString& nodedef)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::NODEDEF, RtType::INTERNSTRING);
    attr->asInternString() = nodedef;
}

const RtString& RtNodeGraph::getDefinition() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::NODEDEF, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
}

void RtNodeGraph::setNamespace(const RtString& namespaceString)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::NAMESPACE, RtType::INTERNSTRING);
    attr->asInternString() = namespaceString;
}

const RtString& RtNodeGraph::getNamespace() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::NAMESPACE, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
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
