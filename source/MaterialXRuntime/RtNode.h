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
class RtNode : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtNode)

public:
    /// Return the nodedef for this node.
    RtPrim getNodeDef() const;

    /// Return the named input.
    RtInput getInput(const RtToken& name) const;

    /// Return the named output.
    RtOutput getOutput(const RtToken& name) const;
};

}

#endif
