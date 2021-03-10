//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtRelationship.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPath.h>

#include <MaterialXRuntime/RtConnectableApi.h>

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(PvtRelationship, RtObjType::RELATIONSHIP, "PvtRelationship")

PvtRelationship::PvtRelationship(const RtToken& name, PvtPrim* parent) :
    PvtObject(name, parent)
{
    setTypeBit<PvtRelationship>();
}

void PvtRelationship::connect(PvtObject* obj)
{
    // Check if this relationship exists already.
    // Linear search here not ideal for performance, but we need the relationship ordering
    // so must maintain a vector here. If performance ever gets noticable we could add an
    // extra set/map if we can affor the storage.
    for (auto it = _connections.begin(); it != _connections.end(); ++it)
    {
        if (*it == obj)
        {
            // Relationship already exists
            return;
        }
    }

    // Validate the relationship with this prims connectable API.
    RtConnectableApi* connectableApi = RtConnectableApi::get(getParent()->prim());
    if (!(connectableApi && connectableApi->acceptRelationship(hnd(), obj->obj())))
    {
        throw ExceptionRuntimeError("'" + getPath().asString() + "' rejected the relationship");
    }

    // Create the relationship.
    _connections.push_back(obj);
}

void PvtRelationship::disconnect(PvtObject* obj)
{
    for (auto it = _connections.begin(); it != _connections.end(); ++it)
    {
        if (*it == obj)
        {
            _connections.erase(it);
            break;
        }
    }
}

}
