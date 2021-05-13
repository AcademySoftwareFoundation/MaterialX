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
#include <MaterialXRuntime/RtString.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtPort : public PvtObject
{
    RT_DECLARE_RUNTIME_OBJECT(PvtPort)

public:
    const RtString& getType() const
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

    bool isUniform() const
    {
        return (_flags & RtPortFlag::UNIFORM) != 0;
    }

    bool isToken() const
    {
        return (_flags & RtPortFlag::TOKEN) != 0;
    }

    const RtString& getColorSpace() const
    {
        const RtTypedValue* attr = getAttribute(RtString::COLORSPACE, RtType::INTERNSTRING);
        return attr ? attr->asInternString() : RtString::EMPTY;
    }

    void setColorSpace(const RtString& colorspace)
    {
        RtTypedValue* attr = createAttribute(RtString::COLORSPACE, RtType::INTERNSTRING);
        attr->asInternString() = colorspace;
    }

protected:
    PvtPort(const RtString& name, const RtString& type, uint32_t flags, PvtPrim* parent);

    RtTypedValue _value;
    uint32_t _flags;
};


class PvtOutput;

class PvtInput : public PvtPort
{
    RT_DECLARE_RUNTIME_OBJECT(PvtInput)

public:
    PvtInput(const RtString& name, const RtString& type, uint32_t flags, PvtPrim* parent);

    bool isUIVisible() const
    {
        // For now since connections require ports to be created and hence visible
        // always assume a connection means visible.
        if (isConnected())
        {
            return true;
        }
        const RtTypedValue* attr = getAttribute(RtString::UIVISIBLE, RtType::BOOLEAN);
        return attr ? attr->asBool() : true;
    }

    void setIsUIVisible(bool val)
    {
        RtTypedValue* attr = createAttribute(RtString::UIVISIBLE, RtType::BOOLEAN);
        attr->asBool() = val;
    }

    const RtString& getUnit() const
    {
        const RtTypedValue* attr = getAttribute(RtString::UNIT, RtType::INTERNSTRING);
        return attr ? attr->asInternString() : RtString::EMPTY;
    }

    void setUnit(const RtString& unit)
    {
        RtTypedValue* attr = createAttribute(RtString::UNIT, RtType::INTERNSTRING);
        attr->asInternString() = unit;
    }

    const RtString& getUnitType() const
    {
        const RtTypedValue* attr = getAttribute(RtString::UNITTYPE, RtType::INTERNSTRING);
        return attr ? attr->asInternString() : RtString::EMPTY;
    }

    void setUnitType(const RtString& unit)
    {
        RtTypedValue* attr = createAttribute(RtString::UNITTYPE, RtType::INTERNSTRING);
        attr->asInternString() = unit;
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
    PvtOutput(const RtString& name, const RtString& type, uint32_t flags, PvtPrim* parent);

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
