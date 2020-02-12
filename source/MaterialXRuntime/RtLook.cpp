//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Util.h>
#include <MaterialXRuntime/RtLook.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    static const RtToken INHERIT("inherit");
    static const RtToken MATERIAL("material");
    static const RtToken COLLECTION("collection");
    static const RtToken GEOM("geom");
    static const RtToken EXCLUSIVE("exclusive");
    static const RtToken MATERIAL_ASSIGN("materialassign");
    static const RtToken ACTIVELOOK("active");
    static const RtToken LOOKS("looks");

    static const RtToken LOOKGROUP1("lookgroup1");
    static const RtToken LOOK1("look1");
    static const RtToken MATERIALASSIGN1("materialassign1");
}

DEFINE_TYPED_SCHEMA(RtLookGroup, "lookgroup");

RtPrim RtLookGroup::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeInfo.getShortTypeName())
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }

    const RtToken primName = name == EMPTY_TOKEN ? LOOKGROUP1 : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createAttribute(ACTIVELOOK, RtType::STRING);
    prim->createRelationship(LOOKS);

    return primH;
}

RtAttribute RtLookGroup::getActiveLook() const
{
    return prim()->getAttribute(ACTIVELOOK)->hnd();
}

void RtLookGroup::addLook(const RtObject& look)
{
    getLooks().addTarget(look);
}

void RtLookGroup::removeLook(const RtObject& look)
{
    getLooks().removeTarget(look);
}

RtRelationship RtLookGroup::getLooks() const
{
    return prim()->getRelationship(LOOKS)->hnd();
}


DEFINE_TYPED_SCHEMA(RtLook, "look");

RtPrim RtLook::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeInfo.getShortTypeName())
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }

    const RtToken primName = name == EMPTY_TOKEN ? LOOK1 : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(INHERIT);
    prim->createRelationship(MATERIAL_ASSIGN);

    return primH;
}

RtRelationship RtLook::getInherit() const
{
    return prim()->getRelationship(INHERIT)->hnd();
}

void RtLook::addMaterialAssign(const RtObject& assignment)
{
    getMaterialAssigns().addTarget(assignment);
}

void RtLook::removeMaterialAssign(const RtObject& assignment)
{
    getMaterialAssigns().removeTarget(assignment);
}

RtRelationship RtLook::getMaterialAssigns() const
{
    return prim()->getRelationship(MATERIAL_ASSIGN)->hnd();
}


DEFINE_TYPED_SCHEMA(RtMaterialAssign, "materialassign");

RtPrim RtMaterialAssign::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeInfo.getShortTypeName())
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }

    const RtToken primName = name == EMPTY_TOKEN ? MATERIALASSIGN1 : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(MATERIAL);
    prim->createRelationship(COLLECTION);
    PvtAttribute* exclusive = prim->createAttribute(EXCLUSIVE, RtType::BOOLEAN);
    exclusive->getValue().asBool() = true;
    PvtAttribute* geom = prim->createAttribute(GEOM, RtType::STRING);
    geom->getValue().asString() = EMPTY_STRING;

    return primH;
}

RtRelationship RtMaterialAssign::getMaterial() const
{
    return prim()->getRelationship(MATERIAL)->hnd();
}

RtRelationship RtMaterialAssign::getCollection() const
{
    return prim()->getRelationship(COLLECTION)->hnd();
}

RtAttribute RtMaterialAssign::getGeom() const
{
    return prim()->getAttribute(GEOM)->hnd();
}

RtAttribute RtMaterialAssign::getExclusive() const
{
    return prim()->getAttribute(EXCLUSIVE)->hnd();
}

}
