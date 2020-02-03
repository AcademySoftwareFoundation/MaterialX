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

RtInput RtNodeDef::createInput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    return prim()->createInput(name, type, flags)->hnd();
}

void RtNodeDef::removeInput(const RtToken& name)
{
    PvtInput* input = prim()->getInput(name);
    if (!(input && input->isA<PvtInput>()))
    {
        throw ExceptionRuntimeError("No input found with name '" + name.str() + "'");
    }
    prim()->removeAttribute(name);
}

RtOutput RtNodeDef::createOutput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    return prim()->createOutput(name, type, flags)->hnd();
}

void RtNodeDef::removeOutput(const RtToken& name)
{
    PvtOutput* output = prim()->getOutput(name);
    if (!(output && output->isA<PvtOutput>()))
    {
        throw ExceptionRuntimeError("No output found with name '" + name.str() + "'");
    }
    prim()->removeAttribute(name);
}

RtInput RtNodeDef::getInput(const RtToken& name) const
{
    PvtInput* input = prim()->getInput(name);
    return input ? input->hnd() : RtInput();
}

RtOutput RtNodeDef::getOutput(const RtToken& name) const
{
    PvtOutput* output = prim()->getOutput(name);
    return output ? output->hnd() : RtOutput();
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
