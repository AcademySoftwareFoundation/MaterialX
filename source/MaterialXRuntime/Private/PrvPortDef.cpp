//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvPortDef.h>
#include <MaterialXRuntime/Private/PrvNodeDef.h>
#include <MaterialXRuntime/Private/PrvNodeGraph.h>

namespace MaterialX
{

const RtToken PrvPortDef::DEFAULT_OUTPUT_NAME("out");

PrvPortDef::PrvPortDef(const RtToken& name, const RtToken& type, uint32_t flags, PrvAllocatingElement* parent) :
PrvElement(RtObjType::PORTDEF, name),
_type(type),
_value(RtValue::createNew(type, parent->getObject())),
_colorspace(EMPTY_TOKEN),
_unit(EMPTY_TOKEN),
_flags(flags)
{
}

PrvObjectHandle PrvPortDef::createNew(PrvElement* parent, const RtToken& name, const RtToken& type, uint32_t flags)
{
    if (!(parent && (parent->hasApi(RtApiType::NODEDEF) || parent->hasApi(RtApiType::NODEGRAPH))))
    {
        throw ExceptionRuntimeError("PrvPortDef::createNew: Parent must be a nodedef or nodegraph");
    }

    PrvObjectHandle portdef(new PrvPortDef(name, type, flags, parent->asA<PrvAllocatingElement>()));

    if (parent->hasApi(RtApiType::NODEDEF))
    {
        parent->asA<PrvNodeDef>()->addPort(portdef);
    }
    else
    {
        parent->asA<PrvNodeGraph>()->addPort(portdef);
    }

    return portdef;
}

bool PrvPortDef::canConnectTo(const PrvPortDef* other) const
{
    // TODO: Check if the data types are compatible.
    return isConnectable() && other->isConnectable() && // both must be connectable
           isOutput() != other->isOutput();             // they must be of different kinds
}

}
