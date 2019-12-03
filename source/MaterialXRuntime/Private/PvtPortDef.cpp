//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtPortDef.h>
#include <MaterialXRuntime/Private/PvtNodeDef.h>
#include <MaterialXRuntime/Private/PvtNodeGraph.h>

namespace MaterialX
{

const RtToken PvtPortDef::DEFAULT_OUTPUT_NAME("out");

PvtPortDef::PvtPortDef(const RtToken& name, const RtToken& type, uint32_t flags, PvtAllocatingElement* parent) :
PvtElement(RtObjType::PORTDEF, name),
_type(type),
_value(RtValue::createNew(type, parent->getObject())),
_colorspace(EMPTY_TOKEN),
_unit(EMPTY_TOKEN),
_flags(flags)
{
}

PvtObjectHandle PvtPortDef::createNew(PvtElement* parent, const RtToken& name, const RtToken& type, uint32_t flags)
{
    if (!(parent && (parent->hasApi(RtApiType::NODEDEF) || parent->hasApi(RtApiType::NODEGRAPH))))
    {
        throw ExceptionRuntimeError("PvtPortDef::createNew: Parent must be a nodedef or nodegraph");
    }

    PvtObjectHandle portdef(new PvtPortDef(name, type, flags, parent->asA<PvtAllocatingElement>()));

    if (parent->hasApi(RtApiType::NODEDEF))
    {
        parent->asA<PvtNodeDef>()->addPort(portdef);
    }
    else
    {
        parent->asA<PvtNodeGraph>()->addPort(portdef);
    }

    return portdef;
}

bool PvtPortDef::canConnectTo(const PvtPortDef* other) const
{
    // TODO: Check if the data types are compatible.
    return isConnectable() && other->isConnectable() && // both must be connectable
           isOutput() != other->isOutput();             // they must be of different kinds
}

}
