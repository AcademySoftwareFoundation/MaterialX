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

RtToken RtNodeDef::NODE("node");
RtToken RtNodeDef::NODEGROUP("nodegroup");
RtToken RtNodeDef::INHERIT("inherit");
RtToken RtNodeDef::TARGET("target");
RtToken RtNodeDef::VERSION("version");
RtToken RtNodeDef::IS_DEFAULT_VERSION("isdefaultversion");

DEFINE_TYPED_SCHEMA(RtNodeDef, "nodedef");

RtPrim RtNodeDef::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    if (typeName != _typeInfo.getShortTypeName())
    {
        throw ExceptionRuntimeError("Type names mismatch when creating prim '" + name.str() + "'");
    }

    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, name, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->addMetadata(NODE, RtType::TOKEN);

    return primH;
}

const RtToken& RtNodeDef::getNode() const
{
    RtTypedValue* v = prim()->getMetadata(NODE);
    return v ? v->getValue().asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setNode(const RtToken& node)
{
    RtTypedValue* v = prim()->addMetadata(NODE, RtType::TOKEN);
    v->getValue().asToken() = node;
}

const RtToken& RtNodeDef::getNodeGroup() const
{
    RtTypedValue* v = prim()->getMetadata(NODEGROUP);
    return v ? v->getValue().asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setNodeGroup(const RtToken& nodegroup)
{
    RtTypedValue* v = prim()->addMetadata(NODEGROUP, RtType::TOKEN);
    v->getValue().asToken() = nodegroup;
}

const RtToken& RtNodeDef::getTarget() const
{
    RtTypedValue* v = prim()->getMetadata(TARGET);
    return v ? v->getValue().asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setTarget(const RtToken& nodegroup)
{
    RtTypedValue* v = prim()->addMetadata(TARGET, RtType::TOKEN);
    v->getValue().asToken() = nodegroup;
}

const RtToken& RtNodeDef::getIneritance() const
{
    RtTypedValue* v = prim()->getMetadata(INHERIT);
    return v ? v->getValue().asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setIneritance(const RtToken& inherit)
{
    RtTypedValue* v = prim()->addMetadata(INHERIT, RtType::TOKEN);
    v->getValue().asToken() = inherit;
}

const RtToken& RtNodeDef::getVersion() const
{
    RtTypedValue* v = prim()->getMetadata(VERSION);
    return v ? v->getValue().asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setVersion(const RtToken& version)
{
    RtTypedValue* v = prim()->addMetadata(VERSION, RtType::TOKEN);
    v->getValue().asToken() = version;
}

bool RtNodeDef::getIsDefaultVersion() const
{
    RtTypedValue* v = prim()->addMetadata(IS_DEFAULT_VERSION, RtType::BOOLEAN);
    return v ? v->getValue().asBool() : false;
}

void RtNodeDef::setIsDefaultVersion(bool isDefault)
{
    RtTypedValue* v = prim()->addMetadata(IS_DEFAULT_VERSION, RtType::BOOLEAN);
    v->getValue().asBool() = isDefault;
}

bool RtNodeDef::isVersionCompatible(const RtToken& version) const
{
    // Test if either the version matches or if no version passed in if this is the default version.
    return ((version == getVersion()) ||
            (version.str().empty() && getIsDefaultVersion()));
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

size_t RtNodeDef::numInputs() const
{
    return prim()->numInputs();
}

RtInput RtNodeDef::getInput(const RtToken& name) const
{
    PvtInput* input = prim()->getInput(name);
    return input ? input->hnd() : RtInput();
}

RtAttrIterator RtNodeDef::getInputs() const
{
    RtObjTypePredicate<RtInput> filter;
    return RtAttrIterator(getPrim(), filter);
}

size_t RtNodeDef::numOutputs() const
{
    return prim()->numOutputs();
}

RtOutput RtNodeDef::getOutput(const RtToken& name) const
{
    PvtOutput* output = prim()->getOutput(name);
    return output ? output->hnd() : RtOutput();
}

RtOutput RtNodeDef::getOutput() const
{
    PvtOutput* output = prim()->getOutput();
    return output ? output->hnd() : RtOutput();
}

RtAttrIterator RtNodeDef::getOutputs() const
{
    RtObjTypePredicate<RtOutput> filter;
    return RtAttrIterator(getPrim(), filter);
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
