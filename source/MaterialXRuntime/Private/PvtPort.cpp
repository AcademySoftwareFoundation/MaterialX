//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtPort.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPath.h>

#include <MaterialXRuntime/RtConnectableApi.h>

namespace MaterialX
{

const RtToken PvtPort::DEFAULT_OUTPUT_NAME("out");
const RtToken PvtPort::COLOR_SPACE("colorspace");
const RtToken PvtPort::UNIT("unit");
const RtToken PvtPort::UNIT_TYPE("unittype");


RT_DEFINE_RUNTIME_OBJECT(PvtPort, RtObjType::PORT, "PvtPort")

PvtPort::PvtPort(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent) :
    PvtObject(name, parent),
    _value(type, RtValue::createNew(type, parent->prim())),
    _flags(flags)
{
    setTypeBit<PvtPort>();
}


RT_DEFINE_RUNTIME_OBJECT(PvtInput, RtObjType::INPUT, "PvtInput")

PvtInput::PvtInput(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent) :
    PvtPort(name, type, flags, parent)
{
    setTypeBit<PvtInput>();
}

bool PvtInput::isConnectable(const PvtOutput* output) const
{
    // We cannot connect to ourselves.
    if (_parent == output->_parent)
    {
        return false;
    }

    // Use the connectable API of this prim to validate the connection.
    RtConnectableApi* connectableApi = RtConnectableApi::get(_parent->prim());
    return connectableApi && connectableApi->acceptConnection(hnd(), output->hnd());
}

void PvtInput::connect(PvtOutput* output)
{
    // Check if this connection exists already.
    if (_connection == output->hnd())
    {
        return;
    }

    // Check if another connection exists already.
    if (isConnected())
    {
        throw ExceptionRuntimeError("'" + getPath().asString() + "' is already connected");
    }

    // Make sure the connection is valid.
    if (!isConnectable(output))
    {
        throw ExceptionRuntimeError("'" + getPath().asString() + "' rejected the connection");
    }

    // Make the connection.
    _connection = output->hnd();
    output->_connections.push_back(this);
}

void PvtInput::disconnect(PvtOutput* output)
{
    // Make sure the connection exists, otherwise we can't break it.
    if (_connection != output->hnd())
    {
        return;
    }

    // Break the connection.
    _connection = nullptr;
    for (auto it = output->_connections.begin(); it != output->_connections.end(); ++it)
    {
        if (*it == this)
        {
            output->_connections.erase(it);
            break;
        }
    }
}

void PvtInput::clearConnection()
{
    if (_connection)
    {
        // Break connection to our single source output.
        PvtOutput* source = _connection->asA<PvtOutput>();
        for (auto it = source->_connections.begin(); it != source->_connections.end(); ++it)
        {
            if (*it == this)
            {
                source->_connections.erase(it);
                break;
            }
        }
        _connection = nullptr;
    }
}


RT_DEFINE_RUNTIME_OBJECT(PvtOutput, RtObjType::OUTPUT, "PvtOutput")

PvtOutput::PvtOutput(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent) :
    PvtPort(name, type, flags, parent)
{
    setTypeBit<PvtOutput>();
}

void PvtOutput::clearConnections()
{
    // Break connections to all destination inputs.
    for (PvtObject* dest : _connections)
    {
        dest->asA<PvtInput>()->_connection = nullptr;
    }
    _connections.clear();
}

}
