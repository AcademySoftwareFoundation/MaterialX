//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtPortDef.h>

#include <MaterialXRuntime/Private/PvtPortDef.h>
#include <MaterialXRuntime/Private/PvtNodeDef.h>
#include <MaterialXRuntime/Private/PvtNodeGraph.h>

namespace MaterialX
{

RtPortDef::RtPortDef(const RtObject& obj) : 
    RtElement(obj)
{
}

RtObject RtPortDef::createNew(RtObject parent, const RtToken& name, const RtToken& type, uint32_t flags)
{
    if (!(parent.hasApi(RtApiType::NODEDEF) || parent.hasApi(RtApiType::NODEGRAPH)))
    {
        throw ExceptionRuntimeError("Parent object must be a nodedef or a nodegraph");
    }
    PvtDataHandle data = PvtPortDef::createNew(PvtObject::ptr<PvtElement>(parent), name, type, flags);
    return PvtObject::object(data);
}

RtApiType RtPortDef::getApiType() const
{
    return RtApiType::PORTDEF;
}

const RtToken& RtPortDef::getType() const
{
    return data()->asA<PvtPortDef>()->getType();
}

const RtValue& RtPortDef::getValue() const
{
    return data()->asA<PvtPortDef>()->getValue();
}

RtValue& RtPortDef::getValue()
{
    return data()->asA<PvtPortDef>()->getValue();
}

void RtPortDef::setValue(const RtValue& v)
{
    return data()->asA<PvtPortDef>()->setValue(v);
}

const RtToken& RtPortDef::getColorSpace() const
{
    return data()->asA<PvtPortDef>()->getColorSpace();
}

void RtPortDef::setColorSpace(const RtToken& colorspace)
{
    return data()->asA<PvtPortDef>()->setColorSpace(colorspace);
}

const RtToken& RtPortDef::getUnit() const
{
    return data()->asA<PvtPortDef>()->getUnit();
}

void RtPortDef::setUnit(const RtToken& unit)
{
    return data()->asA<PvtPortDef>()->setUnit(unit);
}

int32_t RtPortDef::getFlags() const
{
    return data()->asA<PvtPortDef>()->getFlags();
}

bool RtPortDef::hasFlag(uint32_t flag) const
{
    return data()->asA<PvtPortDef>()->hasFlag(flag);
}

bool RtPortDef::isInput() const
{
    return data()->asA<PvtPortDef>()->isInput();
}

bool RtPortDef::isOutput() const
{
    return data()->asA<PvtPortDef>()->isOutput();
}

bool RtPortDef::isConnectable() const
{
    return data()->asA<PvtPortDef>()->isConnectable();
}

bool RtPortDef::isUniform() const
{
    return data()->asA<PvtPortDef>()->isUniform();
}

}
