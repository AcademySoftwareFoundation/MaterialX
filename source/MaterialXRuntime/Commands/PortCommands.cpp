//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Commands/PortCommands.h>
#include <MaterialXRuntime/RtApi.h>

#include <MaterialXRuntime/Private/PvtApi.h>
#include <MaterialXRuntime/Private/PvtCommand.h>
#include <MaterialXRuntime/Private/Commands/PvtSetPortValueCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtConnectionCmd.h>

namespace MaterialX
{
namespace RtCommand
{

void setPortValueFromString(const RtPort& port, const string& valueString, RtCommandResult& result)
{
    // Use try/catch since the conversion from string might fail and throw.
    try
    {
        RtValue v = RtValue::createNew(port.getType(), port.getParent());
        RtValue::fromString(port.getType(), valueString, v);
        PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
        PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
    }
    catch (Exception& e)
    {
        result = RtCommandResult(false, e.what());
    }
}

void setPortValue(const RtPort& port, bool value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setPortValue(const RtPort& port, int value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setPortValue(const RtPort& port, float value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setPortValue(const RtPort& port, const Color3& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setPortValue(const RtPort& port, const Color4& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setPortValue(const RtPort& port, const Vector2& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setPortValue(const RtPort& port, const Vector3& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setPortValue(const RtPort& port, const Vector4& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setPortValue(const RtPort& port, const RtString& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setPortValue(const RtPort& port, void* value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setPortValue(const RtPort& port, const Matrix33& value, RtCommandResult& result)
{
    RtPrim prim(port.getParent());
    RtValue v(value, prim);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setPortValue(const RtPort& port, const Matrix44& value, RtCommandResult& result)
{
    RtPrim prim(port.getParent());
    RtValue v(value, prim);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setPortValue(const RtPort& port, const string& value, RtCommandResult& result)
{
    RtPrim prim(port.getParent());
    RtValue v(value, prim);
    PvtCommandPtr cmd = PvtSetPortValueCmd::create(port, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void makeConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtConnectionCmd::create(src, dest, ConnectionChange::MAKE_CONNECTION);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void breakConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtConnectionCmd::create(src, dest, ConnectionChange::BREAK_CONNECTION);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

} // RtCommands
} // MaterialX
