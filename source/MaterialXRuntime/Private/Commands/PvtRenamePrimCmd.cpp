//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtRenamePrimCmd.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXRuntime/RtPrim.h>

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
        // Validate that the prim exists.
        RtPrim prim = _stage->getPrimAtPath(_path);
        if (!prim)
        {
            result = RtCommandResult(false, "Given path '" + _path.asString() + " does not point to a prim in this stage");
            return;
        }

        // Make sure the new name is a unique child name for the parent.
        // Otherwise the name will be updated during rename in order to be
        // unique, and the message we send will not have correct information.
        RtPrim parent = prim.getParent();
        PvtObjHandle parentH = PvtObject::hnd(parent);
        PvtPrim* parentPtr = parentH->asA<PvtPrim>();
        _newName = parentPtr->makeUniqueChildName(_newName);

        // Send message that the prim is about to be renamed.
        msg().sendRenamePrimMessage(_stage, prim, _newName);

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
