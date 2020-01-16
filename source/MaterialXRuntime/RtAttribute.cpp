//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtAttribute.h>

#include <MaterialXRuntime/Private/PvtAttribute.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

RtAttribute::RtAttribute(const RtObject& obj) : 
    RtPathItem(obj)
{
}

RtApiType RtAttribute::getApiType() const
{
    return RtApiType::ATTRIBUTE;
}

const RtToken& RtAttribute::getType() const
{
    return hnd()->asA<PvtAttribute>()->getType();
}

const RtValue& RtAttribute::getValue() const
{
    return hnd()->asA<PvtAttribute>()->getValue();
}

RtValue& RtAttribute::getValue()
{
    return hnd()->asA<PvtAttribute>()->getValue();
}

void RtAttribute::setValue(const RtValue& v)
{
    return hnd()->asA<PvtAttribute>()->setValue(v);
}

string RtAttribute::getValueString() const
{
    return hnd()->asA<PvtAttribute>()->getValueString();
}

void RtAttribute::setValueString(const string& v)
{
    hnd()->asA<PvtAttribute>()->setValueString(v);
}

const RtToken& RtAttribute::getColorSpace() const
{
    return hnd()->asA<PvtAttribute>()->getColorSpace();
}

void RtAttribute::setColorSpace(const RtToken& colorspace)
{
    return hnd()->asA<PvtAttribute>()->setColorSpace(colorspace);
}

const RtToken& RtAttribute::getUnit() const
{
    return hnd()->asA<PvtAttribute>()->getUnit();
}

void RtAttribute::setUnit(const RtToken& unit)
{
    return hnd()->asA<PvtAttribute>()->setUnit(unit);
}

bool RtAttribute::isInput() const
{
    return hnd()->asA<PvtAttribute>()->isInput();
}

bool RtAttribute::isOutput() const
{
    return hnd()->asA<PvtAttribute>()->isOutput();
}

bool RtAttribute::isConnectable() const
{
    return hnd()->asA<PvtAttribute>()->isConnectable();
}

bool RtAttribute::isConnectable(const RtAttribute& other) const
{
    return hnd()->asA<PvtAttribute>()->isConnectable(other.hnd()->asA<PvtAttribute>());
}

}
