//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTGENERIC_H
#define MATERIALX_RTGENERIC_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtSchema.h>

MATERIALX_NAMESPACE_BEGIN


/// @class RtGeneric
/// Schema for generic/unknown prims.
class RtGeneric : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtGeneric)

public:
    /// Constructor.
    RtGeneric(const RtPrim& prim) : RtTypedSchema(prim) {}

    /// Get the kind for this generic prim,
    /// which gives its custom typename.
    const RtString& getKind() const;

    /// Set the kind for this generic prim,
    /// giving its custom typename.
    void setKind(const RtString& kind);
};

MATERIALX_NAMESPACE_END

#endif
