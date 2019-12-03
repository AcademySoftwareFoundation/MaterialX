//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTNODE_H
#define MATERIALX_PVTNODE_H

#include <MaterialXRuntime/Private/PvtNodeDef.h>

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtValue.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

using RtPortVec = vector<RtPort>;

class PvtNode : public PvtAllocatingElement
{
public:
    static PvtObjectHandle createNew(PvtElement* parent, const PvtObjectHandle& nodedef, const RtToken& name);

    PvtObjectHandle getNodeDef() const
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

    static void connect(const RtPort& source, const RtPort& dest);

    static void disconnect(const RtPort& source, const RtPort& dest);

protected:
    // Constructor creating a node with a fixed interface
    // This is the constructor to use for ordinary nodes.
    PvtNode(const RtToken& name, const PvtObjectHandle& nodedef);

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
        Port();
        RtValue value;
        RtToken colorspace;
        RtToken unit;
        RtPortVec connections;
    };

    PvtObjectHandle _nodedef;
    vector<Port> _ports;

    friend class RtPort;
    friend class PvtNodeGraph;
};

}

#endif
