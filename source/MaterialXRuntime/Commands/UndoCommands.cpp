//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Commands/UndoCommands.h>
#include <MaterialXRuntime/RtApi.h>

#include <MaterialXRuntime/Private/PvtApi.h>

namespace MaterialX
{

namespace RtCommand
{

void undo(RtCommandResult& result)
{
    PvtApi::cast(RtApi::get())->getCommandEngine().undo(result);
}

void redo(RtCommandResult& result)
{
    PvtApi::cast(RtApi::get())->getCommandEngine().redo(result);
}

void flushUndoQueue()
{
    PvtApi::cast(RtApi::get())->getCommandEngine().flushUndoQueue();
}

}

}
