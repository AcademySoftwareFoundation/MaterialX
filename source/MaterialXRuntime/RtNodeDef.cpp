//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtApi.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    static const RtToken NODE("node");
}

DEFINE_TYPED_SCHEMA(RtNodeDef, "nodedef");

RtPrim RtNodeDef::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeName)
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }

    PvtDataHandle primH = PvtPrim::createNew(name, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->setTypeName(_typeName);
    prim->addMetadata(NODE, RtType::TOKEN);

    return primH;
}

const RtToken& RtNodeDef::getNode() const
{
    RtTypedValue* v = prim()->getMetadata(NODE);
    return v->getValue().asToken();
}

void RtNodeDef::setNode(const RtToken& node)
{
    RtTypedValue* v = prim()->getMetadata(NODE);
    v->getValue().asToken() = node;
}

void RtNodeDef::registerMasterPrim() const
{
    RtApi::get().registerMasterPrim(prim()->hnd());
}

void RtNodeDef::unregisterMasterPrim() const
{
    RtApi::get().unregisterMasterPrim(prim()->getName());
}

bool RtNodeDef::isMasterPrim() const
{
    return RtApi::get().hasMasterPrim(prim()->getName());
}

}
