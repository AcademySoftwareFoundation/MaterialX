//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODE_H
#define MATERIALX_RTNODE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtSchema.h>
#include <MaterialXRuntime/RtTraversal.h>

namespace MaterialX
{

/// @class RtNode
/// Schema for node prims.
class RtNode : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtNode)
    static RtPrim createNode(RtPrim nodedef, const RtString& name, RtPrim parent);

public:
    /// Constructor.
    RtNode(const RtPrim& prim) : RtTypedSchema(prim) {}

    /// Set the nodedef for this node.
    void setNodeDef(RtPrim prim);

    /// Return the nodedef for this node.
    RtPrim getNodeDef() const;

    /// Set the version for this node.
    void setVersion(const RtString& version);

    /// Return the version for this node.
    const RtString& getVersion() const;

    /// Return a port (input or output) by name, or a null object
    /// if no such port exists.
    /// Shorthand for calling getPrim().getPort().
    RtPort getPort(const RtString& name) const
    {
        return getPrim().getPort(name);
    }

    /// Return the number of inputs on the node.
    /// Shorthand for calling getPrim().numInputs().
    size_t numInputs() const
    {
        return getPrim().numInputs();
    }

    /// Return an input by index.
    /// Shorthand for calling getPrim().getInput().
    RtInput getInput(size_t index) const
    {
        return getPrim().getInput(index);
    }

    /// Return an input by name.
    /// Shorthand for calling getPrim().getInput().
    RtInput getInput(const RtString& name) const
    {
        return getPrim().getInput(name);
    }

    /// Return an iterator over all inputs.
    /// Shorthand for calling getPrim().getInputs().
    RtInputIterator getInputs() const
    {
        return getPrim().getInputs();
    }

    /// Return the number of outputs on the node.
    /// Shorthand for calling getPrim().numOutputs().
    size_t numOutputs() const
    {
        return getPrim().numOutputs();
    }

    /// Return an output by index.
    /// Shorthand for calling getPrim().getOutput().
    RtOutput getOutput(size_t index = 0) const
    {
        return getPrim().getOutput(index);
    }

    /// Return an output by name.
    /// Shorthand for calling getPrim().getOutput().
    RtOutput getOutput(const RtString& name) const
    {
        return getPrim().getOutput(name);
    }

    /// Return an iterator over all outputs.
    /// Shorthand for calling getPrim().getOutputs().
    RtOutputIterator getOutputs() const
    {
        return getPrim().getOutputs();
    }
};

}

#endif
