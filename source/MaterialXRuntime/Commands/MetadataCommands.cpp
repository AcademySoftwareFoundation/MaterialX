//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//
#include <MaterialXRuntime/Commands/MetadataCommands.h>

#include <MaterialXRuntime/Private/Commands/PvtSetMetadataCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtRemoveMetadataCmd.h>

#include <MaterialXRuntime/Private/PvtApi.h>

#include <MaterialXRuntime/RtTypeDef.h>

namespace MaterialX
{

namespace RtCommand
{

void setMetadata(const RtObject& src, const RtToken& metadataName, const string& metadataValue, RtCommandResult& result)
{
    try
    {
        RtValue v = RtValue::createNew(RtType::STRING, src);
        RtValue::fromString(RtType::STRING, metadataValue, v);
        PvtCommandPtr cmd = PvtSetMetadataCmd::create(src, metadataName, v);
        PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
    }
    catch (Exception& e)
    {
        result = RtCommandResult(false, e.what());
    }
}

void removeMetadata(const RtObject& src, const RtToken& metadataName, RtCommandResult& result)
{
    try
    {
        PvtCommandPtr cmd = PvtRemoveMetadataCmd::create(src, metadataName);
        PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
    }
    catch (Exception& e)
    {
        result = RtCommandResult(false, e.what());
    }
}

}

}
