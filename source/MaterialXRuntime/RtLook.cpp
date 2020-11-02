//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtLook.h>
#include <MaterialXRuntime/RtCollection.h>
#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXCore/Util.h>

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

    static const string MSG_NONE_ROOT_LOOKGROUP("A lookgroup can only be created at the top / root level");
    static const string MSG_NONE_ROOT_LOOK("A look can only be created at the top / root level");
    static const string MSG_NONE_ROOT_MATERIALASSIGN("A materialassign can only be created at the top / root level");
}

DEFINE_TYPED_SCHEMA(RtLookGroup, "bindelement:lookgroup");

RtPrim RtLookGroup::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeInfo.getShortTypeName())
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }
    PvtPath::throwIfNotRoot(parent.getPath(), MSG_NONE_ROOT_LOOKGROUP);

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

bool RtLookGroupConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == LOOKS)
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
    if (typeName != _typeInfo.getShortTypeName())
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }
    PvtPath::throwIfNotRoot(parent.getPath(), MSG_NONE_ROOT_LOOK);

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

bool RtLookConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == INHERIT)
    {
        // 'inherit' relationship only accepts other looks as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtLook>();
    }
    else if (rel.getName() == MATERIAL_ASSIGN)
    {
        // 'materialassign' relationship only accepts materialassigns as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtMaterialAssign>();
    }
    return false;
}


DEFINE_TYPED_SCHEMA(RtMaterialAssign, "bindelement:materialassign");

RtPrim RtMaterialAssign::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeInfo.getShortTypeName())
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }
    PvtPath::throwIfNotRoot(parent.getPath(), MSG_NONE_ROOT_MATERIALASSIGN);

    const RtToken primName = name == EMPTY_TOKEN ? MATERIALASSIGN1 : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createInput(MATERIAL, RtType::MATERIAL);
    PvtAttribute* exclusive = prim->createAttribute(EXCLUSIVE, RtType::BOOLEAN);
    exclusive->getValue().asBool() = true;
    prim->createAttribute(GEOM, RtType::STRING);
    prim->createRelationship(COLLECTION);

    return primH;
}

RtInput RtMaterialAssign::getMaterial() const
{
    return prim()->getInput(MATERIAL)->hnd();
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

bool RtMaterialAssignConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == COLLECTION)
    {
        // 'collection' relationship only accepts other collections as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtCollection>();
    }
    return false;
}

}
