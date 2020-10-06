//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtGeneric.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    static const RtToken KIND("kind");
    static const RtToken GENERIC1("generic1");
    static const RtToken UNKNOWN("unknown");
}

DEFINE_TYPED_SCHEMA(RtGeneric, "generic");

RtPrim RtGeneric::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeInfo.getShortTypeName())
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }

    const RtToken primName = name == EMPTY_TOKEN ? GENERIC1 : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->addMetadata(KIND, RtType::TOKEN);

    return primH;
}

const RtToken& RtGeneric::getKind() const
{
    RtTypedValue* v = prim()->getMetadata(KIND);
    return v && v->getType() == RtType::TOKEN ? v->getValue().asToken() : UNKNOWN;
}

void RtGeneric::setKind(const RtToken& kind)
{
    PvtPrim* p = prim();
    RtTypedValue* v = p->getMetadata(KIND);
    if (!v)
    {
        v = p->addMetadata(KIND, RtType::TOKEN);
    }
    else if (v->getType() != RtType::TOKEN)
    {
        p->removeMetadata(KIND);
        v = p->addMetadata(KIND, RtType::TOKEN);
    }
    v->getValue().asToken() = kind;
}

}
