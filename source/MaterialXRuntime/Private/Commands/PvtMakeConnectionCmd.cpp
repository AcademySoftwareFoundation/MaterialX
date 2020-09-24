//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtMakeConnectionCmd.h>
#include <MaterialXRuntime/RtPath.h>

namespace MaterialX
{

PvtCommandPtr PvtMakeConnectionCmd::create(const RtOutput& src, const RtInput& dest)
{
    return std::make_shared<PvtMakeConnectionCmd>(src, dest);
}

void PvtMakeConnectionCmd::execute(RtCommandResult& result)
{
    // Validate the connection
    if (!(_src.isValid() && _dest.isValid()))
    {
        result = RtCommandResult(false, string("Ports to connect are no longer valid"));
        return;
    }
    if (!_src.isConnectable(_dest))
    {
        result = RtCommandResult(false, "Output '" + _src.getPath().asString() + "' is not connectable to input '" + _dest.getPath().asString() + "'");
        return;
    }

    try
    {
        // Make the connection
        _src.connect(_dest);

        // Send message that the connection has been made.
        msg().sendMakeConnectionMessage(_src, _dest);

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void PvtMakeConnectionCmd::undo(RtCommandResult& result)
{
    if (_src.isValid() && _dest.isValid())
    {
        try
        {
            // Break the connection
            _src.disconnect(_dest);

            // Send message that the connection has been broken.
            msg().sendBreakConnectionMessage(_src, _dest);

            result = RtCommandResult(true);
        }
        catch (const ExceptionRuntimeError& e)
        {
            result = RtCommandResult(false, string(e.what()));
        }
    }
    else
    {
        result = RtCommandResult(false, string("Ports to disconnect are no longer valid"));
    }
}

}
