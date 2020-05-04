//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Commands/AttributeCommands.h>
#include <MaterialXRuntime/RtApi.h>

#include <MaterialXRuntime/Private/PvtApi.h>
#include <MaterialXRuntime/Private/PvtCommand.h>
#include <MaterialXRuntime/Private/Commands/PvtSetAttributeCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtMakeConnectionCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtBreakConnectionCmd.h>

namespace MaterialX
{

namespace RtCommand
{

void setAttribute(const RtAttribute& attr, const RtValue& value, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, value);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, bool value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, int value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, float value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Color2& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Color3& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Color4& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Vector2& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Vector3& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Vector4& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const RtToken& value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, void* value, RtCommandResult& result)
{
    RtValue v(value);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Matrix33& value, RtCommandResult& result)
{
    RtPrim prim(attr.getParent());
    RtValue v(value, prim);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const Matrix44& value, RtCommandResult& result)
{
    RtPrim prim(attr.getParent());
    RtValue v(value, prim);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void setAttribute(const RtAttribute& attr, const string& value, RtCommandResult& result)
{
    RtPrim prim(attr.getParent());
    RtValue v(value, prim);
    PvtCommandPtr cmd = PvtSetAttributeCmd::create(attr, v);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void makeConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtMakeConnectionCmd::create(src, dest);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void breakConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtBreakConnectionCmd::create(src, dest);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

} // RtCommands

} // MaterialX
