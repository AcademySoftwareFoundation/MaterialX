//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtCollection.h>

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    static const RtToken INCLUDE_GEOM("includegeom");
    static const RtToken EXCLUDE_GEOM("excludegeom");
    static const RtToken INCLUDE_COLLECTION("includecollection");

    static const RtToken COLLECTION1("collection1");

    static const string MSG_NONE_ROOT_COLLECTION("A collection can only be created at the top / root level");
}

DEFINE_TYPED_SCHEMA(RtCollection, "bindelement:collection");

RtPrim RtCollection::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeInfo.getShortTypeName())
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }
    PvtPath::throwIfNotRoot(parent.getPath(), MSG_NONE_ROOT_COLLECTION);

    const RtToken primName = name == EMPTY_TOKEN ? COLLECTION1 : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createAttribute(INCLUDE_GEOM, RtType::STRING);
    prim->createAttribute(EXCLUDE_GEOM, RtType::STRING);
    prim->createRelationship(INCLUDE_COLLECTION);

    return primH;
}

RtAttribute RtCollection::getIncludeGeom() const
{
    return prim()->getAttribute(INCLUDE_GEOM)->hnd();
}

RtAttribute RtCollection::getExcludeGeom() const
{
    return prim()->getAttribute(EXCLUDE_GEOM)->hnd();
}

void RtCollection::addCollection(const RtObject& collection)
{
    getIncludeCollection().addTarget(collection);
}

void RtCollection::removeCollection(const RtObject& collection)
{
    getIncludeCollection().removeTarget(collection);
}

RtRelationship RtCollection::getIncludeCollection() const
{
    return prim()->getRelationship(INCLUDE_COLLECTION)->hnd();
}

bool RtCollectionConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == INCLUDE_COLLECTION)
    {
        // 'includecollection' only accepts other collection prims as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtCollection>();
    }
    return false;
}

}
