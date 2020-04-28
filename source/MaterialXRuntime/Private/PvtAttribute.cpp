//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtAttribute.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

const RtToken PvtAttribute::DEFAULT_OUTPUT_NAME("out");
const RtToken PvtAttribute::COLOR_SPACE("colorspace");
const RtToken PvtAttribute::UNIT("unit");
const RtToken PvtAttribute::UNIT_TYPE("unittype");

RT_DEFINE_RUNTIME_OBJECT(PvtAttribute, RtObjType::ATTRIBUTE, "PvtAttribute")

PvtAttribute::PvtAttribute(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent) :
    PvtObject(name, parent),
    _value(type, RtValue::createNew(type, parent->prim())),
    _flags(flags)
{
    setTypeBit<PvtAttribute>();
}


RT_DEFINE_RUNTIME_OBJECT(PvtOutput, RtObjType::OUTPUT, "PvtOutput")

PvtOutput::PvtOutput(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent) :
    PvtAttribute(name, type, flags, parent)
{
    setTypeBit<PvtOutput>();
}

bool PvtOutput::isConnectable(const PvtInput* other) const
{
    // TODO: Check if the data types are compatible.
    return _parent != other->_parent;
}

void PvtOutput::connect(PvtInput* input)
{
    if (input->isConnected())
    {
        throw ExceptionRuntimeError("Input is already connected");
    }
    if (!isConnectable(input))
    {
        throw ExceptionRuntimeError("Output is not connectable to this input");
    }
    _connections.push_back(input->hnd());
    input->_connection = hnd();
}

void PvtOutput::disconnect(PvtInput* input)
{
    if (!input->_connection || input->_connection.get() != this)
    {
        throw ExceptionRuntimeError("Given source and destination is not connected");
    }

    input->_connection = nullptr;
    for (auto it = _connections.begin(); it != _connections.end(); ++it)
    {
        if (it->get() == input)
        {
            _connections.erase(it);
            break;
        }
    }
}

void PvtOutput::clearConnections()
{
    // Break connections to all destination inputs.
    for (PvtDataHandle destH : _connections)
    {
        destH->asA<PvtInput>()->_connection = nullptr;
    }
    _connections.clear();
}


RT_DEFINE_RUNTIME_OBJECT(PvtInput, RtObjType::INPUT, "PvtInput")

PvtInput::PvtInput(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent) :
    PvtAttribute(name, type, flags, parent)
{
    setTypeBit<PvtInput>();
}

void PvtInput::clearConnection()
{
    if (_connection)
    {
        // Break connection to our single source output.
        PvtOutput* source = _connection->asA<PvtOutput>();
        for (auto it = source->_connections.begin(); it != source->_connections.end(); ++it)
        {
            if (it->get() == this)
            {
                source->_connections.erase(it);
                break;
            }
        }
        _connection = nullptr;
    }
}

}
