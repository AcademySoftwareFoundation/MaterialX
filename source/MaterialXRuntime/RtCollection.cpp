//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtCollection.h>
#include <MaterialXRuntime/Identifiers.h>

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{
namespace
{
    class PvtCollectionPrimSpec : public PvtPrimSpec
    {
    public:
        PvtCollectionPrimSpec()
        {
            // TODO: We should derive this from a data driven XML schema.
            addPrimAttribute(Identifiers::DOC, RtType::STRING);
            addPrimAttribute(Identifiers::XPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::YPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::WIDTH, RtType::INTEGER);
            addPrimAttribute(Identifiers::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Identifiers::UICOLOR, RtType::COLOR3);
            addPrimAttribute(Identifiers::INCLUDEGEOM, RtType::STRING);
            addPrimAttribute(Identifiers::EXCLUDEGEOM, RtType::STRING);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtCollection, "bindelement:collection");

RtPrim RtCollection::createPrim(const RtIdentifier& typeName, const RtIdentifier& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtIdentifier DEFAULT_NAME("collection1");
    const RtIdentifier primName = name == EMPTY_IDENTIFIER ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Identifiers::INCLUDECOLLECTION);

    return primH;
}

const RtPrimSpec& RtCollection::getPrimSpec() const
{
    static const PvtCollectionPrimSpec s_primSpec;
    return s_primSpec;
}

void RtCollection::setIncludeGeom(const string& geom)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::INCLUDEGEOM, RtType::STRING);
    attr->asString() = geom;
}

const string& RtCollection::getIncludeGeom() const
{
    const RtTypedValue* attr = prim()->getAttribute(Identifiers::INCLUDEGEOM, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtCollection::setExcludeGeom(const string& geom)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::EXCLUDEGEOM, RtType::STRING);
    attr->asString() = geom;
}

const string& RtCollection::getExcludeGeom() const
{
    const RtTypedValue* attr = prim()->getAttribute(Identifiers::EXCLUDEGEOM, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtCollection::addCollection(const RtObject& collection)
{
    getIncludeCollection().connect(collection);
}

void RtCollection::removeCollection(const RtObject& collection)
{
    getIncludeCollection().disconnect(collection);
}

RtRelationship RtCollection::getIncludeCollection() const
{
    return prim()->getRelationship(Identifiers::INCLUDECOLLECTION)->hnd();
}

bool RtCollectionConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Identifiers::INCLUDECOLLECTION)
    {
        // 'includecollection' only accepts other collection prims as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtCollection>();
    }
    return false;
}

}
