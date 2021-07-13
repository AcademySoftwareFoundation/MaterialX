//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtConnectionCmd.h>
#include <MaterialXRuntime/RtPath.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNodeGraph.h>

namespace MaterialX
{

PvtCommandPtr PvtConnectionCmd::create(const RtOutput& src, const RtInput& dest, ConnectionChange change)
{
    return std::make_shared<PvtConnectionCmd>(src, dest, change);
}

void PvtConnectionCmd::execute(RtCommandResult& result)
{
    if (_change == ConnectionChange::MAKE_CONNECTION)
    {
        makeConnection(result);
    }
    else
    {
        breakConnection(result);
    }
}

void PvtConnectionCmd::undo(RtCommandResult& result)
{
    if (_change == ConnectionChange::MAKE_CONNECTION)
    {
        breakConnection(result);
    }
    else
    {
        makeConnection(result);
    }
}

void PvtConnectionCmd::makeConnection(RtCommandResult& result)
{
    try
    {
        // Validate the operation.
        // Note: Connectability is validated inside the connect method below,
        // so here we only need to check that the operands are valid.
        if (!(_src && _dest))
        {
            result = RtCommandResult(false, string("PvtConnectionCmd: Command operands are no longer valid"));
            return;
        }
        
        // Make the connection
        _dest.connect(_src);

        // Update any required connection properties
        updateConnectionProperties(_src, _dest, result);

        // Send message that the connection has been made.
        msg().sendConnectionMessage(_src, _dest, ConnectionChange::MAKE_CONNECTION);

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void PvtConnectionCmd::breakConnection(RtCommandResult& result)
{
    try
    {
        // Validate the operation
        if (!(_src && _dest))
        {
            result = RtCommandResult(false, string("PvtConnectionCmd: Command operands are no longer valid"));
            return;
        }

        // Break the connection
        _dest.disconnect(_src);

        // Send message that the connection has been broken.
        msg().sendConnectionMessage(_src, _dest, ConnectionChange::BREAK_CONNECTION);

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void PvtConnectionCmd::updateConnectionProperties(const RtOutput& /*src*/, const RtInput& /*dest*/, RtCommandResult& /*result*/) 
{
    // No-op
}


PvtCommandPtr PvtInterfaceConnectionCmd::create(const RtOutput& src, const RtInput& dest, ConnectionChange change)
{
    return std::make_shared<PvtInterfaceConnectionCmd>(src, dest, change);
}

void PvtInterfaceConnectionCmd::updateConnectionProperties(const RtOutput& src, const RtInput& dest, RtCommandResult& /*result*/)
{
    // Copy token information over
    if (dest.isToken())
    {
        RtPrim parent = src.getParent();
        if (parent.hasApi<RtNodeGraph>())
        {
            RtNodeGraph graph(parent);
            RtInput socketInput = graph.getInput(src.getName());
            if (socketInput)
            {
                socketInput.setIsToken(true);
            }
        }
    }
}

}
