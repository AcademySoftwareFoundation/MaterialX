//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTCOLLECTION_H
#define MATERIALX_RTCOLLECTION_H

/// @file
/// Classes related to collections of geometry identifiers

#include <MaterialXRuntime/RtSchema.h>

namespace MaterialX
{

/// @class RtCollection
/// Schema for collection prims.
class RtCollection: public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtCollection)

public:
    /// Constructor.
    RtCollection(const RtPrim& prim) : RtTypedSchema(prim) {}

    /// Return included geometry identifiers
    RtAttribute getIncludeGeom() const;

    /// Return excluded geometry identifiers
    RtAttribute getExcludeGeom() const;

    /// Add a collection
    void addCollection(const RtObject& collection);

    /// Remove a collection
    void removeCollection(const RtObject& collection);

    /// Return the referenced collections
    RtRelationship getIncludeCollection() const;
};

}

#endif
