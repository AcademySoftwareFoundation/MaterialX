//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PRVNODEGRAPH_H
#define MATERIALX_PRVNODEGRAPH_H

#include <MaterialXRuntime/Private/PrvElement.h>
#include <MaterialXRuntime/Private/PrvNode.h>

#include <MaterialXRuntime/RtObject.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PrvNodeGraph : public PrvNode
{
public:
    static PrvObjectHandle createNew(PrvElement* parent, const RtToken& name);

    void addNode(PrvObjectHandle node);

    void removeNode(const RtToken& name)
    {
        removeChild(name);
    }

    void removePort(const RtToken& name);

    RtPort getInputSocket(size_t index) const
    {
        const PrvPortDef* portdef = inputSocketsNodeDef()->getPort(index);
        return portdef ? RtPort(_inputSockets, index) : RtPort();
    }

    RtPort getOutputSocket(size_t index) const
    {
        const PrvPortDef* portdef = outputSocketsNodeDef()->getPort(index);
        return portdef ? RtPort(_outputSockets, index) : RtPort();
    }

    RtPort findInputSocket(const RtToken& name) const
    {
        const size_t index = inputSocketsNodeDef()->findPortIndex(name);
        return index != INVALID_INDEX ? RtPort(_inputSockets, index) : RtPort();
    }

    RtPort findOutputSocket(const RtToken& name) const
    {
        const size_t index = outputSocketsNodeDef()->findPortIndex(name);
        return index != INVALID_INDEX ? RtPort(_outputSockets, index) : RtPort();
    }

    PrvNode* getNode(size_t index) const
    {
        return (PrvNode*)getChild(index).get();
    }

    PrvNode* findNode(const RtToken& name) const
    {
        return (PrvNode*)findChildByName(name).get();
    }

    string asStringDot() const;

    // Token constants.
    static const RtToken UNPUBLISHED_NODEDEF;
    static const RtToken INPUT_SOCKETS_NODEDEF;
    static const RtToken OUTPUT_SOCKETS_NODEDEF;
    static const RtToken INPUT_SOCKETS;
    static const RtToken OUTPUT_SOCKETS;
    static const RtToken SOCKETS_NODE_NAME;

protected:
    PrvNodeGraph(const RtToken& name);

    void addPort(PrvObjectHandle portdef);

    PrvNodeDef* inputSocketsNodeDef() const { return (PrvNodeDef*)_inputSocketsNodeDef.get(); }
    PrvNode* inputSockets() const { return (PrvNode*)_inputSockets.get(); }

    PrvNodeDef* outputSocketsNodeDef() const { return (PrvNodeDef*)_outputSocketsNodeDef.get(); }
    PrvNode* outputSockets() const { return (PrvNode*)_outputSockets.get(); }

    PrvObjectHandle _inputSocketsNodeDef;
    PrvObjectHandle _inputSockets;
    PrvObjectHandle _outputSocketsNodeDef;
    PrvObjectHandle _outputSockets;
    friend class PrvPortDef;
};

}

#endif
