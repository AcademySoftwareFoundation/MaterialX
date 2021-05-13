//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtBackdrop.h>

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
            addPrimAttribute(RtString::DOC, RtType::STRING);
            addPrimAttribute(RtString::XPOS, RtType::FLOAT);
            addPrimAttribute(RtString::YPOS, RtType::FLOAT);
            addPrimAttribute(RtString::WIDTH, RtType::INTEGER);
            addPrimAttribute(RtString::HEIGHT, RtType::INTEGER);
            addPrimAttribute(RtString::UICOLOR, RtType::COLOR3);
            addPrimAttribute(RtString::UINAME, RtType::STRING);
            addPrimAttribute(RtString::CONTAINS, RtType::STRINGARRAY);
            addPrimAttribute(RtString::MINIMIZED, RtType::BOOLEAN);

        }
    };
}

DEFINE_TYPED_SCHEMA(RtBackdrop, "backdrop");

RtPrim RtBackdrop::createPrim(const RtString& typeName, const RtString& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name);

    static const RtString DEFAULT_NAME("backdrop1");
    const RtString primName = name.empty() ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(RtString::CONTAINS);

    return primH;
}

const RtPrimSpec& RtBackdrop::getPrimSpec() const
{
    static const PvtBackdropPrimSpec s_primSpec;
    return s_primSpec;
}

RtRelationship RtBackdrop::getContains() const
{
    return prim()->getRelationship(RtString::CONTAINS)->hnd();
}

void RtBackdrop::setNote(const string& note)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::NOTE, RtType::STRING);
    attr->asString() = note;
}

const string& RtBackdrop::getNote() const
{
    const RtTypedValue* attr = prim()->getAttribute(RtString::NOTE, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtBackdrop::setWidth(float width)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::WIDTH, RtType::FLOAT);
    attr->asFloat() = width;
}

float RtBackdrop::getWidth() const
{
    const RtTypedValue* attr = prim()->getAttribute(RtString::WIDTH, RtType::FLOAT);
    return attr ? attr->asFloat() : 0.0f;
}

void RtBackdrop::setHeight(float width)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::HEIGHT, RtType::FLOAT);
    attr->asFloat() = width;
}

float RtBackdrop::getHeight() const
{
    const RtTypedValue* attr = prim()->getAttribute(RtString::HEIGHT, RtType::FLOAT);
    return attr ? attr->asFloat() : 0.0f;
}

}
