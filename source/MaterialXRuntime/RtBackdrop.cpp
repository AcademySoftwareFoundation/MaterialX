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
    static const RtToken CONTAINS("contains");
    static const RtToken WIDTH("width");
    static const RtToken HEIGHT("height");
    static const RtToken NOTE("note");
    static const RtToken KIND("kind");
    static const RtToken BACKDROP1("backdrop1");
    static const RtToken GENERIC1("generic1");

}

DEFINE_TYPED_SCHEMA(RtBackdrop, "backdrop");

RtPrim RtBackdrop::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeName)
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }

    const RtToken primName = name == EMPTY_TOKEN ? BACKDROP1 : name;
    PvtDataHandle primH = PvtPrim::createNew(primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->setTypeName(_typeName);
    prim->createRelationship(CONTAINS);
    prim->createAttribute(WIDTH, RtType::FLOAT);
    prim->createAttribute(HEIGHT, RtType::FLOAT);
    prim->createAttribute(NOTE, RtType::STRING);

    return primH;
}

RtRelationship RtBackdrop::contains() const
{
    return prim()->getRelationship(CONTAINS)->hnd();
}

RtAttribute RtBackdrop::note() const
{
    return prim()->getAttribute(NOTE)->hnd();
}

RtAttribute RtBackdrop::width() const
{
    return prim()->getAttribute(WIDTH)->hnd();
}

RtAttribute RtBackdrop::height() const
{
    return prim()->getAttribute(HEIGHT)->hnd();
}



DEFINE_TYPED_SCHEMA(RtGeneric, "generic");

RtPrim RtGeneric::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeName)
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }

    const RtToken primName = name == EMPTY_TOKEN ? GENERIC1 : name;
    PvtDataHandle primH = PvtPrim::createNew(primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->setTypeName(_typeName);
    prim->addMetadata(KIND, RtType::TOKEN);

    return primH;
}

const RtToken& RtGeneric::getKind() const
{
    RtTypedValue* v = prim()->getMetadata(KIND);
    return v->getValue().asToken();
}

void RtGeneric::setKind(const RtToken& kind) const
{
    RtTypedValue* v = prim()->getMetadata(KIND);
    v->getValue().asToken() = kind;
}

}
