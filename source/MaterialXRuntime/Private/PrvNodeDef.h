//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PRVNODEDEF_H
#define MATERIALX_PRVNODEDEF_H

#include <MaterialXRuntime/Private/PrvElement.h>
#include <MaterialXRuntime/Private/PrvPortDef.h>

#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtValue.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PrvNodeDef : public PrvAllocatingElement
{
public:
    PrvNodeDef(const RtToken& name, const RtToken& category);

    static PrvObjectHandle createNew(PrvElement* parent, const RtToken& name, const RtToken& nodeName);

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

    PrvPortDef* getPort(size_t index) const
    {
        return getChild(index)->asA<PrvPortDef>();
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

    PrvPortDef* getOutput(size_t index) const
    {
        return getPort(getOutputsOffset() + index);
    }

    PrvPortDef* getInput(size_t index) const
    {
        return getPort(getInputsOffset() + index);
    }

    PrvPortDef* findPort(const RtToken& name) const
    {
        return findChildByName(name)->asA<PrvPortDef>();
    }

    size_t findPortIndex(const RtToken& name)
    {
        auto it = _portIndex.find(name);
        return it != _portIndex.end() ? it->second : INVALID_INDEX;
    }

protected:
    void addPort(PrvObjectHandle portdef);

    void rebuildPortIndex();

    RtToken _nodeName;
    size_t _numOutputs;
    RtTokenMap<size_t> _portIndex;
    friend class PrvNode;
    friend class PrvNodeGraph;
    friend class PrvPortDef;
    friend class RtPort;
};

}

#endif
