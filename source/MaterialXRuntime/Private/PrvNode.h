//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PRVNODE_H
#define MATERIALX_PRVNODE_H

#include <MaterialXRuntime/Private/PrvNodeDef.h>

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtValue.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

using RtPortVec = vector<RtPort>;

class PrvNode : public PrvAllocatingElement
{
public:
    static PrvObjectHandle createNew(PrvElement* parent, const PrvObjectHandle& nodedef, const RtToken& name);

    PrvObjectHandle getNodeDef() const
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
        PrvPortDef* portdef = nodeDef()->getPort(index);
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
    PrvNode(const RtToken& name, const PrvObjectHandle& nodedef);

    // Constructor creating a node without a fixed interface.
    // Used for constructing nodegraphs.
    PrvNode(const RtToken& name);

    PrvNodeDef* nodeDef() const
    {
        return _nodedef->asA<PrvNodeDef>();
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

    PrvObjectHandle _nodedef;
    vector<Port> _ports;

    friend class RtPort;
    friend class PrvNodeGraph;
};

}

#endif
