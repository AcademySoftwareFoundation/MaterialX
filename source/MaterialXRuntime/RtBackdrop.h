//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTBACKDROP_H
#define MATERIALX_RTBACKDROP_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtPrim.h>

namespace MaterialX
{

/// @class RtBackdrop
/// Schema for backdrop prims.
class RtBackdrop : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtBackdrop)

public:
    RtRelationship contains() const;
    RtAttribute note() const;
    RtAttribute width() const;
    RtAttribute height() const;
};


/// @class RtGeneric
/// Schema for generic prims.
class RtGeneric : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtGeneric)

public:
    const RtToken& getKind() const;
    void setKind(const RtToken& kind) const;
};


}

#endif
