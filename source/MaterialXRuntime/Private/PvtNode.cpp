//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtNode.h>
#include <MaterialXRuntime/RtToken.h>
#include <MaterialXRuntime/RtTypeDef.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

const RtToken PvtNode::ATTR_COLOR_SPACE("colorspace");
const RtToken PvtNode::ATTR_UNIT("unit");

// Construction with an interface is for nodes.
PvtNode::PvtNode(const RtToken& name, const PvtObjectHandle& nodedef) :
    PvtAllocatingElement(RtObjType::NODE, name),
    _nodedef(nodedef)
{
    const size_t numPorts = nodeDef()->numPorts();
    _ports.resize(numPorts);
    _portAttrs.resize(numPorts);

    // Set indices and default values
    for (size_t i = 0; i < numPorts; ++i)
    {
        PvtPortDef* p = nodeDef()->getPort(i);
        _ports[i].value = p->getValue();
    }
}

// Construction without an interface is only for nodegraphs.
PvtNode::PvtNode(const RtToken& name) :
    PvtAllocatingElement(RtObjType::NODEGRAPH, name),
    _nodedef(nullptr)
{
}

PvtObjectHandle PvtNode::createNew(PvtElement* parent, const PvtObjectHandle& nodedef, const RtToken& name)
{
    if (parent && !(parent->hasApi(RtApiType::STAGE) || parent->hasApi(RtApiType::NODEGRAPH)))
    {
        throw ExceptionRuntimeError("Parent must be a stage or a nodegraph");
    }

    // If a name is not given generate one.
    // The name will be made unique if needed
    // when the node is added to the parent below.
    RtToken nodeName = name;
    if (nodeName == EMPTY_TOKEN)
    {
        nodeName = RtToken(nodedef->asA<PvtNodeDef>()->getNodeName().str() + "1");
    }

    PvtObjectHandle node(new PvtNode(nodeName, nodedef));
    if (parent)
    {
        parent->addChild(node);
    }

    return node;
}

RtAttribute* PvtNode::addPortAttribute(size_t index, const RtToken& attrName, const RtToken& attrType, uint32_t attrFlags)
{
    PvtAttributeMapPtr& entry = _portAttrs[index];
    if (!entry)
    {
        entry.reset(new PvtAttributeMap());
    }

    PvtAttributePtr attr = createAttribute(attrName, attrType, attrFlags);
    (*entry)[attrName] = attr;

    return attr.get();
}

void PvtNode::removePortAttribute(size_t index, const RtToken& attrName)
{
    const PvtAttributeMapPtr& entry = _portAttrs[index];
    if (entry)
    {
        entry->erase(attrName);
    }
}

RtAttribute* PvtNode::getPortAttribute(size_t index, const RtToken & attrName)
{
    const PvtAttributeMapPtr& entry = _portAttrs[index];
    if (entry)
    {
        auto it = entry->find(attrName);
        return it != entry->end() ? it->second.get() : nullptr;
    }
    return nullptr;
}

void PvtNode::connect(const RtPort& source, const RtPort& dest)
{
    if (dest.isConnected())
    {
        throw ExceptionRuntimeError("Destination port is already connected");
    }
    if (!(source.isOutput() && source.isConnectable()))
    {
        throw ExceptionRuntimeError("Source port is not a connectable output");
    }
    if (!(dest.isInput() && dest.isConnectable()))
    {
        throw ExceptionRuntimeError("Destination port is not a connectable input");
    }

    PvtNode* sourceNode = source._data->asA<PvtNode>();
    PvtNode* destNode = dest._data->asA<PvtNode>();
    RtPortVec& sourceConnections = sourceNode->_ports[source._index].connections;
    RtPortVec& destConnections = destNode->_ports[dest._index].connections;

    // Make room for the new source.
    destConnections.resize(1);
    destConnections[0] = source;
    sourceConnections.push_back(dest);
}

void PvtNode::disconnect(const RtPort& source, const RtPort& dest)
{
    PvtNode* destNode = dest._data->asA<PvtNode>();
    RtPortVec& destConnections = destNode->_ports[dest._index].connections;
    if (destConnections.size() != 1 || destConnections[0] != source)
    {
        throw ExceptionRuntimeError("Given source and destination is not connected");
    }

    destConnections.clear();

    PvtNode* sourceNode = source._data->asA<PvtNode>();
    RtPortVec& sourceConnections = sourceNode->_ports[source._index].connections;
    for (auto it = sourceConnections.begin(); it != sourceConnections.end(); ++it)
    {
        if (*it == dest)
        {
            sourceConnections.erase(it);
            break;
        }
    }
}

}
