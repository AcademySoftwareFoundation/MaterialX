//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtLook.h>
#include <MaterialXRuntime/RtCollection.h>
#include <MaterialXRuntime/Tokens.h>
#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

DEFINE_TYPED_SCHEMA(RtLookGroup, "bindelement:lookgroup");

RtPrim RtLookGroup::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("lookgroup1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createAttribute(Tokens::ACTIVELOOK, RtType::STRING);
    prim->createRelationship(Tokens::LOOKS);

    return primH;
}

RtAttribute RtLookGroup::getActiveLook() const
{
    return prim()->getAttribute(Tokens::ACTIVELOOK)->hnd();
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
    return prim()->getRelationship(Tokens::LOOKS)->hnd();
}

bool RtLookGroupConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Tokens::LOOKS)
    {
        // 'looks' relationship only accepts looks or lookgroups as target.
        return target.isA<RtPrim>() && 
            (target.asA<RtPrim>().hasApi<RtLook>() || target.asA<RtPrim>().hasApi<RtLookGroup>());
    }
    return false;
}


DEFINE_TYPED_SCHEMA(RtLook, "bindelement:look");

RtPrim RtLook::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("look1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Tokens::INHERIT);
    prim->createRelationship(Tokens::MATERIAL_ASSIGN);

    return primH;
}

RtRelationship RtLook::getInherit() const
{
    return prim()->getRelationship(Tokens::INHERIT)->hnd();
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
    return prim()->getRelationship(Tokens::MATERIAL_ASSIGN)->hnd();
}

bool RtLookConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Tokens::INHERIT)
    {
        // 'inherit' relationship only accepts other looks as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtLook>();
    }
    else if (rel.getName() == Tokens::MATERIAL_ASSIGN)
    {
        // 'materialassign' relationship only accepts materialassigns as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtMaterialAssign>();
    }
    return false;
}


DEFINE_TYPED_SCHEMA(RtMaterialAssign, "bindelement:materialassign");

RtPrim RtMaterialAssign::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("materialassign1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createInput(Tokens::MATERIAL, RtType::MATERIAL);
    PvtAttribute* exclusive = prim->createAttribute(Tokens::EXCLUSIVE, RtType::BOOLEAN);
    exclusive->getValue().asBool() = true;
    prim->createAttribute(Tokens::GEOM, RtType::STRING);
    prim->createRelationship(Tokens::COLLECTION);

    return primH;
}

RtInput RtMaterialAssign::getMaterial() const
{
    return prim()->getInput(Tokens::MATERIAL)->hnd();
}

RtRelationship RtMaterialAssign::getCollection() const
{
    return prim()->getRelationship(Tokens::COLLECTION)->hnd();
}

RtAttribute RtMaterialAssign::getGeom() const
{
    return prim()->getAttribute(Tokens::GEOM)->hnd();
}

RtAttribute RtMaterialAssign::getExclusive() const
{
    return prim()->getAttribute(Tokens::EXCLUSIVE)->hnd();
}

bool RtMaterialAssignConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Tokens::COLLECTION)
    {
        // 'collection' relationship only accepts other collections as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtCollection>();
    }
    return false;
}

}
