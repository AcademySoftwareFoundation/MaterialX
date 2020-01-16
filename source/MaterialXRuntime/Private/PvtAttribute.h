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

using PvtConnectionData = vector<PvtDataHandle>;

class PvtAttribute : public PvtPathItem
{
public:
    static const RtObjType typeId;
    static const RtToken typeName;

public:
    PvtAttribute(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent);

    RtObjType getObjType() const override
    {
        return typeId;
    }

    const RtToken& getObjTypeName() const override
    {
        return typeName;
    }

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

    bool isInput() const
    {
        return !isOutput();
    }

    bool isOutput() const
    {
        return (_flags & RtAttrFlag::OUTPUT) != 0;
    }

    bool isSocket() const
    {
        return (_flags & RtAttrFlag::SOCKET) != 0;
    }

    bool isConnectable() const
    {
        return (_flags & RtAttrFlag::CONNECTABLE) != 0;
    }

    bool isConnectable(const PvtAttribute* source) const;

    bool isUniform() const
    {
        return (_flags & RtAttrFlag::UNIFORM) != 0;
    }

    bool isConnected() const
    {
        return !_connections.empty();
    }

    static void connect(PvtAttribute* source, PvtAttribute* dest);

    static void disconnect(PvtAttribute* source, PvtAttribute* dest);

    void connectToSource(PvtAttribute* source)
    {
        connect(source, this);
    }

    void disconnectSource(PvtAttribute* source)
    {
        disconnect(source, this);
    }

    void connectToDestination(PvtAttribute* dest)
    {
        connect(this, dest);
    }

    void disconnectDestination(PvtAttribute* dest)
    {
        disconnect(this, dest);
    }

    void clearConnections();

    PvtAttribute* getConnection() const
    {
        return _connections.size() ? _connections[0]->asA<PvtAttribute>() : nullptr;
    }

    RtConnectionIterator getConnections() const
    {
        return RtConnectionIterator(this->obj());
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

    static const RtToken DEFAULT_OUTPUT_NAME;
    static const RtToken COLOR_SPACE;
    static const RtToken UNIT;
    static const RtToken ATTRIBUTE;

protected:
    RtTypedValue _value;
    PvtConnectionData _connections;
    uint32_t _flags;

    friend class RtConnectionIterator;
};

}

#endif
