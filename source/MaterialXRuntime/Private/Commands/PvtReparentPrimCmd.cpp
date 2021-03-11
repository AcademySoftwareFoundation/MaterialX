//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtReparentPrimCmd.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXRuntime/RtPrim.h>

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
        // Validate that the prim exists.
        RtPrim prim = _stage->getPrimAtPath(_path);
        if (!prim)
        {
            result = RtCommandResult(false, "Given path '" + _path.asString() + " does not point to a prim in this stage");
            return;
        }

        // Validate that the new parent exists.
        RtPrim newParent = _stage->getPrimAtPath(_newParentPath);
        if (!newParent)
        {
            result = RtCommandResult(false, "Given path '" + _newParentPath.asString() + " does not point to a prim in this stage");
            return;
        }

        // Construct the new path making sure the prim name is according to
        // any rename that might happen during the reparent operation.
        PvtObjHandle newParentH = PvtObject::hnd(newParent);
        PvtPrim* newParentPtr = newParentH->asA<PvtPrim>();
        const RtToken newName = newParentPtr->makeUniqueChildName(prim.getName());
        RtPath newPath = _newParentPath;
        newPath.push(newName);

        // Send message that the prim is about to be reparented.
        msg().sendReparentPrimMessage(_stage, prim, newPath);

        // Do the reparenting operation.
        RtToken resultName = _stage->reparentPrim(_path, _newParentPath);
        if (resultName != newName)
        {
            result = RtCommandResult(false, "Reparent gave inconsistent naming. Expected new name '" + newName.str() + "' but got new name '" + resultName.str() + "'");
            return;
        }

        // Update paths so we can undo later.
        _newParentPath = _path;
        _newParentPath.pop();
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
        // Validate that the prim exists.
        RtPrim prim = _stage->getPrimAtPath(_path);
        if (!prim)
        {
            result = RtCommandResult(false, "Given path '" + _path.asString() + " does not point to a prim in this stage");
            return;
        }

        // Validate that the new parent exists.
        RtPrim newParent = _stage->getPrimAtPath(_newParentPath);
        if (!newParent)
        {
            result = RtCommandResult(false, "Given path '" + _newParentPath.asString() + " does not point to a prim in this stage");
            return;
        }

        RtPath newPath = _newParentPath;
        newPath.push(_originalName);

        // Send message that the prim is about to be reparented
        // back to the original path.
        msg().sendReparentPrimMessage(_stage, prim, newPath);

        RtToken resultName = _stage->reparentPrim(_path, _newParentPath);

        // Make sure we get back to the original name
        // in case the reparenting forced the prim name to change.
        if (resultName != _originalName)
        {
            RtPath p = _newParentPath;
            p.push(resultName);
            _stage->renamePrim(p, _originalName);
        }

        // Update paths so we can redo later
        _newParentPath = _path;
        _newParentPath.pop();
        _path = newPath;

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

}
