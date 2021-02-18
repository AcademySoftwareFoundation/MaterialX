//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTMETADATACOMMANDS_H
#define MATERIALX_RTMETADATACOMMANDS_H

/// @file
/// Commands for connection handling.

#include <MaterialXRuntime/RtCommand.h>
#include <MaterialXRuntime/RtToken.h>

namespace MaterialX
{

namespace RtCommand
{
    /// Set a metadata value from data given as a value string. Creates the metadata if it doesn't exist.
    void setMetadata(const RtObject& src, const RtToken& metadataName, const string& metadataValue, RtCommandResult& result);

    /// Removes the named metadata
    void removeMetadata(const RtObject& src, const RtToken& metadataName, RtCommandResult& result);
}

}

#endif
