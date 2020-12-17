//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtGeneric.h>
#include <MaterialXRuntime/Tokens.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

DEFINE_TYPED_SCHEMA(RtGeneric, "generic");

RtPrim RtGeneric::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name);

    static const RtToken DEFAULT_NAME("generic1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->addMetadata(Tokens::KIND, RtType::TOKEN);

    return primH;
}

const RtToken& RtGeneric::getKind() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::KIND, RtType::TOKEN);
    return v ? v->getValue().asToken() : Tokens::UNKNOWN;
}

void RtGeneric::setKind(const RtToken& kind)
{
    PvtPrim* p = prim();
    RtTypedValue* v = p->addMetadata(Tokens::KIND, RtType::TOKEN);
    v->getValue().asToken() = kind;
}

}
