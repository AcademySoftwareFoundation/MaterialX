//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTNODEDEF_H
#define MATERIALX_PVTNODEDEF_H

#include <MaterialXRuntime/Private/PvtElement.h>
#include <MaterialXRuntime/Private/PvtPortDef.h>

#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtValue.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtNodeDef : public PvtAllocatingElement
{
public:
    PvtNodeDef(const RtToken& name, const RtToken& category);

    static PvtDataHandle createNew(PvtElement* parent, const RtToken& name, const RtToken& nodeName);

    const RtToken& getNodeName() const
    {
        return _nodeName;
    }

    void removePort(const RtToken& name);

    size_t numPorts() const
    {
        return numChildren();
    }

    size_t numOutputs() const
    {
        return _numOutputs;
    }

    size_t numInputs() const
    {
        return numPorts() - numOutputs();
    }

    PvtPortDef* getPort(size_t index) const
    {
        return getChild(index)->asA<PvtPortDef>();
    }

    size_t getOutputsOffset() const
    {
        // Outputs are stored first
        return 0;
    }

    size_t getInputsOffset() const
    {
        // Inputs are stored after the outputs
        return _numOutputs;
    }

    PvtPortDef* getOutput(size_t index) const
    {
        return getPort(getOutputsOffset() + index);
    }

    PvtPortDef* getInput(size_t index) const
    {
        return getPort(getInputsOffset() + index);
    }

    PvtPortDef* findPort(const RtToken& name) const
    {
        return findChildByName(name)->asA<PvtPortDef>();
    }

    size_t findPortIndex(const RtToken& name)
    {
        auto it = _portIndex.find(name);
        return it != _portIndex.end() ? it->second : INVALID_INDEX;
    }

protected:
    void addPort(PvtDataHandle portdef);

    void rebuildPortIndex();

    RtToken _nodeName;
    size_t _numOutputs;
    RtTokenMap<size_t> _portIndex;
    friend class PvtNode;
    friend class PvtNodeGraph;
    friend class PvtPortDef;
    friend class RtPort;
};

}

#endif
