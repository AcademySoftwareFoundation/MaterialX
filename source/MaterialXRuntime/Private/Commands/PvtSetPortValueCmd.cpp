//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtSetPortValueCmd.h>

#include <MaterialXRuntime/RtPrim.h>

namespace MaterialX
{

PvtSetPortValueCmd::PvtSetPortValueCmd(const RtPort& port, const RtValue& value) :
    _port(port),
    _value(RtValue::clone(port.getType(), value, port.getParent()))
{
}

PvtCommandPtr PvtSetPortValueCmd::create(const RtPort& port, const RtValue& value)
{
    return std::make_shared<PvtSetPortValueCmd>(port, value);
}

void PvtSetPortValueCmd::execute(RtCommandResult& result)
{
    if (_port.isValid())
    {
        try
        {
            // Send message that the port value is changing
            msg().sendSetPortValueMessage(_port, _value);

            // Save old value for undo/redo
            _oldValue = RtValue::clone(_port.getType(), _port.getValue(), _port.getParent());

            // Set the value
            _port.setValue(_value);
            result = RtCommandResult(true);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Port to set value on is no longer valid"));
    }
}

void PvtSetPortValueCmd::undo(RtCommandResult& result)
{
    if (_port.isValid())
    {
        try
        {
            // Send message that the port value is changing
            msg().sendSetPortValueMessage(_port, _oldValue);

            // Reset the value
            _port.setValue(_oldValue);
            result = RtCommandResult(true);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Port to set value on is no longer valid"));
    }
}

void PvtSetPortValueCmd::redo(RtCommandResult& result)
{
    if (_port.isValid())
    {
        try
        {
            // Send message that the port value is changing
            msg().sendSetPortValueMessage(_port, _value);

            // Set the value
            _port.setValue(_value);
            result = RtCommandResult(true);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Port to set value on is no longer valid"));
    }
}

}
