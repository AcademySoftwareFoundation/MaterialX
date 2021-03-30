//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPORTCOMMANDS_H
#define MATERIALX_RTPORTCOMMANDS_H

/// @file
/// Commands for setting values and connections on ports.

#include <MaterialXRuntime/RtCommand.h>
#include <MaterialXRuntime/RtPort.h>

namespace MaterialX
{
namespace RtCommand
{
    /// Set a port value from data given as a value string.
    void setPortValueFromString(const RtPort& port, const string& valueString, RtCommandResult& result);

    /// Set value on a bool port.
    void setPortValue(const RtPort& port, bool value, RtCommandResult& result);

    /// Set value on a integer port.
    void setPortValue(const RtPort& port, int value, RtCommandResult& result);

    /// Set value on a float port.
    void setPortValue(const RtPort& port, float value, RtCommandResult& result);

    /// Set value on a color3 port.
    void setPortValue(const RtPort& port, const Color3& value, RtCommandResult& result);

    /// Set value on a color4 port.
    void setPortValue(const RtPort& port, const Color4& value, RtCommandResult& result);

    /// Set value on a vector2 port.
    void setPortValue(const RtPort& port, const Vector2& value, RtCommandResult& result);

    /// Set value on a vector3 port.
    void setPortValue(const RtPort& port, const Vector3& value, RtCommandResult& result);

    /// Set value on a vector4 port.
    void setPortValue(const RtPort& port, const Vector4& value, RtCommandResult& result);

    /// Set value on a string port.
    void setPortValue(const RtPort& port, const RtIdentifier& value, RtCommandResult& result);

    /// Set value on a pointer port.
    void setPortValue(const RtPort& port, void* value, RtCommandResult& result);

    /// Set value on a matrix33 port.
    void setPortValue(const RtPort& port, const Matrix33& value, RtCommandResult& result);

    /// Set value on a matrix44 port.
    void setPortValue(const RtPort& port, const Matrix44& value, RtCommandResult& result);

    /// Set value on a string port.
    void setPortValue(const RtPort& port, const string& value, RtCommandResult& result);

    /// Make a connection from src output to dest input.
    void makeConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result);

    /// Break a connection from src output to dest input.
    void breakConnection(const RtOutput& src, const RtInput& dest, RtCommandResult& result);

} // RtCommand
} // MAterialX

#endif
