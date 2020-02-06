//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTLOOK_H
#define MATERIALX_RTLOOK_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtSchema.h>

namespace MaterialX
{

/// @class RtLookGroup
/// Schema for lookgroup prims.
class RtLookGroup : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtLookGroup)

public:
    /// Return the active look.
    RtAttribute getActiveLook() const;

    void addLook(const RtObject& look);

    void removeLook(const RtObject& look);

    /// Return the referenced looks.
    RtRelationship getLooks() const;
};


/// @class RtLook
/// Schema for look prims.
class RtLook : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtLook)

public:
    /// Return the inherit relationship.
    RtRelationship getInherit() const;

    /// Return an iterator over the material assigns in the look.
    RtPrimIterator getMaterialAssigns() const;
};

/// @class RtMaterialAssign
/// Schema for materialassign prims.
class RtMaterialAssign : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtMaterialAssign)

public:
    /// Return the material relationship.
    RtRelationship getMaterial() const;

    /// Return the collection relationship.
    RtRelationship getCollection() const;

    /// Return the exclusive flag attribute.
    RtAttribute getExclusive() const;
};

}

#endif
