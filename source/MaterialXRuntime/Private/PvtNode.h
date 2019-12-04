//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTNODE_H
#define MATERIALX_PVTNODE_H

#include <MaterialXRuntime/Private/PvtNodeDef.h>

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtTypeDef.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

using RtPortVec = vector<RtPort>;

class PvtNode : public PvtAllocatingElement
{
public:
    static PvtDataHandle createNew(PvtElement* parent, const PvtDataHandle& nodedef, const RtToken& name);

    PvtDataHandle getNodeDef() const
    {
        return _nodedef;
    }

    const RtToken& getNodeName() const
    {
        return nodeDef()->getNodeName();
    }

    size_t numPorts() const
    {
        return nodeDef()->numPorts();
    }

    size_t numOutputs() const
    {
        return nodeDef()->numOutputs();
    }

    size_t numInputs() const
    {
        return numPorts() - numOutputs();
    }

    size_t getOutputsOffset() const
    {
        return nodeDef()->getOutputsOffset();
    }

    size_t getInputsOffset() const
    {
        return nodeDef()->getInputsOffset();
    }

    RtPort getPort(size_t index)
    {
        PvtPortDef* portdef = nodeDef()->getPort(index);
        return portdef ? RtPort(shared_from_this(), index) : RtPort();
    }

    RtPort findPort(const RtToken& name)
    {
        const size_t index = nodeDef()->findPortIndex(name);
        return index != INVALID_INDEX ? RtPort(shared_from_this(), index) : RtPort();
    }

    RtAttribute* addPortAttribute(size_t index, const RtToken& attrName, const RtToken& attrType, uint32_t attrFlags = 0);

    void removePortAttribute(size_t index, const RtToken& attrName);

    RtAttribute* getPortAttribute(size_t index, const RtToken& attrName);

    const RtAttribute* getPortAttribute(size_t index, const RtToken& attrName) const
    {
        return const_cast<PvtNode*>(this)->getPortAttribute(index, attrName);
    }

    static void connect(const RtPort& source, const RtPort& dest);

    static void disconnect(const RtPort& source, const RtPort& dest);

    const RtToken& getPortColorSpace(size_t index) const
    {
        const RtAttribute* attr = getPortAttribute(index, ATTR_COLOR_SPACE);
        return attr ? attr->getValue().asToken() : EMPTY_TOKEN;
    }

    void setPortColorSpace(size_t index, const RtToken& colorspace)
    {
        RtAttribute* attr = getPortAttribute(index, ATTR_COLOR_SPACE);
        if (!attr)
        {
            attr = addPortAttribute(index, ATTR_COLOR_SPACE, RtType::TOKEN);
        }
        attr->getValue().asToken() = colorspace;
    }

    const RtToken& getPortUnit(size_t index) const
    {
        const RtAttribute* attr = getPortAttribute(index, ATTR_UNIT);
        return attr ? attr->getValue().asToken() : EMPTY_TOKEN;
    }

    void setPortUnit(size_t index, const RtToken& unit)
    {
        RtAttribute* attr = getPortAttribute(index, ATTR_UNIT);
        if (!attr)
        {
            attr = addPortAttribute(index, ATTR_UNIT, RtType::TOKEN);
        }
        attr->getValue().asToken() = unit;
    }

    // Attribute names.
    static const RtToken ATTR_COLOR_SPACE;
    static const RtToken ATTR_UNIT;

protected:
    // Constructor creating a node with a fixed interface
    // This is the constructor to use for ordinary nodes.
    PvtNode(const RtToken& name, const PvtDataHandle& nodedef);

    // Constructor creating a node without a fixed interface.
    // Used for constructing nodegraphs.
    PvtNode(const RtToken& name);

    PvtNodeDef* nodeDef() const
    {
        return _nodedef->asA<PvtNodeDef>();
    }

protected:
    struct Port
    {
        RtValue value;
        RtPortVec connections;
    };

    using PvtAttributeMapPtr = std::unique_ptr<PvtAttributeMap>;

    PvtDataHandle _nodedef;
    vector<Port> _ports;
    vector<PvtAttributeMapPtr> _portAttrs;

    friend class RtPort;
    friend class PvtNodeGraph;
};

}

#endif
