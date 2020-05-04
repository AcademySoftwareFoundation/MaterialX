//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtRemovePrimCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtBreakConnectionCmd.h>

namespace MaterialX
{

PvtCommandPtr PvtRemovePrimCmd::create(RtStagePtr stage, const RtPath& path)
{
    return std::make_shared<PvtRemovePrimCmd>(stage, path);
}

void PvtRemovePrimCmd::execute(RtCommandResult& result)
{
    try
    {
        _prim = _stage->getPrimAtPath(_path);
        if (!_prim)
        {
            throw ExceptionRuntimeError("Can't find a prim with path '" + _path.asString() + "'");
        }

        // Clear any previous commands.
        clearCommands();

        // Collect commands for breaking all input connections
        RtObjTypePredicate<RtInput> inputFilter;
        for (RtAttribute attr : _prim.getAttributes(inputFilter))
        {
            RtInput input = attr.asA<RtInput>();
            if (input.isConnected())
            {
                RtOutput output = input.getConnection();
                addCommand(PvtBreakConnectionCmd::create(output, input));
            }
        }

        // Collect commands for breaking all output connections
        RtObjTypePredicate<RtOutput> outputFilter;
        for (RtAttribute attr : _prim.getAttributes(outputFilter))
        {
            RtOutput output = attr.asA<RtOutput>();
            if (output.isConnected())
            {
                for (RtObject inputObj : output.getConnections())
                {
                    RtInput input = inputObj.asA<RtInput>();
                    addCommand(PvtBreakConnectionCmd::create(output, input));
                }
            }
        }

        // Send message that the prim is about to be removed.
        msg().sendRemovePrimMessage(_stage, _prim);

        // Execute all the break connection commands.
        PvtCommandList::execute(result);

        // Dispose the prim.
        if (result.success())
        {
            _stage->disposePrim(_path);
        }

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void PvtRemovePrimCmd::undo(RtCommandResult& result)
{
    try
    {
        // Bring the prim back to life.
        RtPath parentPath(_path);
        parentPath.pop();
        _stage->restorePrim(parentPath, _prim);

        // Undo all the break connection commands.
        PvtCommandList::undo(result);
        clearCommands();

        // Send message that the prim has been created/restored.
        msg().sendCreatePrimMessage(_stage, _prim);

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

}
