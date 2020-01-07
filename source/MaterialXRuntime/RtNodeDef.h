//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODEDEF_H
#define MATERIALX_RTNODEDEF_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtElement.h>

namespace MaterialX
{

/// @class RtNodeDef
/// API for accessing a node definition. This API can only be
/// attached to objects of type NODEDEF.
class RtNodeDef : public RtElement
{
public:
    /// Constructor attaching and object to the API.
    RtNodeDef(const RtObject& obj);

    /// Create a new nodedef in a stage.
    static RtObject createNew(RtObject stage, const RtToken& name, const RtToken& nodeName);

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Return the node name.
    const RtToken& getNodeName() const;

    /// Add a port to the definition
    void addPort(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove a port from the definition.
    void removePort(RtObject portdef);

    /// Return the port count.
    size_t numPorts() const;

    /// Return the output count.
    size_t numOutputs() const;

    /// Return a port definition by index,
    /// or a null object if no such port exists.
    RtObject getPort(size_t index) const;

    /// Get the index offset for outputs.
    /// This index points to the first output.
    size_t getOutputsOffset() const;

    /// Get the index offset for inputs.
    /// This index points to the first input.
    size_t getInputsOffset() const;

    /// Get the i:th output port definition,
    /// or a null object if no such port exists.
    RtObject getOutput(size_t index) const
    {
        return getPort(getOutputsOffset() + index);
    }

    /// Get the i:th input port definition,
    /// or a null object if no such port exists.
    RtObject getInput(size_t index) const
    {
        return getPort(getInputsOffset() + index);
    }

    /// Find a port definition by name.
    /// Return a null object if no such port is found.
    RtObject findPort(const RtToken& name) const;
};

}

#endif
