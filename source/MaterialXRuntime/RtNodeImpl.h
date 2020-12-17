//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved. See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODEIMPL_H
#define MATERIALX_RTNODEIMPL_H

/// @file RtNodeImpl.h
/// TODO: Docs

#include <MaterialXRuntime/RtSchema.h>

namespace MaterialX
{

/// @class RtNodeImpl
/// Base class for schemas handling node implementation prims.
class RtNodeImpl : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtNodeImpl)

public:
    /// Constructor.
    RtNodeImpl(const RtPrim& prim) : RtTypedSchema(prim) {}

    /// Set an identifier for the target used by this implementation.
    void setTarget(const RtToken& target);

    /// Return an identifier for the target used by this implementation.
    /// If an empty token is returned, this prim supports all targets.
    const RtToken& getTarget() const;

    /// Set the name of the nodedef this implementation applies to.
    void setNodeDef(const RtToken& nodedef);

    /// Return the name of the nodedef this implementation applies to.
    const RtToken& getNodeDef() const;

    /// Set an alternative name for this node for this specific target.
    /// This allows one to say that for this particular target, the native
    /// node or shader is called something else but is functionally equivalent
    /// to the node described by the nodedef.
    void setImplName(const RtToken& implname);

    /// Return an alternative name for this node and this specific target.
    /// This allows one to say that for this particular target, the native
    /// node or shader is called something else but is functionally equivalent
    /// to the node described by the nodedef.
    const RtToken& getImplName() const;
};

}

#endif
