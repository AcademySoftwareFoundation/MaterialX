//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtBackdrop.h>
#include <MaterialXRuntime/Tokens.h>

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
            addPrimAttribute(Tokens::DOC, RtType::STRING);
            addPrimAttribute(Tokens::XPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::YPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::WIDTH, RtType::INTEGER);
            addPrimAttribute(Tokens::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Tokens::UICOLOR, RtType::COLOR3);
            addPrimAttribute(Tokens::UINAME, RtType::STRING);
            addPrimAttribute(Tokens::CONTAINS, RtType::STRINGARRAY);
            addPrimAttribute(Tokens::MINIMIZED, RtType::BOOLEAN);

        }
    };
}

DEFINE_TYPED_SCHEMA(RtBackdrop, "backdrop");

RtPrim RtBackdrop::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name);

    static const RtToken DEFAULT_NAME("backdrop1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Tokens::CONTAINS);

    return primH;
}

const RtPrimSpec& RtBackdrop::getPrimSpec() const
{
    static const PvtBackdropPrimSpec s_primSpec;
    return s_primSpec;
}

RtRelationship RtBackdrop::getContains() const
{
    return prim()->getRelationship(Tokens::CONTAINS)->hnd();
}

void RtBackdrop::setNote(const string& note)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::NOTE, RtType::STRING);
    attr->asString() = note;
}

const string& RtBackdrop::getNote() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::NOTE, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtBackdrop::setWidth(float width)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::WIDTH, RtType::FLOAT);
    attr->asFloat() = width;
}

float RtBackdrop::getWidth() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::WIDTH, RtType::FLOAT);
    return attr ? attr->asFloat() : 0.0f;
}

void RtBackdrop::setHeight(float width)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::HEIGHT, RtType::FLOAT);
    attr->asFloat() = width;
}

float RtBackdrop::getHeight() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::HEIGHT, RtType::FLOAT);
    return attr ? attr->asFloat() : 0.0f;
}

}
