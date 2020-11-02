//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTLOOK_H
#define MATERIALX_RTLOOK_H

/// @file RtLook.h
/// Specifications for a set of classes related to material binding.
/// This includes:
///     1. Material Assignments: which associate collections of geometry with materials
///     2. Looks: which contain one or more material assignments.
///     3. Look Groups: which reference a collection of looks with one look being active.

#include <MaterialXRuntime/RtBindElement.h>
#include <MaterialXRuntime/RtConnectableApi.h>

namespace MaterialX
{

/// @class RtLookGroup
/// Schema for 'lookgroup' prims.
class RtLookGroup : public RtBindElement
{
    DECLARE_TYPED_SCHEMA(RtLookGroup)

public:
    /// Constructor.
    RtLookGroup(const RtPrim& prim) : RtBindElement(prim) {}

    /// Return the active look.
    RtAttribute getActiveLook() const;

    /// Add a look
    void addLook(const RtObject& look);

    /// Remove a look
    void removeLook(const RtObject& look);

    /// Return the referenced looks.
    RtRelationship getLooks() const;
};

/// @class RtLookGroupConnectableApi
/// API for validating connections and relationships for the 'lookgroup' prim type.
class RtLookGroupConnectableApi : public RtConnectableApi
{
public:
    bool acceptRelationship(const RtRelationship& rel, const RtObject& target) const override;
};


/// @class RtLook
/// Schema for 'look' prims.
class RtLook : public RtBindElement
{
    DECLARE_TYPED_SCHEMA(RtLook)

public:
    /// Constructor.
    RtLook(const RtPrim& prim) : RtBindElement(prim) {}

    /// Return the inherit relationship.
    RtRelationship getInherit() const;

    /// Add a material assignment
    void addMaterialAssign(const RtObject& assignment);

    /// Remove a material assignment
    void removeMaterialAssign(const RtObject& assignment);

    /// Return an material assignment relationship.
    RtRelationship getMaterialAssigns() const;
};

/// @class RtLookConnectableApi
/// API for validating connections and relationships for the 'look' prim type.
class RtLookConnectableApi : public RtConnectableApi
{
public:
    bool acceptRelationship(const RtRelationship& rel, const RtObject& target) const override;
};


/// @class RtMaterialAssign
/// Schema for 'materialassign' prims.
class RtMaterialAssign : public RtBindElement
{
    DECLARE_TYPED_SCHEMA(RtMaterialAssign)

public:
    /// Constructor.
    RtMaterialAssign(const RtPrim& prim) : RtBindElement(prim) {}

    /// Return the material input.
    RtInput getMaterial() const;

    /// Return the collection relationship.
    RtRelationship getCollection() const;

    /// Return the geometry attribute.
    RtAttribute getGeom() const;

    /// Return the exclusive flag attribute.
    RtAttribute getExclusive() const;
};

/// @class RtMaterialAssignConnectableApi
/// API for validating connections and relationships for the 'materialssign' prim type.
class RtMaterialAssignConnectableApi : public RtConnectableApi
{
public:
    bool acceptRelationship(const RtRelationship& rel, const RtObject& target) const override;
};

}

#endif
