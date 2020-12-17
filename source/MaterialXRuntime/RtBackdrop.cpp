//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtBackdrop.h>
#include <MaterialXRuntime/Tokens.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

DEFINE_TYPED_SCHEMA(RtBackdrop, "backdrop");

RtPrim RtBackdrop::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name);

    static const RtToken DEFAULT_NAME("backdrop1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Tokens::CONTAINS);
    prim->createAttribute(Tokens::WIDTH, RtType::FLOAT);
    prim->createAttribute(Tokens::HEIGHT, RtType::FLOAT);
    prim->createAttribute(Tokens::NOTE, RtType::STRING);

    return primH;
}

RtRelationship RtBackdrop::getContains() const
{
    return prim()->getRelationship(Tokens::CONTAINS)->hnd();
}

RtAttribute RtBackdrop::getNote() const
{
    return prim()->getAttribute(Tokens::NOTE)->hnd();
}

RtAttribute RtBackdrop::getWidth() const
{
    return prim()->getAttribute(Tokens::WIDTH)->hnd();
}

RtAttribute RtBackdrop::getHeight() const
{
    return prim()->getAttribute(Tokens::HEIGHT)->hnd();
}

}
