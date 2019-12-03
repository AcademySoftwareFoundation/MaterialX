//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtAttribute.h>

namespace MaterialX
{

RtAttribute::RtAttribute(const RtToken& name, const RtToken& type, RtObject parent, uint32_t flags) :
    _name(name),
    _type(type),
    _value(RtValue::createNew(type, parent)),
    _flags(flags)
{
}

string RtAttribute::getValueString() const
{
    string dest;
    RtValue::toString(getType(), _value, dest);
    return dest;
}

void RtAttribute::setValueString(const string& v)
{
    RtValue::fromString(getType(), v, _value);
}

}
