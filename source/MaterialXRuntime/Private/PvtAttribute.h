//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTATTRIBUTE_H
#define MATERIALX_PVTATTRIBUTE_H

#include <MaterialXRuntime/Private/PvtObject.h>

#include <MaterialXRuntime/RtAttribute.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtTraversal.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtAttribute : public PvtObject
{
    RT_DECLARE_RUNTIME_OBJECT(PvtAttribute)

public:
    PvtAttribute(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent);

    const RtToken& getType() const
    {
        return _value.getType();
    }

    const RtValue& getValue() const
    {
        return _value.getValue();
    }

    RtValue& getValue()
    {
        return _value.getValue();
    }

    void setValue(const RtValue& v)
    {
        _value.getValue() = v;
    }

    string getValueString() const
    {
        return _value.getValueString();
    }

    void setValueString(const string& v)
    {
        return _value.setValueString(v);
    }

    uint32_t getFlags() const
    {
        return _flags;
    }

    bool hasFlag(uint32_t flag) const
    {
        return (_flags & flag) != 0;
    }

    bool isSocket() const
    {
        return (_flags & RtAttrFlag::SOCKET) != 0;
    }

    const RtToken& getColorSpace() const
    {
        const RtTypedValue* md = getMetadata(PvtAttribute::COLOR_SPACE);
        return md ? md->getValue().asToken() : EMPTY_TOKEN;
    }

    void setColorSpace(const RtToken& colorspace)
    {
        RtTypedValue* md = getMetadata(PvtAttribute::COLOR_SPACE);
        if (!md)
        {
            md = addMetadata(PvtAttribute::COLOR_SPACE, RtType::TOKEN);
        }
        md->getValue().asToken() = colorspace;
    }

    const RtToken& getUnit() const
    {
        const RtTypedValue* md = getMetadata(PvtAttribute::UNIT);
        return md ? md->getValue().asToken() : EMPTY_TOKEN;
    }

    void setUnit(const RtToken& unit)
    {
        RtTypedValue* md = getMetadata(PvtAttribute::UNIT);
        if (!md)
        {
            md = addMetadata(PvtAttribute::UNIT, RtType::TOKEN);
        }
        md->getValue().asToken() = unit;
    }

    const RtToken& getUnitType() const
    {
        const RtTypedValue* md = getMetadata(PvtAttribute::UNIT_TYPE);
        return md ? md->getValue().asToken() : EMPTY_TOKEN;
    }

    void setUnitType(const RtToken& unit)
    {
        RtTypedValue* md = getMetadata(PvtAttribute::UNIT_TYPE);
        if (!md)
        {
            md = addMetadata(PvtAttribute::UNIT_TYPE, RtType::TOKEN);
        }
        md->getValue().asToken() = unit;
    }

    static const RtToken DEFAULT_OUTPUT_NAME;
    static const RtToken COLOR_SPACE;
    static const RtToken UNIT;
    static const RtToken UNIT_TYPE;
    static const RtToken ATTRIBUTE;

protected:
    RtTypedValue _value;
    uint32_t _flags;

    friend class RtConnectionIterator;
};


class PvtOutput;

class PvtInput : public PvtAttribute
{
    RT_DECLARE_RUNTIME_OBJECT(PvtInput)

public:
    PvtInput(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent);

    bool isUniform() const
    {
        return (_flags & RtAttrFlag::UNIFORM) != 0;
    }

    void setUniform(bool uniform)
    {
        if (uniform)
        {
            _flags |= RtAttrFlag::UNIFORM;
        }
        else
        {
            _flags &= ~RtAttrFlag::UNIFORM;
        }
    }

    bool isConnected() const
    {
        return _connection != nullptr;
    }

    PvtOutput* getConnection() const
    {
        return _connection ? _connection->asA<PvtOutput>() : nullptr;
    }

    bool isConnectable(const PvtOutput* output) const;

    void connect(PvtOutput* output);

    void disconnect(PvtOutput* output);

    void clearConnection();

protected:
    PvtDataHandle _connection;
    friend class PvtOutput;
};


class PvtOutput : public PvtAttribute
{
    RT_DECLARE_RUNTIME_OBJECT(PvtOutput)

public:
    PvtOutput(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent);

    bool isConnected() const
    {
        return !_connections.empty();
    }

    bool isConnectable(const PvtInput* input) const
    {
        return input->isConnectable(this);
    }

    void connect(PvtInput* input)
    {
        return input->connect(this);
    }

    void disconnect(PvtInput* input)
    {
        return input->disconnect(this);
    }

    RtConnectionIterator getConnections() const
    {
        return RtConnectionIterator(this->obj());
    }

    void clearConnections();

protected:
    PvtDataHandleVec _connections;
    friend class PvtInput;
    friend class RtConnectionIterator;
};



}

#endif
