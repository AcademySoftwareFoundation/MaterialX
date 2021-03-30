//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTRELATIONSHIP_H
#define MATERIALX_RTRELATIONSHIP_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>

namespace MaterialX
{

class RtConnectionIterator;

/// @class RtRelationship
/// Object holding a relationship on a prim.
class RtRelationship : public RtObject
{
    RT_DECLARE_RUNTIME_OBJECT(RtRelationship)

public:
    /// Empty constructor.
    /// Creating an invalid object.
    RtRelationship() {}

    /// Construct from a handle.
    RtRelationship(PvtObjHandle hnd);

    /// Return the name of this relationship.
    const RtIdentifier& getName() const;

    /// Connect an object to this relationship.
    void connect(const RtObject& obj);

    /// Discconect an object from this relationship.
    void disconnect(const RtObject& obj);

    /// Return true if this relationship has any object connections.
    bool hasConnections() const;

    /// Return the number of object connections.
    size_t numConnections() const;

    /// Return an object by connection index.
    RtObject getConnection(size_t index = 0) const;

    /// Return an iterator over all object connections for this relationship.
    RtConnectionIterator getConnections() const;

    /// Clear all connections from this relationship.
    void clearConnections();

    /// Return all object names as a string.
    string getObjectNames() const;
};

}

#endif
