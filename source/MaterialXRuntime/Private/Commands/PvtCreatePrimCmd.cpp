//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtCreatePrimCmd.h>

namespace MaterialX
{

PvtCommandPtr PvtCreatePrimCmd::create(RtStagePtr stage, const RtToken& typeName, const RtPath& parentPath, const RtToken& name)
{
    return std::make_shared<PvtCreatePrimCmd>(stage, typeName, parentPath, name);
}

void PvtCreatePrimCmd::execute(RtCommandResult& result)
{
    try
    {
        _prim = _stage->createPrim(_parentPath, _name, _typeName);
        
        // Update the name if it was changed so we can undo later.
        _name = _prim.getName();

        result = RtCommandResult(_prim.asA<RtObject>());
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void PvtCreatePrimCmd::undo(RtCommandResult& result)
{
    try
    {
        RtPath path(_parentPath);
        path.push(_name);
        _stage->disposePrim(path);
        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void PvtCreatePrimCmd::redo(RtCommandResult& result)
{
    try
    {
        _stage->restorePrim(_parentPath, _prim);
        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

}
