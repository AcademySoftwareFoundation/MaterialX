//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTNODEGRAPH_H
#define MATERIALX_PVTNODEGRAPH_H

#include <MaterialXRuntime/Private/PvtElement.h>
#include <MaterialXRuntime/Private/PvtNode.h>

#include <MaterialXRuntime/RtObject.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtNodeGraph : public PvtNode
{
public:
    static PvtObjectHandle createNew(PvtElement* parent, const RtToken& name);

    void addNode(PvtObjectHandle node);

    void removeNode(const RtToken& name)
    {
        removeChild(name);
    }

    void removePort(const RtToken& name);

    RtPort getInputSocket(size_t index) const
    {
        const PvtPortDef* portdef = inputSocketsNodeDef()->getPort(index);
        return portdef ? RtPort(_inputSockets, index) : RtPort();
    }

    RtPort getOutputSocket(size_t index) const
    {
        const PvtPortDef* portdef = outputSocketsNodeDef()->getPort(index);
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

    PvtNode* getNode(size_t index) const
    {
        return (PvtNode*)getChild(index).get();
    }

    PvtNode* findNode(const RtToken& name) const
    {
        return (PvtNode*)findChildByName(name).get();
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
    PvtNodeGraph(const RtToken& name);

    void addPort(PvtObjectHandle portdef);

    PvtNodeDef* inputSocketsNodeDef() const { return (PvtNodeDef*)_inputSocketsNodeDef.get(); }
    PvtNode* inputSockets() const { return (PvtNode*)_inputSockets.get(); }

    PvtNodeDef* outputSocketsNodeDef() const { return (PvtNodeDef*)_outputSocketsNodeDef.get(); }
    PvtNode* outputSockets() const { return (PvtNode*)_outputSockets.get(); }

    PvtObjectHandle _inputSocketsNodeDef;
    PvtObjectHandle _inputSockets;
    PvtObjectHandle _outputSocketsNodeDef;
    PvtObjectHandle _outputSockets;
    friend class PvtPortDef;
};

}

#endif
