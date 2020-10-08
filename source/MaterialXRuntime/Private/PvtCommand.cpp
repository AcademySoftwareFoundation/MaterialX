//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtCommand.h>
#include <MaterialXRuntime/Private/PvtApi.h>

namespace MaterialX
{

PvtMessageHandler& PvtCommand::msg()
{
    return PvtApi::cast(RtApi::get())->getMessageHandler();
}

void PvtCommandList::addCommand(PvtCommandPtr cmd)
{
    _commands.push_back(cmd);
}

void PvtCommandList::clearCommands()
{
    _commands.clear();
}

void PvtCommandList::execute(RtCommandResult& result)
{
    // Initialize to success in case the list is empty.
    result = RtCommandResult(true);

    // Execute all commands.
    for (PvtCommandPtr cmd : _commands)
    {
        cmd->execute(result);
        if (!result.success())
        {
            break;
        }
    }
}

void PvtCommandList::undo(RtCommandResult& result)
{
    // Initialize to success in case the list is empty.
    result = RtCommandResult(true);

    // Undo all commands.
    for (PvtCommandPtr cmd : _commands)
    {
        cmd->undo(result);
        if (!result.success())
        {
            break;
        }
    }
}

void PvtCommandList::redo(RtCommandResult& result)
{
    // Initialize to success in case the list is empty.
    result = RtCommandResult(true);

    // Redo all commands.
    for (PvtCommandPtr cmd : _commands)
    {
        cmd->redo(result);
        if (!result.success())
        {
            break;
        }
    }
}


void PvtCommandEngine::execute(PvtCommandPtr cmd, RtCommandResult& result)
{
    cmd->execute(result);

    if (result.success())
    {
        _undoQueue.push_back(cmd);
        _redoQueue.clear();
    }
}

void PvtCommandEngine::undo(RtCommandResult& result)
{
    if (!_undoQueue.empty())
    {
        PvtCommandPtr cmd = _undoQueue.back();
        _undoQueue.pop_back();

        cmd->undo(result);

        if (result.success())
        {
            _redoQueue.push_back(cmd);
        }
    }
    else
    {
        result = RtCommandResult(false, string("No command to undo"));
    }
}

void PvtCommandEngine::redo(RtCommandResult& result)
{
    if (!_redoQueue.empty())
    {
        PvtCommandPtr cmd = _redoQueue.back();
        _redoQueue.pop_back();

        cmd->redo(result);

        if (result.success())
        {
            _undoQueue.push_back(cmd);
        }
    }
    else
    {
        result = RtCommandResult(false, string("No command to redo"));
    }
}

void PvtCommandEngine::flushUndoQueue()
{
    _undoQueue.clear();
    _redoQueue.clear();
}

}
