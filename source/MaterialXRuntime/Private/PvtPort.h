//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTPORT_H
#define MATERIALX_PVTPORT_H

#include <MaterialXRuntime/Private/PvtObject.h>

#include <MaterialXRuntime/RtPort.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtTraversal.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtPort : public PvtObject
{
    RT_DECLARE_RUNTIME_OBJECT(PvtPort)

public:
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
        return (_flags & RtPortFlag::SOCKET) != 0;
    }

    const RtToken& getColorSpace() const
    {
        const RtTypedValue* attr = getAttribute(PvtPort::COLOR_SPACE, RtType::TOKEN);
        return attr ? attr->asToken() : EMPTY_TOKEN;
    }

    void setColorSpace(const RtToken& colorspace)
    {
        RtTypedValue* attr = createAttribute(PvtPort::COLOR_SPACE, RtType::TOKEN);
        attr->asToken() = colorspace;
    }

    const RtToken& getUnit() const
    {
        const RtTypedValue* attr = getAttribute(PvtPort::UNIT, RtType::TOKEN);
        return attr ? attr->asToken() : EMPTY_TOKEN;
    }

    void setUnit(const RtToken& unit)
    {
        RtTypedValue* attr = createAttribute(PvtPort::UNIT, RtType::TOKEN);
        attr->asToken() = unit;
    }

    const RtToken& getUnitType() const
    {
        const RtTypedValue* attr = getAttribute(PvtPort::UNIT_TYPE, RtType::TOKEN);
        return attr ? attr->asToken() : EMPTY_TOKEN;
    }

    void setUnitType(const RtToken& unit)
    {
        RtTypedValue* attr = createAttribute(PvtPort::UNIT_TYPE, RtType::TOKEN);
        attr->asToken() = unit;
    }

    static const RtToken DEFAULT_OUTPUT_NAME;
    static const RtToken COLOR_SPACE;
    static const RtToken UNIT;
    static const RtToken UNIT_TYPE;
    static const RtToken ATTRIBUTE;

protected:
    PvtPort(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent);

    RtTypedValue _value;
    uint32_t _flags;

    friend class RtConnectionIterator;
};


class PvtOutput;

class PvtInput : public PvtPort
{
    RT_DECLARE_RUNTIME_OBJECT(PvtInput)

public:
    PvtInput(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent);

    bool isUniform() const
    {
        return (_flags & RtPortFlag::UNIFORM) != 0;
    }

    void setUniform(bool uniform)
    {
        if (uniform)
        {
            _flags |= RtPortFlag::UNIFORM;
        }
        else
        {
            _flags &= ~RtPortFlag::UNIFORM;
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
    PvtObjHandle _connection;
    friend class PvtOutput;
};


class PvtOutput : public PvtPort
{
    RT_DECLARE_RUNTIME_OBJECT(PvtOutput)

public:
    PvtOutput(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent);

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

    bool isConnected() const
    {
        return !_connections.empty();
    }

    size_t numConnections() const
    {
        return !_connections.size();
    }

    RtInput getConnection(size_t index) const
    {
        return _connections[index]->hnd();
    }

    RtConnectionIterator getConnections() const
    {
        return RtConnectionIterator(this->obj());
    }

    void clearConnections();

protected:
    PvtObjHandleVec _connections;

    friend class PvtInput;
    friend class RtInputIterator;
    friend class RtConnectionIterator;  
};

}

#endif
