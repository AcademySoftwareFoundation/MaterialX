//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPRIMCOMMANDS_H
#define MATERIALX_RTPRIMCOMMANDS_H

/// @file
/// Commands for prim handling.

#include <MaterialXRuntime/RtCommand.h>
#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtPrim.h>

namespace MaterialX
{

namespace RtCommand
{
    /// Create a new prim at the root of the stage.
    void createPrim(RtStagePtr stage, const RtIdentifier& typeName, RtCommandResult& result);

    /// Create a new prim at the given path.
    void createPrim(RtStagePtr stage, const RtIdentifier& typeName, const RtPath& path, RtCommandResult& result);

    /// Create a new prim inside the parent given by path.
    /// If an empty name is given a name will be generated.
    void createPrim(RtStagePtr stage, const RtIdentifier& typeName, const RtPath& parentPath, const RtIdentifier& name, RtCommandResult& result);

    /// Make a copy of the given prim and place the copy at the same parent path.
    void copyPrim(RtStagePtr stage, const RtPrim& prim, RtCommandResult& result);

    /// Make a copy of the given prim and place the copy at the given parent path.
    void copyPrim(RtStagePtr stage, const RtPrim& prim, const RtPath& parentPath, RtCommandResult& result);

    /// Remove a prim from a stage.
    void removePrim(RtStagePtr stage, const RtPath& path, RtCommandResult& result);

    /// Rename a prim.
    void renamePrim(RtStagePtr stage, const RtPath& path, const RtIdentifier& newName, RtCommandResult& result);

    /// Reparent a prim.
    void reparentPrim(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath, RtCommandResult& result);
}

}

#endif
