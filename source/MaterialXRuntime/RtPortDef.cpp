//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtPortDef.h>

#include <MaterialXRuntime/Private/PrvPortDef.h>
#include <MaterialXRuntime/Private/PrvNodeDef.h>
#include <MaterialXRuntime/Private/PrvNodeGraph.h>

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
    return RtObject(PrvPortDef::createNew(parent.data()->asA<PrvElement>(), name, type, flags));
}

RtApiType RtPortDef::getApiType() const
{
    return RtApiType::PORTDEF;
}

const RtToken& RtPortDef::getType() const
{
    return data()->asA<PrvPortDef>()->getType();
}

const RtValue& RtPortDef::getValue() const
{
    return data()->asA<PrvPortDef>()->getValue();
}

RtValue& RtPortDef::getValue()
{
    return data()->asA<PrvPortDef>()->getValue();
}

void RtPortDef::setValue(const RtValue& v)
{
    return data()->asA<PrvPortDef>()->setValue(v);
}

const RtToken& RtPortDef::getColorSpace() const
{
    return data()->asA<PrvPortDef>()->getColorSpace();
}

void RtPortDef::setColorSpace(const RtToken& colorspace)
{
    return data()->asA<PrvPortDef>()->setColorSpace(colorspace);
}

const RtToken& RtPortDef::getUnit() const
{
    return data()->asA<PrvPortDef>()->getUnit();
}

void RtPortDef::setUnit(const RtToken& unit)
{
    return data()->asA<PrvPortDef>()->setUnit(unit);
}

int32_t RtPortDef::getFlags() const
{
    return data()->asA<PrvPortDef>()->getFlags();
}

bool RtPortDef::hasFlag(uint32_t flag) const
{
    return data()->asA<PrvPortDef>()->hasFlag(flag);
}

bool RtPortDef::isInput() const
{
    return data()->asA<PrvPortDef>()->isInput();
}

bool RtPortDef::isOutput() const
{
    return data()->asA<PrvPortDef>()->isOutput();
}

bool RtPortDef::isConnectable() const
{
    return data()->asA<PrvPortDef>()->isConnectable();
}

bool RtPortDef::isUniform() const
{
    return data()->asA<PrvPortDef>()->isUniform();
}

}
