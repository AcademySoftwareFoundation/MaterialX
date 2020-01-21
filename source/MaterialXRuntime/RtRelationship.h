//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTRELATIONSHIP_H
#define MATERIALX_RTRELATIONSHIP_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtPathItem.h>
#include <MaterialXRuntime/RtTraversal.h>

namespace MaterialX
{

/// @class RtRelationship
/// API for accessing a relationship on a prim.
class RtRelationship : public RtPathItem
{
public:
    /// Constructor attaching an object to the API.
    RtRelationship(const RtObject& obj);

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Return the name of this relationship.
    const RtToken& getName() const;

    /// Return true if this relationship has any targets.
    bool hasTargets() const;

    /// Add a target to this relationship.
    void addTarget(const RtObject& target);

    /// Remove a target from this relationship.
    void removeTarget(const RtObject& target);

    /// Clear all targets from this relationship.
    void clearTargets();

    /// Return an iterator over all targets for this relationship.
    RtConnectionIterator getTargets() const;
};

}

#endif
