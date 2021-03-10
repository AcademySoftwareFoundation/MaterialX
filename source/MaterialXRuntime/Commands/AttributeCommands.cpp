//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//
#include <MaterialXRuntime/Commands/AttributeCommands.h>

#include <MaterialXRuntime/Private/Commands/PvtSetAttributeCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtRemoveAttributeCmd.h>

#include <MaterialXRuntime/Private/PvtApi.h>

#include <MaterialXRuntime/RtTypeDef.h>

namespace MaterialX
{

namespace RtCommand
{

void setAttributeFromString(const RtObject& obj, const RtToken& name, const string& value, RtCommandResult& result)
{
    // Use try/catch since the conversion from string might fail and throw.
    try
    {
        RtValue v = RtValue::createNew(RtType::STRING, obj);
        RtValue::fromString(RtType::STRING, value, v);
        PvtCommandPtr cmd = PvtSetAttributeCmd::create(obj, name, v);
        PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
    }
    catch (Exception& e)
    {
        result = RtCommandResult(false, e.what());
    }
}

void removeAttribute(const RtObject& obj, const RtToken& name, RtCommandResult& result)
{
    try
    {
        PvtCommandPtr cmd = PvtRemoveAttributeCmd::create(obj, name);
        PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
    }
    catch (Exception& e)
    {
        result = RtCommandResult(false, e.what());
    }
}

}

}
