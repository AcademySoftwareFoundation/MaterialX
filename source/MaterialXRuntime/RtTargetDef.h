//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved. See LICENSE.txt for license.
//

#ifndef MATERIALX_RTTARGETDEF_H
#define MATERIALX_RTTARGETDEF_H

/// @file RtTargetDef.h
/// TODO: Docs

#include <MaterialXRuntime/RtSchema.h>

namespace MaterialX
{

/// @class RtTargetDef
/// Schemas for targetdef prims, handling the definition of an implementation target.
class RtTargetDef : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtNodeImpl)

public:
    /// Constructor.
    RtTargetDef(const RtPrim& prim) : RtTypedSchema(prim) {}

    /// Set the name of another targetdef this target inherits from.
    void setInherit(const RtToken& inherit);

    /// Return the name of another targetdef this target inherits from.
    const RtToken& getInherit() const;

    /// Return true if the given target name is matching this target
    /// either through a direct match or inheritance.
    bool isMatching(const RtToken& target) const;
};

}

#endif
