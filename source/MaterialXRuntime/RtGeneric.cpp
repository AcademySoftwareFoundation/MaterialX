//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtGeneric.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

DEFINE_TYPED_SCHEMA(RtGeneric, "generic");

RtPrim RtGeneric::createPrim(const RtString& typeName, const RtString& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name);

    static const RtString DEFAULT_NAME("generic1");
    const RtString primName = name.empty() ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    return primH;
}

const RtPrimSpec& RtGeneric::getPrimSpec() const
{
    static const PvtPrimSpec s_primSpec;
    return s_primSpec;
}

const RtString& RtGeneric::getKind() const
{
    RtTypedValue* v = prim()->getAttribute(RtString::KIND, RtType::INTERNSTRING);
    return v ? v->getValue().asInternString() : RtString::UNKNOWN;
}

void RtGeneric::setKind(const RtString& kind)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::KIND, RtType::INTERNSTRING);
    attr->asInternString() = kind;
}

}
