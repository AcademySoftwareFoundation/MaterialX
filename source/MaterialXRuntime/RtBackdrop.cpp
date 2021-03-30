//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtBackdrop.h>
#include <MaterialXRuntime/Identifiers.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{
namespace
{
    // TODO: We should derive this from a data driven XML schema.
    class PvtBackdropPrimSpec : public PvtPrimSpec
    {
    public:
        PvtBackdropPrimSpec()
        {
            addPrimAttribute(Identifiers::DOC, RtType::STRING);
            addPrimAttribute(Identifiers::XPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::YPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::WIDTH, RtType::INTEGER);
            addPrimAttribute(Identifiers::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Identifiers::UICOLOR, RtType::COLOR3);
            addPrimAttribute(Identifiers::UINAME, RtType::STRING);
            addPrimAttribute(Identifiers::CONTAINS, RtType::STRINGARRAY);
            addPrimAttribute(Identifiers::MINIMIZED, RtType::BOOLEAN);

        }
    };
}

DEFINE_TYPED_SCHEMA(RtBackdrop, "backdrop");

RtPrim RtBackdrop::createPrim(const RtIdentifier& typeName, const RtIdentifier& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name);

    static const RtIdentifier DEFAULT_NAME("backdrop1");
    const RtIdentifier primName = name == EMPTY_IDENTIFIER ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Identifiers::CONTAINS);

    return primH;
}

const RtPrimSpec& RtBackdrop::getPrimSpec() const
{
    static const PvtBackdropPrimSpec s_primSpec;
    return s_primSpec;
}

RtRelationship RtBackdrop::getContains() const
{
    return prim()->getRelationship(Identifiers::CONTAINS)->hnd();
}

void RtBackdrop::setNote(const string& note)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::NOTE, RtType::STRING);
    attr->asString() = note;
}

const string& RtBackdrop::getNote() const
{
    const RtTypedValue* attr = prim()->getAttribute(Identifiers::NOTE, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtBackdrop::setWidth(float width)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::WIDTH, RtType::FLOAT);
    attr->asFloat() = width;
}

float RtBackdrop::getWidth() const
{
    const RtTypedValue* attr = prim()->getAttribute(Identifiers::WIDTH, RtType::FLOAT);
    return attr ? attr->asFloat() : 0.0f;
}

void RtBackdrop::setHeight(float width)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::HEIGHT, RtType::FLOAT);
    attr->asFloat() = width;
}

float RtBackdrop::getHeight() const
{
    const RtTypedValue* attr = prim()->getAttribute(Identifiers::HEIGHT, RtType::FLOAT);
    return attr ? attr->asFloat() : 0.0f;
}

}
