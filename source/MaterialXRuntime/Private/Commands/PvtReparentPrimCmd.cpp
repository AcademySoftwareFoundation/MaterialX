//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtReparentPrimCmd.h>

namespace MaterialX
{

PvtCommandPtr PvtReparentPrimCmd::create(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath)
{
    return std::make_shared<PvtReparentPrimCmd>(stage, path, newParentPath);
}

void PvtReparentPrimCmd::execute(RtCommandResult& result)
{
    try
    {
        RtToken resultName = _stage->reparentPrim(_path, _parentPath);

        // Update paths so we can undo later
        RtPath newPath = _parentPath;
        newPath.push(resultName);
        _parentPath = _path;
        _parentPath.pop();
        _path = newPath;

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void PvtReparentPrimCmd::undo(RtCommandResult& result)
{
    try
    {
        RtToken resultName = _stage->reparentPrim(_path, _parentPath);

        // Make sure we get back to the original name
        // in case the reparenting forced the prim name to change.
        if (resultName != _originalName)
        {
            RtPath newPath = _parentPath;
            newPath.push(resultName);
            _stage->renamePrim(newPath, _originalName);
            resultName = _originalName;
        }

        // Update paths so we can redo later
        RtPath newPath = _parentPath;
        newPath.push(resultName);
        _parentPath = _path;
        _parentPath.pop();
        _path = newPath;

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

}
