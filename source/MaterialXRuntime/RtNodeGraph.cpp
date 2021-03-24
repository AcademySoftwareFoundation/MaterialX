//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtConnectableApi.h>
#include <MaterialXRuntime/Tokens.h>

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
            addPrimAttribute(Tokens::DOC, RtType::STRING);
            addPrimAttribute(Tokens::XPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::YPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::WIDTH, RtType::INTEGER);
            addPrimAttribute(Tokens::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Tokens::UICOLOR, RtType::COLOR3);
            addPrimAttribute(Tokens::UINAME, RtType::STRING);
            addPrimAttribute(Tokens::VERSION, RtType::TOKEN);
            addPrimAttribute(Tokens::NAMESPACE, RtType::TOKEN);
            addPrimAttribute(Tokens::NODEDEF, RtType::TOKEN);
            addPrimAttribute(Tokens::COLORSPACE, RtType::TOKEN);
            addPrimAttribute(Tokens::FILEPREFIX, RtType::STRING);

            addInputAttribute(Tokens::DOC, RtType::STRING);
            addInputAttribute(Tokens::MEMBER, RtType::STRING);
            addInputAttribute(Tokens::CHANNELS, RtType::STRING);
            addInputAttribute(Tokens::UIADVANCED, RtType::BOOLEAN);
            addInputAttribute(Tokens::UIVISIBLE, RtType::BOOLEAN);

            addInputAttributeByType(RtType::COLOR3, Tokens::COLORSPACE, RtType::TOKEN);
            addInputAttributeByType(RtType::COLOR4, Tokens::COLORSPACE, RtType::TOKEN);

            addInputAttributeByType(RtType::FLOAT, Tokens::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::FLOAT, Tokens::UNITTYPE, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR2, Tokens::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR2, Tokens::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR2, Tokens::DEFAULTGEOMPROP, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR3, Tokens::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR3, Tokens::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR3, Tokens::DEFAULTGEOMPROP, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR4, Tokens::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR4, Tokens::UNITTYPE, RtType::TOKEN);

            addInputAttributeByType(RtType::FILENAME, Tokens::COLORSPACE, RtType::TOKEN);
            addInputAttributeByType(RtType::FILENAME, Tokens::FILEPREFIX, RtType::STRING);

            addOutputAttribute(Tokens::DOC, RtType::STRING);
            addOutputAttribute(Tokens::MEMBER, RtType::STRING);
            addOutputAttribute(Tokens::WIDTH, RtType::INTEGER);
            addOutputAttribute(Tokens::HEIGHT, RtType::INTEGER);
            addOutputAttribute(Tokens::BITDEPTH, RtType::INTEGER);

            addOutputAttributeByType(RtType::COLOR3, Tokens::COLORSPACE, RtType::TOKEN);
            addOutputAttributeByType(RtType::COLOR4, Tokens::COLORSPACE, RtType::TOKEN);
        }
    };

}


// Private implementation of nodegraph prim.
class PvtNodeGraphPrim : public PvtPrim
{
public:
    PvtNodeGraphPrim(const RtTypeInfo* typeInfo, const RtToken& name, PvtPrim* parent)
        : PvtPrim(typeInfo, name, parent)
    {}

    PvtOutput* createInputSocket(const RtToken& name, const RtToken& type, uint32_t flags)
    {
        PvtOutput* port = new PvtOutput(name, type, flags | RtPortFlag::SOCKET, this);
        _inputSockets.add(port);
        return port;
    }

    PvtInput* createOutputSocket(const RtToken& name, const RtToken& type, uint32_t flags)
    {
        PvtInput* port = new PvtInput(name, type, flags | RtPortFlag::SOCKET, this);
        _outputSockets.add(port);
        return port;
    }

    void removeInputSocket(const RtToken& name)
    {
        PvtObjHandle hnd = _inputSockets.remove(name);
        hnd->asA<PvtPort>()->setDisposed(true);
    }

    void removeOutputSocket(const RtToken& name)
    {
        PvtObjHandle hnd = _outputSockets.remove(name);
        hnd->asA<PvtPort>()->setDisposed(true);
    }

    void renameInputSocket(const RtToken& name, const RtToken& newName)
    {
        PvtObjHandle hnd = _inputSockets.remove(name);
        hnd->asA<PvtPort>()->setName(newName);
        _inputSockets.add(hnd.get());
    }

    void renameOutputSocket(const RtToken& name, const RtToken& newName)
    {
        PvtObjHandle hnd = _outputSockets.remove(name);
        hnd->asA<PvtPort>()->setName(newName);
        _outputSockets.add(hnd.get());
    }

    PvtOutput* getInputSocket(const RtToken& name) const
    {
        PvtObject* obj = _inputSockets.find(name);
        return obj ? obj->asA<PvtOutput>() : nullptr;
    }

    PvtInput* getOutputSocket(const RtToken& name) const
    {
        PvtObject* obj = _outputSockets.find(name);
        return obj ? obj->asA<PvtInput>() : nullptr;
    }

    PvtObjectList _inputSockets;
    PvtObjectList _outputSockets;
};


DEFINE_TYPED_SCHEMA(RtNodeGraph, "node:nodegraph");

RtPrim RtNodeGraph::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name);

    static const RtToken DEFAULT_NAME("nodegraph1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew<PvtNodeGraphPrim>(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    return primH;
}

const RtPrimSpec& RtNodeGraph::getPrimSpec() const
{
    static const PvtNodeGraphPrimSpec s_primSpec;
    return s_primSpec;
}

RtInput RtNodeGraph::createInput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    PvtInput* port = graph->createInput(name, type, flags);
    graph->createInputSocket(port->getName(), type, flags);
    return port->hnd();
}

void RtNodeGraph::removeInput(const RtToken& name)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    graph->removeInputSocket(name);
    graph->removeInput(name);
}

RtToken RtNodeGraph::renameInput(const RtToken& name, const RtToken& newName)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    RtToken newPortName = graph->renameInput(name, newName);
    graph->renameInputSocket(name, newPortName);
    return newPortName;
}

RtOutput RtNodeGraph::createOutput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    PvtOutput* port = graph->createOutput(name, type, flags);
    graph->createOutputSocket(port->getName(), type, flags);
    return port->hnd();
}

void RtNodeGraph::removeOutput(const RtToken& name)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    graph->removeOutputSocket(name);
    graph->removeOutput(name);
}

RtToken RtNodeGraph::renameOutput(const RtToken& name, const RtToken& newName)
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    RtToken newPortName = graph->renameOutput(name, newName);
    graph->renameOutputSocket(name, newPortName);
    return newPortName;
}

RtOutput RtNodeGraph::getInputSocket(const RtToken& name) const
{
    PvtNodeGraphPrim* graph = prim()->asA< PvtNodeGraphPrim>();
    PvtOutput* socket = graph->getInputSocket(name);
    return socket ? socket->hnd() : RtOutput();
}

RtInput RtNodeGraph::getOutputSocket(const RtToken& name) const
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
        RtTypedValue* attr = input->getAttribute(Tokens::UIFOLDER, RtType::STRING);
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
    RtTokenSet processed;
    PvtObjectList newInputList;
    PvtObjectList newOutputList;
    PvtObjectList newInputSocketList;
    PvtObjectList newOutputSocketList;
    for (const RtToken& name : layout.order)
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
            RtTypedValue* attr = input->createAttribute(Tokens::UIFOLDER, RtType::STRING);
            attr->asString() = it->second;
        }
        else
        {
            input->removeAttribute(Tokens::UIFOLDER);
        }
    }
}

RtPrim RtNodeGraph::getNode(const RtToken& name) const
{
    PvtPrim* p = prim()->getChild(name);
    return p && p->getTypeInfo()->isCompatible(RtNode::typeName()) ? p->hnd() : RtPrim();
}

RtPrimIterator RtNodeGraph::getNodes() const
{
    RtSchemaPredicate<RtNode> predicate;
    return RtPrimIterator(hnd(), predicate);
}

void RtNodeGraph::setDefinition(const RtToken& nodedef)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::NODEDEF, RtType::TOKEN);
    attr->asToken() = nodedef;
}

const RtToken& RtNodeGraph::getDefinition() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::NODEDEF, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

void RtNodeGraph::setNamespace(const RtToken& namespaceString)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::NAMESPACE, RtType::TOKEN);
    attr->asToken() = namespaceString;
}

const RtToken& RtNodeGraph::getNamespace() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::NAMESPACE, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
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
