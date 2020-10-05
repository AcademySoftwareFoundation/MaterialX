//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Commands/AttributeCommands.h>
#include <MaterialXRuntime/RtApi.h>

#include <MaterialXRuntime/Private/PvtApi.h>
#include <MaterialXRuntime/Private/PvtCommand.h>
#include <MaterialXRuntime/Private/Commands/PvtRelationshipCmd.h>

namespace MaterialX
{

namespace RtCommand
{

void makeRelationship(const RtRelationship& rel, const RtObject& target, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtRelationshipCmd::create(rel, target, ConnectionChange::MAKE_CONNECTION);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void breakRelationship(const RtRelationship& rel, const RtObject& target, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtRelationshipCmd::create(rel, target, ConnectionChange::BREAK_CONNECTION);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

} // RtCommands

} // MaterialX
