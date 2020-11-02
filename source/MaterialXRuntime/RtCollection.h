//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTCOLLECTION_H
#define MATERIALX_RTCOLLECTION_H

/// @file
/// Classes related to collections of geometry identifiers

#include <MaterialXRuntime/RtBindElement.h>
#include <MaterialXRuntime/RtConnectableApi.h>

namespace MaterialX
{

/// @class RtCollection
/// Schema for collection prims.
class RtCollection : public RtBindElement
{
    DECLARE_TYPED_SCHEMA(RtCollection)

public:
    /// Constructor.
    RtCollection(const RtPrim& prim) : RtBindElement(prim) {}

    /// Return included geometry identifiers
    RtAttribute getIncludeGeom() const;

    /// Return excluded geometry identifiers
    RtAttribute getExcludeGeom() const;

    /// Add a child collection
    void addCollection(const RtObject& collection);

    /// Remove a child collection
    void removeCollection(const RtObject& collection);

    /// Return the referenced collections
    RtRelationship getIncludeCollection() const;
};

/// @class RtCollectionConnectableApi
/// API for validating connections and relationships for the 'collection' prim type.
class RtCollectionConnectableApi : public RtConnectableApi
{
public:
    bool acceptRelationship(const RtRelationship& rel, const RtObject& target) const override;
};

}

#endif
