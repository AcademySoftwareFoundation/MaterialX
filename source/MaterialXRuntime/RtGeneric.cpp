//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtGeneric.h>
#include <MaterialXRuntime/Identifiers.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

DEFINE_TYPED_SCHEMA(RtGeneric, "generic");

RtPrim RtGeneric::createPrim(const RtIdentifier& typeName, const RtIdentifier& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name);

    static const RtIdentifier DEFAULT_NAME("generic1");
    const RtIdentifier primName = name == EMPTY_IDENTIFIER ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    return primH;
}

const RtPrimSpec& RtGeneric::getPrimSpec() const
{
    static const PvtPrimSpec s_primSpec;
    return s_primSpec;
}

const RtIdentifier& RtGeneric::getKind() const
{
    RtTypedValue* v = prim()->getAttribute(Identifiers::KIND, RtType::IDENTIFIER);
    return v ? v->getValue().asIdentifier() : Identifiers::UNKNOWN;
}

void RtGeneric::setKind(const RtIdentifier& kind)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::KIND, RtType::IDENTIFIER);
    attr->asIdentifier() = kind;
}

}
