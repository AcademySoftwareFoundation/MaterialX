//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtLook.h>
#include <MaterialXRuntime/RtCollection.h>
#include <MaterialXRuntime/Identifiers.h>
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
            addPrimAttribute(Identifiers::DOC, RtType::STRING);
            addPrimAttribute(Identifiers::XPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::YPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::WIDTH, RtType::INTEGER);
            addPrimAttribute(Identifiers::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Identifiers::UICOLOR, RtType::COLOR3);
            addPrimAttribute(Identifiers::LOOKS, RtType::STRING);
            addPrimAttribute(Identifiers::ENABLEDLOOKS, RtType::STRING);
            addPrimAttribute(Identifiers::DEFAULT, RtType::STRING);
        }
    };

    class PvtLookPrimSpec : public PvtPrimSpec
    {
    public:
        PvtLookPrimSpec()
        {
            // TODO: We should derive this from a data driven XML schema.
            addPrimAttribute(Identifiers::DOC, RtType::STRING);
            addPrimAttribute(Identifiers::XPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::YPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::WIDTH, RtType::INTEGER);
            addPrimAttribute(Identifiers::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Identifiers::UICOLOR, RtType::COLOR3);
        }
    };

    class PvtMaterialAssignPrimSpec : public PvtPrimSpec
    {
    public:
        PvtMaterialAssignPrimSpec()
        {
            // TODO: We should derive this from a data driven XML schema.
            addPrimAttribute(Identifiers::DOC, RtType::STRING);
            addPrimAttribute(Identifiers::XPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::YPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::WIDTH, RtType::INTEGER);
            addPrimAttribute(Identifiers::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Identifiers::UICOLOR, RtType::COLOR3);
            addPrimAttribute(Identifiers::GEOM, RtType::STRING);
            addPrimAttribute(Identifiers::EXCLUSIVE, RtType::BOOLEAN);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtLookGroup, "bindelement:lookgroup");

RtPrim RtLookGroup::createPrim(const RtIdentifier& typeName, const RtIdentifier& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtIdentifier DEFAULT_NAME("lookgroup1");
    const RtIdentifier primName = name == EMPTY_IDENTIFIER ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Identifiers::LOOKS);

    return primH;
}

const RtPrimSpec& RtLookGroup::getPrimSpec() const
{
    static const PvtLookGroupPrimSpec s_lookGroupSpec;
    return s_lookGroupSpec;
}

void RtLookGroup::setEnabledLooks(const string& looks)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::ENABLEDLOOKS, RtType::STRING);
    attr->asString() = looks;
}

const string& RtLookGroup::getEnabledLooks() const
{
    const RtTypedValue* attr = prim()->getAttribute(Identifiers::ENABLEDLOOKS);
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
    return prim()->getRelationship(Identifiers::LOOKS)->hnd();
}

bool RtLookGroupConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Identifiers::LOOKS)
    {
        // 'looks' relationship only accepts looks or lookgroups as target.
        return target.isA<RtPrim>() && 
            (target.asA<RtPrim>().hasApi<RtLook>() || target.asA<RtPrim>().hasApi<RtLookGroup>());
    }
    return false;
}


DEFINE_TYPED_SCHEMA(RtLook, "bindelement:look");

RtPrim RtLook::createPrim(const RtIdentifier& typeName, const RtIdentifier& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtIdentifier DEFAULT_NAME("look1");
    const RtIdentifier primName = name == EMPTY_IDENTIFIER ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Identifiers::INHERIT);
    prim->createRelationship(Identifiers::MATERIALASSIGN);

    return primH;
}

const RtPrimSpec& RtLook::getPrimSpec() const
{
    static const PvtLookPrimSpec s_primSpec;
    return s_primSpec;
}

RtRelationship RtLook::getInherit() const
{
    return prim()->getRelationship(Identifiers::INHERIT)->hnd();
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
    return prim()->getRelationship(Identifiers::MATERIALASSIGN)->hnd();
}

bool RtLookConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Identifiers::INHERIT)
    {
        // 'inherit' relationship only accepts other looks as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtLook>();
    }
    else if (rel.getName() == Identifiers::MATERIALASSIGN)
    {
        // 'materialassign' relationship only accepts materialassigns as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtMaterialAssign>();
    }
    return false;
}


DEFINE_TYPED_SCHEMA(RtMaterialAssign, "bindelement:materialassign");

RtPrim RtMaterialAssign::createPrim(const RtIdentifier& typeName, const RtIdentifier& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtIdentifier DEFAULT_NAME("materialassign1");
    const RtIdentifier primName = name == EMPTY_IDENTIFIER ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createInput(Identifiers::MATERIAL, RtType::MATERIAL);
    prim->createRelationship(Identifiers::COLLECTION);

    return primH;
}

const RtPrimSpec& RtMaterialAssign::getPrimSpec() const
{
    static const PvtMaterialAssignPrimSpec s_primSpec;
    return s_primSpec;
}

RtInput RtMaterialAssign::getMaterial() const
{
    return prim()->getInput(Identifiers::MATERIAL)->hnd();
}

RtRelationship RtMaterialAssign::getCollection() const
{
    return prim()->getRelationship(Identifiers::COLLECTION)->hnd();
}

void RtMaterialAssign::setGeom(const string& geom)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::GEOM, RtType::STRING);
    attr->asString() = geom;
}

const string& RtMaterialAssign::getGeom() const
{
    const RtTypedValue* attr = prim()->getAttribute(Identifiers::GEOM, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtMaterialAssign::setExclusive(bool exclusive)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::EXCLUSIVE, RtType::BOOLEAN);
    attr->asBool() = exclusive;
}

bool RtMaterialAssign::getExclusive() const
{
    const RtTypedValue* attr = prim()->getAttribute(Identifiers::EXCLUSIVE, RtType::BOOLEAN);
    return attr ? attr->asBool() : false;
}

bool RtMaterialAssignConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Identifiers::COLLECTION)
    {
        // 'collection' relationship only accepts other collections as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtCollection>();
    }
    return false;
}

}
