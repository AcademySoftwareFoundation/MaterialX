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
    class PvtCollectionPrimSpec : public PvtPrimSpec
    {
    public:
        PvtCollectionPrimSpec()
        {
            // TODO: We should derive this from a data driven XML schema.
            addPrimAttribute(RtString::DOC, RtType::STRING);
            addPrimAttribute(RtString::XPOS, RtType::FLOAT);
            addPrimAttribute(RtString::YPOS, RtType::FLOAT);
            addPrimAttribute(RtString::WIDTH, RtType::INTEGER);
            addPrimAttribute(RtString::HEIGHT, RtType::INTEGER);
            addPrimAttribute(RtString::UICOLOR, RtType::COLOR3);
            addPrimAttribute(RtString::INCLUDEGEOM, RtType::STRING);
            addPrimAttribute(RtString::EXCLUDEGEOM, RtType::STRING);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtCollection, "bindelement:collection");

RtPrim RtCollection::createPrim(const RtString& typeName, const RtString& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtString DEFAULT_NAME("collection1");
    const RtString primName = name.empty() ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(RtString::INCLUDECOLLECTION);

    return primH;
}

const RtPrimSpec& RtCollection::getPrimSpec() const
{
    static const PvtCollectionPrimSpec s_primSpec;
    return s_primSpec;
}

void RtCollection::setIncludeGeom(const string& geom)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::INCLUDEGEOM, RtType::STRING);
    attr->asString() = geom;
}

const string& RtCollection::getIncludeGeom() const
{
    const RtTypedValue* attr = prim()->getAttribute(RtString::INCLUDEGEOM, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtCollection::setExcludeGeom(const string& geom)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::EXCLUDEGEOM, RtType::STRING);
    attr->asString() = geom;
}

const string& RtCollection::getExcludeGeom() const
{
    const RtTypedValue* attr = prim()->getAttribute(RtString::EXCLUDEGEOM, RtType::STRING);
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
    return prim()->getRelationship(RtString::INCLUDECOLLECTION)->hnd();
}

bool RtCollectionConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == RtString::INCLUDECOLLECTION)
    {
        // 'includecollection' only accepts other collection prims as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtCollection>();
    }
    return false;
}

}
