//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTRELATIONSHIP_H
#define MATERIALX_PVTRELATIONSHIP_H

#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtPort.h>

#include <MaterialXRuntime/RtTraversal.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtRelationship : public PvtObject
{
    RT_DECLARE_RUNTIME_OBJECT(PvtRelationship)

public:
    PvtRelationship(const RtToken& name, PvtPrim* parent);

    void connect(PvtObject* obj);

    void disconnect(PvtObject* obj);

    bool hasConnections() const
    {
        return !_connections.empty();
    }

    size_t numConnections() const
    {
        return _connections.size();
    }

    RtObject getConnection(size_t index = 0) const
    {
        return _connections[index];
    }

    RtConnectionIterator getConnections() const
    {
        return RtConnectionIterator(this->obj());
    }

    void clearConnections()
    {
        _connections.clear();
    }

protected:
    PvtObjHandleVec _connections;
    friend class RtConnectionIterator;
};

}

#endif
