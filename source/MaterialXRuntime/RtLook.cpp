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

namespace
{
    class PvtLookGroupPrimSpec : public PvtPrimSpec
    {
    public:
        PvtLookGroupPrimSpec()
        {
            // TODO: We should derive this from a data driven XML schema.
            addPrimAttribute(Tokens::DOC, RtType::STRING);
            addPrimAttribute(Tokens::XPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::YPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::WIDTH, RtType::INTEGER);
            addPrimAttribute(Tokens::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Tokens::UICOLOR, RtType::COLOR3);
            addPrimAttribute(Tokens::LOOKS, RtType::STRING);
            addPrimAttribute(Tokens::ACTIVELOOK, RtType::STRING);
            addPrimAttribute(Tokens::DEFAULT, RtType::STRING);
        }
    };

    class PvtLookPrimSpec : public PvtPrimSpec
    {
    public:
        PvtLookPrimSpec()
        {
            // TODO: We should derive this from a data driven XML schema.
            addPrimAttribute(Tokens::DOC, RtType::STRING);
            addPrimAttribute(Tokens::XPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::YPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::WIDTH, RtType::INTEGER);
            addPrimAttribute(Tokens::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Tokens::UICOLOR, RtType::COLOR3);
        }
    };

    class PvtMaterialAssignPrimSpec : public PvtPrimSpec
    {
    public:
        PvtMaterialAssignPrimSpec()
        {
            // TODO: We should derive this from a data driven XML schema.
            addPrimAttribute(Tokens::DOC, RtType::STRING);
            addPrimAttribute(Tokens::XPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::YPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::WIDTH, RtType::INTEGER);
            addPrimAttribute(Tokens::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Tokens::UICOLOR, RtType::COLOR3);
            addPrimAttribute(Tokens::GEOM, RtType::STRING);
            addPrimAttribute(Tokens::EXCLUSIVE, RtType::BOOLEAN);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtLookGroup, "bindelement:lookgroup");

RtPrim RtLookGroup::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("lookgroup1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Tokens::LOOKS);

    return primH;
}

const RtPrimSpec& RtLookGroup::getPrimSpec() const
{
    static const PvtLookGroupPrimSpec s_lookGroupSpec;
    return s_lookGroupSpec;
}

void RtLookGroup::setActiveLook(const string& look)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::ACTIVELOOK, RtType::STRING);
    attr->asString() = look;
}

const string& RtLookGroup::getActiveLook() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::ACTIVELOOK);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtLookGroup::addLook(const RtObject& look)
{
    getLooks().connect(look);
}

void RtLookGroup::removeLook(const RtObject& look)
{
    getLooks().disconnect(look);
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
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Tokens::INHERIT);
    prim->createRelationship(Tokens::MATERIALASSIGN);

    return primH;
}

const RtPrimSpec& RtLook::getPrimSpec() const
{
    static const PvtLookPrimSpec s_primSpec;
    return s_primSpec;
}

RtRelationship RtLook::getInherit() const
{
    return prim()->getRelationship(Tokens::INHERIT)->hnd();
}

void RtLook::addMaterialAssign(const RtObject& assignment)
{
    getMaterialAssigns().connect(assignment);
}

void RtLook::removeMaterialAssign(const RtObject& assignment)
{
    getMaterialAssigns().disconnect(assignment);
}

RtRelationship RtLook::getMaterialAssigns() const
{
    return prim()->getRelationship(Tokens::MATERIALASSIGN)->hnd();
}

bool RtLookConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Tokens::INHERIT)
    {
        // 'inherit' relationship only accepts other looks as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtLook>();
    }
    else if (rel.getName() == Tokens::MATERIALASSIGN)
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
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createInput(Tokens::MATERIAL, RtType::MATERIAL);
    prim->createRelationship(Tokens::COLLECTION);

    return primH;
}

const RtPrimSpec& RtMaterialAssign::getPrimSpec() const
{
    static const PvtMaterialAssignPrimSpec s_primSpec;
    return s_primSpec;
}

RtInput RtMaterialAssign::getMaterial() const
{
    return prim()->getInput(Tokens::MATERIAL)->hnd();
}

RtRelationship RtMaterialAssign::getCollection() const
{
    return prim()->getRelationship(Tokens::COLLECTION)->hnd();
}

void RtMaterialAssign::setGeom(const string& geom)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::GEOM, RtType::STRING);
    attr->asString() = geom;
}

const string& RtMaterialAssign::getGeom() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::GEOM, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtMaterialAssign::setExclusive(bool exclusive)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::EXCLUSIVE, RtType::BOOLEAN);
    attr->asBool() = exclusive;
}

bool RtMaterialAssign::getExclusive() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::EXCLUSIVE, RtType::BOOLEAN);
    return attr ? attr->asBool() : false;
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
