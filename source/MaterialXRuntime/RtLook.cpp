//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtLook.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    static const RtToken INHERIT("inherit");
    static const RtToken MATERIAL("material");
    static const RtToken COLLECTION("collection");
    static const RtToken EXCLUSIVE("exclusive");
    static const RtToken ACTIVELOOK("active");
    static const RtToken LOOKS("looks");

    static const RtToken LOOKGROUP1("lookgroup1");
    static const RtToken LOOK1("look1");
    static const RtToken MATERIALASSIGN1("materialassign1");
}

DEFINE_TYPED_SCHEMA(RtLookGroup, "lookgroup");

RtPrim RtLookGroup::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeName)
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }

    const RtToken primName = name == EMPTY_TOKEN ? LOOK1 : name;
    PvtDataHandle primH = PvtPrim::createNew(primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->setTypeName(_typeName);

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
    RtRelationship rel = getLooks();
    rel.removeTarget(look);
}

void RtLookGroup::removeLook(const RtObject& look)
{
    RtRelationship rel = getLooks();
    rel.addTarget(look);
}

RtRelationship RtLookGroup::getLooks() const
{
    return prim()->getRelationship(LOOKS)->hnd();
}


DEFINE_TYPED_SCHEMA(RtLook, "look");

RtPrim RtLook::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeName)
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }

    const RtToken primName = name == EMPTY_TOKEN ? LOOK1 : name;
    PvtDataHandle primH = PvtPrim::createNew(primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->setTypeName(_typeName);
    prim->createRelationship(INHERIT);

    return primH;
}

RtRelationship RtLook::getInherit() const
{
    return prim()->getRelationship(INHERIT)->hnd();
}

RtPrimIterator RtLook::getMaterialAssigns() const
{
    RtSchemaPredicate<RtMaterialAssign> filter;
    return RtPrimIterator(hnd(), filter);
}


DEFINE_TYPED_SCHEMA(RtMaterialAssign, "materialassign");

RtPrim RtMaterialAssign::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeName)
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }

    const RtToken primName = name == EMPTY_TOKEN ? MATERIALASSIGN1 : name;
    PvtDataHandle primH = PvtPrim::createNew(primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->setTypeName(_typeName);
    prim->createRelationship(MATERIAL);
    prim->createRelationship(COLLECTION);
    PvtAttribute* exclusive = prim->createAttribute(EXCLUSIVE, RtType::BOOLEAN);
    exclusive->getValue().asBool() = true;

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

RtAttribute RtMaterialAssign::getExclusive() const
{
    return prim()->getAttribute(EXCLUSIVE)->hnd();
}

}
