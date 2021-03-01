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
    static const RtToken SOCKETS("_nodegraph_internal_sockets");
}

DEFINE_TYPED_SCHEMA(RtNodeGraph, "node:nodegraph");

const RtTypeInfo RtNodeGraph::SOCKETS_TYPE_INFO("_nodegraph_internal_sockets");

RtPrim RtNodeGraph::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name);

    static const RtToken DEFAULT_NAME("nodegraph1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();

    // Add a child prim to hold the internal sockets.
    PvtDataHandle socketH = PvtPrim::createNew(&SOCKETS_TYPE_INFO, SOCKETS, prim);
    prim->addChildPrim(socketH->asA<PvtPrim>());

    return primH;
}

RtInput RtNodeGraph::createInput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    PvtPrim* socket = prim()->getChild(SOCKETS);
    RtInput input = prim()->createInput(name, type, flags)->hnd();
    socket->createOutput(input.getName(), type, flags | RtAttrFlag::SOCKET);
    return input;
}

void RtNodeGraph::removeInput(const RtToken& name)
{
    PvtInput* input = prim()->getInput(name);
    if (!(input && input->isA<PvtInput>()))
    {
        throw ExceptionRuntimeError("No input found with name '" + name.str() + "'");
    }
    PvtPrim* socket = prim()->getChild(SOCKETS);
    socket->removeAttribute(name);
    prim()->removeAttribute(name);
}

RtToken RtNodeGraph::renameInput(const RtToken& name, const RtToken& newName)
{
    PvtInput* input = prim()->getInput(name);
    if (!(input && input->isA<PvtInput>()))
    {
        throw ExceptionRuntimeError("No input found with name '" + name.str() + "'");
    }
    PvtPrim* socket = prim()->getChild(SOCKETS);
    RtToken newPrimAttrName = prim()->renameAttribute(name, newName);
    socket->setAttributeName(name, newPrimAttrName);
    return newPrimAttrName;
}

RtOutput RtNodeGraph::createOutput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    PvtPrim* socket = prim()->getChild(SOCKETS);
    RtOutput output = prim()->createOutput(name, type, flags)->hnd();
    socket->createInput(output.getName(), type, flags | RtAttrFlag::SOCKET);
    return output;
}

void RtNodeGraph::removeOutput(const RtToken& name)
{
    PvtOutput* output = prim()->getOutput(name);
    if (!(output && output->isA<PvtOutput>()))
    {
        throw ExceptionRuntimeError("No output found with name '" + name.str() + "'");
    }
    PvtPrim* socket = prim()->getChild(SOCKETS);
    socket->removeAttribute(name);
    prim()->removeAttribute(name);
}

RtToken RtNodeGraph::renameOutput(const RtToken& name, const RtToken& newName)
{
    PvtOutput* output = prim()->getOutput(name);
    if (!(output && output->isA<PvtOutput>()))
    {
        throw ExceptionRuntimeError("No output found with name '" + name.str() + "'");
    }
    PvtPrim* socket = prim()->getChild(SOCKETS);
    RtToken newPrimAttrName = prim()->renameAttribute(name, newName);
    socket->setAttributeName(name, newPrimAttrName);
    return newPrimAttrName;
}

RtOutput RtNodeGraph::getInputSocket(const RtToken& name) const
{
    PvtPrim* socket = prim()->getChild(SOCKETS);
    // Input socket is an output in practice.
    PvtOutput* output = socket->getOutput(name);
    return output ? output->hnd() : RtOutput();
}

RtInput RtNodeGraph::getOutputSocket(const RtToken& name) const
{
    PvtPrim* socket = prim()->getChild(SOCKETS);
    // Output socket is an input in practice.
    PvtInput* input = socket->getInput(name);
    return input ? input->hnd() : RtInput();
}

RtNodeLayout RtNodeGraph::getNodeLayout()
{
    RtNodeLayout layout;
    for (RtAttribute input : getInputs())
    {
        layout.order.push_back(input.getName());
        RtTypedValue* data = input.getMetadata(Tokens::UIFOLDER);
        if (data && data->getType() == RtType::STRING)
        {
            layout.uifolder[input.getName()] = data->getValue().asString();
        }
    }
    return layout;
}

void RtNodeGraph::setNodeLayout(const RtNodeLayout& layout)
{
    PvtPrim* p = prim();

    // Create a new order map with attributes in the specifed order 
    RtTokenSet processed;
    PvtDataHandleVec newAttrOrder;
    for (const RtToken& name : layout.order)
    {
        if (!processed.count(name))
        {
            PvtInput* input = p->getInput(name);
            if (input)
            {
                newAttrOrder.push_back(input->hnd());
            }
            processed.insert(name);
        }
    }

    // Move over any attributes that were not specifed in the new order.
    for (const PvtDataHandle& hnd : p->_attrOrder)
    {
        if (!processed.count(hnd->getName()))
        {
            newAttrOrder.push_back(hnd);
            processed.insert(hnd->getName());
        }
    }

    // Make sure all attributes were moved.
    if (newAttrOrder.size() != p->_attrOrder.size())
    {
        throw ExceptionRuntimeError("Failed setting new node layout for '" + getName().str() + "'. Changing the attribute count is not allowed.");
    }

    // Switch to the new order.
    p->_attrOrder = newAttrOrder;

    // Assign uifolder metadata.
    for (RtAttribute input : getInputs())
    {
        auto it = layout.uifolder.find(input.getName());
        if (it != layout.uifolder.end() && !it->second.empty())
        {
            RtTypedValue* data = input.getMetadata(Tokens::UIFOLDER);
            if (!data)
            {
                data = input.addMetadata(Tokens::UIFOLDER, RtType::STRING);
            }
            else if (data->getType() != RtType::STRING)
            {
                input.removeMetadata(Tokens::UIFOLDER);
                data = input.addMetadata(Tokens::UIFOLDER, RtType::STRING);
            }
            data->getValue().asString() = it->second;
        }
        else
        {
            input.removeMetadata(Tokens::UIFOLDER);
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

const RtToken& RtNodeGraph::getDefinition() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::NODEDEF);
    return v ? v->getValue().asToken() : EMPTY_TOKEN;
}

void RtNodeGraph::setDefinition(const RtToken& value)
{
    RtTypedValue* v = prim()->addMetadata(Tokens::NODEDEF, RtType::TOKEN);
    v->getValue().asToken() = value;
}

string RtNodeGraph::asStringDot() const
{
    string dot = "digraph {\n";

    // Add input/output interface boxes.
    dot += "    \"inputs\" ";
    dot += "[shape=box];\n";
    dot += "    \"outputs\" ";
    dot += "[shape=box];\n";


    RtObjTypePredicate<RtInput> inputFilter;

    // Add all nodes.
    for (const RtPrim prim : getNodes())
    {
        dot += "    \"" + prim.getName().str() + "\" ";
        dot += "[shape=ellipse];\n";
    }

    // Add connections inbetween nodes
    // and between nodes and input interface.
    for (const RtPrim node : getNodes())
    {
        const string dstName = node.getName().str();
        for (const RtObject inputObj : node.getAttributes(inputFilter))
        {
            const RtInput& input = static_cast<const RtInput&>(inputObj);
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

    PvtPrim* sockets = prim()->getChild(SOCKETS);

    // Add connections between nodes and output sockets.
    for (RtObject socketObj : sockets->getAttributes(inputFilter))
    {
        const RtInput& socket = static_cast<const RtInput&>(socketObj);
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
