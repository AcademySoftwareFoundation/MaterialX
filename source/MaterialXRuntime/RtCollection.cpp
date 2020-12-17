//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtCollection.h>
#include <MaterialXRuntime/Tokens.h>

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

DEFINE_TYPED_SCHEMA(RtCollection, "bindelement:collection");

RtPrim RtCollection::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("collection1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createAttribute(Tokens::INCLUDE_GEOM, RtType::STRING);
    prim->createAttribute(Tokens::EXCLUDE_GEOM, RtType::STRING);
    prim->createRelationship(Tokens::INCLUDE_COLLECTION);

    return primH;
}

RtAttribute RtCollection::getIncludeGeom() const
{
    return prim()->getAttribute(Tokens::INCLUDE_GEOM)->hnd();
}

RtAttribute RtCollection::getExcludeGeom() const
{
    return prim()->getAttribute(Tokens::EXCLUDE_GEOM)->hnd();
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
    return prim()->getRelationship(Tokens::INCLUDE_COLLECTION)->hnd();
}

bool RtCollectionConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Tokens::INCLUDE_COLLECTION)
    {
        // 'includecollection' only accepts other collection prims as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtCollection>();
    }
    return false;
}

}
