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
namespace
{
    class PvtCollectionPrimSpec : public PvtPrimSpec
    {
    public:
        PvtCollectionPrimSpec()
        {
            // TODO: We should derive this from a data driven XML schema.
            addPrimAttribute(Tokens::DOC, RtType::STRING);
            addPrimAttribute(Tokens::XPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::YPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::WIDTH, RtType::INTEGER);
            addPrimAttribute(Tokens::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Tokens::UICOLOR, RtType::COLOR3);
            addPrimAttribute(Tokens::INCLUDEGEOM, RtType::STRING);
            addPrimAttribute(Tokens::EXCLUDEGEOM, RtType::STRING);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtCollection, "bindelement:collection");

RtPrim RtCollection::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("collection1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Tokens::INCLUDECOLLECTION);

    return primH;
}

const RtPrimSpec& RtCollection::getPrimSpec() const
{
    static const PvtCollectionPrimSpec s_primSpec;
    return s_primSpec;
}

void RtCollection::setIncludeGeom(const string& geom)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::INCLUDEGEOM, RtType::STRING);
    attr->asString() = geom;
}

const string& RtCollection::getIncludeGeom() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::INCLUDEGEOM, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtCollection::setExcludeGeom(const string& geom)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::EXCLUDEGEOM, RtType::STRING);
    attr->asString() = geom;
}

const string& RtCollection::getExcludeGeom() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::EXCLUDEGEOM, RtType::STRING);
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
    return prim()->getRelationship(Tokens::INCLUDECOLLECTION)->hnd();
}

bool RtCollectionConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Tokens::INCLUDECOLLECTION)
    {
        // 'includecollection' only accepts other collection prims as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtCollection>();
    }
    return false;
}

}
