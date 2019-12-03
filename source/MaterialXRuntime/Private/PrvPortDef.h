//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPORTDEFDATA_H
#define MATERIALX_RTPORTDEFDATA_H

#include <MaterialXRuntime/Private/PrvElement.h>

#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtPortDef.h>
#include <MaterialXRuntime/RtValue.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PrvPortDef : public PrvElement
{
public:
    static PrvObjectHandle createNew(PrvElement* parent, const RtToken& name, const RtToken& type, uint32_t flags = 0);

    const RtToken& getType() const
    {
        return _type;
    }

    const RtValue& getValue() const
    {
        return _value;
    }

    RtValue& getValue()
    {
        return _value;
    }

    void setValue(const RtValue& v)
    {
        _value = v;
    }

    string getValueString() const
    {
        string dest;
        RtValue::toString(getType(), _value, dest);
        return dest;
    }

    const RtToken& getColorSpace() const
    {
        return _colorspace;
    }

    void setColorSpace(const RtToken& colorspace)
    {
        _colorspace = colorspace;
    }

    const RtToken& getUnit() const
    {
        return _unit;
    }

    void setUnit(const RtToken& unit)
    {
        _unit = unit;
    }

    uint32_t getFlags() const
    {
        return _flags;
    }

    bool hasFlag(uint32_t flag) const
    {
        return (_flags & flag) != 0;
    }

    bool isInput() const
    {
        return !isOutput();
    }

    bool isOutput() const
    {
        return (_flags & RtPortFlag::OUTPUT) != 0;
    }

    bool isConnectable() const
    {
        return !(_flags & RtPortFlag::UNCONNECTABLE);
    }

    bool isUniform() const
    {
        return (_flags & RtPortFlag::UNIFORM) != 0;
    }

    bool canConnectTo(const PrvPortDef* dest) const;

    static const RtToken DEFAULT_OUTPUT_NAME;

protected:
    PrvPortDef(const RtToken& name, const RtToken& type, uint32_t flags, PrvAllocatingElement* parent);

    RtToken _type;
    RtValue _value;
    RtToken _colorspace;
    RtToken _unit;
    uint32_t _flags;
};

}

#endif
