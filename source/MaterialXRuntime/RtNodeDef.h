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

/// @struct RtNodeLayout
/// Container for node layout information.
struct RtNodeLayout
{
    RtTokenVec order;
    RtTokenMap<string> uifolder;
};

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

    /// Return the namespaced node for this nodedef.
    RtToken getNamespacedNode() const;

    /// Set the node for this nodedef.
    void setNode(const RtToken& node);

    /// Return the node group for this nodedef.
    const RtToken& getNodeGroup() const;

    /// Set the node for this nodedef.
    void setNodeGroup(const RtToken& nodegroup);

    /// Return the target for this nodedef.
    const RtToken& getTarget() const;

    /// Set the target for this nodedef.
    void setTarget(const RtToken& nodegroup);

    /// Return the inheritance for this nodedef.
    const RtToken& getIneritance() const;

    /// Set the inheritance for this nodedef.
    void setIneritance(const RtToken& inherit);

    /// Return the version for this nodedef.
    const RtToken& getVersion() const;

    /// Set the version for this nodedef.
    void setVersion(const RtToken& version);

    /// Is the version for this definition compatible with the version passed in
    bool isVersionCompatible(const RtToken& version) const;

    /// Return if this definition is the default version.
    bool getIsDefaultVersion() const;

    /// Set the version for this nodedef.
    void setIsDefaultVersion(bool isDefault);

    /// Return the namespace for this nodedef.
    const string& getNamespace() const;

    /// Set the namespace for this nodedef.
    void setNamespace(const string& space);

    /// Add an input attribute to the interface.
    RtInput createInput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove an input attribute from the interface.
    void removeInput(const RtToken& name);

    /// Add an output attribute to the interface.
    RtOutput createOutput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove an output attribute from the interface.
    void removeOutput(const RtToken& name);

    /// Return the number of inputs on the interface.
    size_t numInputs() const;

    /// Return the named input.
    RtInput getInput(const RtToken& name) const;

    /// Return an iterator traversing all input attributes.
    RtAttrIterator getInputs() const;

    /// Return the number of outputs on the interface.
    size_t numOutputs() const;

    /// Return the named output.
    RtOutput getOutput(const RtToken& name) const;

    /// Return the single output for single output nodes.
    /// Or if multiple outputs are available return the 
    /// last created output.
    RtOutput getOutput() const;

    /// Return an iterator traversing all output attributes.
    RtAttrIterator getOutputs() const;

    /// Return the relationship maintaining all node implementations registered for this nodedef.
    RtRelationship getNodeImpls() const;

    /// Return the node implementation prim for this nodedef matching the given target.
    /// If no such implementation can be found a null prim is returned.
    RtPrim getNodeImpl(const RtToken& target) const;

    /// Return a node layout struct for this nodedef.
    /// Containing its input ordering and uifolder hierarchy.
    RtNodeLayout getNodeLayout();

    /// Returns a vector of public nodegraph metadata names
    const RtTokenVec& getPublicMetadataNames() const override;

    /// Returns a vector of public metadata names for a port.
    const RtTokenVec& getPublicPortMetadataNames(const RtToken& name) const override;
};

}

#endif
