//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtRenamePrimCmd.h>

namespace MaterialX
{

PvtCommandPtr PvtRenamePrimCmd::create(RtStagePtr stage, const RtPath& path, const RtToken& newName)
{
    return std::make_shared<PvtRenamePrimCmd>(stage, path, newName);
}

void PvtRenamePrimCmd::execute(RtCommandResult& result)
{
    try
    {
        RtToken oldName = _path.getName();
        RtToken resultName = _stage->renamePrim(_path, _newName);

        // Update the path and name so we can undo later
        _path.pop();
        _path.push(resultName);
        _newName = oldName;

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void PvtRenamePrimCmd::undo(RtCommandResult& result)
{
    try
    {
        // Path and name was updated after execution
        // so we can just re-execute the command.
        execute(result);

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

}
