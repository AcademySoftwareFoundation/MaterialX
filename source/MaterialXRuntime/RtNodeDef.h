//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODEDEF_H
#define MATERIALX_RTNODEDEF_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtSchema.h>

namespace MaterialX
{

/// @class RtNodeDef
/// Schema for nodedef prims.
class RtNodeDef : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtNodeDef)

public:
    /// Constructor.
    RtNodeDef(const RtPrim& prim) : RtTypedSchema(prim) {}

    /// Return the node for this nodedef.
    const RtToken& getNode() const;

    /// Set the node for this nodedef.
    void setNode(const RtToken& node);

    /// Add an input attribute to the interface.
    RtInput createInput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove an input attribute from the interface.
    void removeInput(const RtToken& name);

    /// Add an output attribute to the interface.
    RtOutput createOutput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove an output attribute from the interface.
    void removeOutput(const RtToken& name);

    /// Return the named input.
    RtInput getInput(const RtToken& name) const;

    /// Return an iterator traversing all input attributes.
    RtAttrIterator getInputs() const;

    /// Return the named output.
    RtOutput getOutput(const RtToken& name) const;

    /// Return an iterator traversing all output attributes.
    RtAttrIterator getOutputs() const;

    /// Register this nodedef as a master prim
    /// to make it instantiable for node creation.
    void registerMasterPrim() const;

    /// Unregister this nodedef as a master prim.
    void unregisterMasterPrim() const;

    /// Return true if this nodedef is registerd as a master prim.
    bool isMasterPrim() const;
};

}

#endif
