//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtMakeConnectionCmd.h>

namespace MaterialX
{

PvtCommandPtr PvtMakeConnectionCmd::create(const RtOutput& src, const RtInput& dest)
{
    return std::make_shared<PvtMakeConnectionCmd>(src, dest);
}

void PvtMakeConnectionCmd::execute(RtCommandResult& result)
{
    if (_src.isValid() && _dest.isValid())
    {
        try
        {
            //
            // TODO: Do validation on the connections
            //

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
    else
    {
        result = RtCommandResult(false, string("Ports to connect are no longer valid"));
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
