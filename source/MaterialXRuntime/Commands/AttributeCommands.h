//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTATTRIBUTECOMMANDS_H
#define MATERIALX_RTATTRIBUTECOMMANDS_H

/// @file
/// Commands for attribute handling.

#include <MaterialXRuntime/RtCommand.h>
#include <MaterialXRuntime/RtToken.h>

namespace MaterialX
{

namespace RtCommand
{
    /// Set an attribute value from data given as a value string. Creates the attribute if it doesn't exist.
    void setAttributeFromString(const RtObject& obj, const RtToken& name, const string& value, RtCommandResult& result);

    /// Remove an attribute.
    void removeAttribute(const RtObject& obj, const RtToken& name, RtCommandResult& result);
}

}

#endif
