//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTCOMMAND_H
#define MATERIALX_RTCOMMAND_H

/// @file
/// Classes related to command execution.

#include <MaterialXRuntime/RtObject.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

/// @class RtCommandResult
/// Class holding the results of a command execution.
class RtCommandResult
{
public:
    /// Empty constructor.
    RtCommandResult() :
        _success(false)
    {
    }

    /// Construct a command result from a boolean state and an optional error message.
    RtCommandResult(bool success, const string& message = EMPTY_STRING) :
        _success(success),
        _message(message)
    {
    }

    /// Construct a command result from a return object and optionally
    /// a boolean state and an error message.
    RtCommandResult(const RtObject& object, bool success = true, const string& message = EMPTY_STRING) :
        _success(success),
        _message(message),
        _object(object)
    {
    }

    /// Return true if the command executed succesfully.
    bool success() const
    {
        return _success;
    }

    /// Return true if the command executed succesfully.
    explicit operator bool() const
    {
        return success();
    }

    /// Return an error message if set when executing the command.
    const string& getMessage() const
    {
        return _message;
    }

    /// Return an object resulting from executing the command.
    const RtObject& getObject() const
    {
        return _object;
    }

private:
    bool _success;
    string _message;
    RtObject _object;
};

}

#endif
