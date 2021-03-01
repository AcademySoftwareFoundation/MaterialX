//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODE_H
#define MATERIALX_RTNODE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtSchema.h>

namespace MaterialX
{

/// @class RtNode
/// Schema for node prims.
class RtNode : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtNode)

public:
    /// Constructor.
    RtNode(const RtPrim& prim) : RtTypedSchema(prim) {}

    /// Return the nodedef for this node.
    RtPrim getNodeDef() const;

    /// Set the nodedef for this node.
    void setNodeDef(RtPrim prim);

    /// Return the version for this node.
    const RtToken& getVersion() const;

    /// Set the version for this node.
    void setVersion(const RtToken& version);

    /// Return the number of inputs on the node.
    size_t numInputs() const;

    /// Return the named input.
    RtInput getInput(const RtToken& name) const;

    /// Return an iterator traversing all input attributes
    /// on this node. Shorthand for getPrim().getInputs().
    RtAttrIterator getInputs() const;

    /// Return the number of outputs on the node.
    size_t numOutputs() const;

    /// Return the named output.
    RtOutput getOutput(const RtToken& name) const;

    /// Return the single output for single output nodes.
    /// Or if multiple outputs are available return the 
    /// last created output.
    RtOutput getOutput() const;

    /// Return an iterator traversing all output attributes
    /// on this node. Shorthand for getPrim().getOutputs().
    RtAttrIterator getOutputs() const;

    /// Returns a vector of public nodegraph metadata names
    const RtTokenVec& getPublicMetadataNames() const override;

    /// Returns a vector of public metadata names for a port.
    const RtTokenVec& getPublicPortMetadataNames(const RtToken& name) const override;


};

}

#endif

