//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTBINDELEMENT_H
#define MATERIALX_RTBINDELEMENT_H

/// @file RtBindElement.h
/// TODO: Docs

#include <MaterialXRuntime/RtSchema.h>
#include <MaterialXRuntime/RtTraversal.h>

namespace MaterialX
{

/// @class RtBindElement
/// Base class for all schemas handling material bindings.
class RtBindElement : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtBindElement)

public:
    /// Constructor.
    RtBindElement(const RtPrim& prim) : RtTypedSchema(prim) {}

    /// Return the named relationship.
    /// Shorthand for getPrim().getRelationship(name).
    RtRelationship getRelationship(const RtIdentifier& name) const
    {
        return getPrim().getRelationship(name);
    }

    /// Return an iterator traversing all relationships on this
    /// element. Shorthand for getPrim().getRelationships().
    RtRelationshipIterator getRelationships() const
    {
        return getPrim().getRelationships();
    }
};

}

#endif
