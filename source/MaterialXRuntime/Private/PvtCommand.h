//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTCOMMANDENGINE_H
#define MATERIALX_PVTCOMMANDENGINE_H


#include <MaterialXRuntime/RtCommand.h>

#include <MaterialXRuntime/Private/PvtMessage.h>

namespace MaterialX
{

// Base class for runtime commands.
class PvtCommand
{
public:
    virtual ~PvtCommand() {};

    PvtCommand(const PvtCommand&) = delete;
    PvtCommand& operator=(const PvtCommand&) = delete;

    // Execute the command.
    virtual void execute(RtCommandResult& result) = 0;

    // Undo the command.
    virtual void undo(RtCommandResult& result) = 0;

    // Redo the command.
    virtual void redo(RtCommandResult& result)
    {
        execute(result);
    }

protected:
    PvtCommand() {}

    // Utility for message handler access.
    PvtMessageHandler& msg();
};

// A shared pointer to a runtime command.
using PvtCommandPtr = RtSharedPtr<PvtCommand>;

class PvtCommandList : public PvtCommand
{
public:
    // Add a command to the batch.
    void addCommand(PvtCommandPtr cmd);

    // Clear all commands in the batch.
    void clearCommands();

    // Execute the command.
    void execute(RtCommandResult& result) override;

    // Undo the command.
    void undo(RtCommandResult& result)  override;

    // Redo the command.
    void redo(RtCommandResult& result)  override;

protected:
    vector<PvtCommandPtr> _commands;
};

class PvtCommandEngine
{
public:
    // Execute a new command.
    void execute(PvtCommandPtr cmd, RtCommandResult& result);

    // Undo the last previously executed command.
    void undo(RtCommandResult& result);

    // Redo the last previously executed undo command.
    void redo(RtCommandResult& result);

    // Flush the undo and redo queues.
    // All commands previously executed will no longer be undoable.
    void flushUndoQueue();

private:
    vector<PvtCommandPtr> _undoQueue;
    vector<PvtCommandPtr> _redoQueue;
};

}

#endif
