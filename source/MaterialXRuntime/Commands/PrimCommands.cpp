//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Commands/PrimCommands.h>
#include <MaterialXRuntime/Commands/PortCommands.h>
#include <MaterialXRuntime/RtApi.h>

#include <MaterialXRuntime/Private/PvtApi.h>
#include <MaterialXRuntime/Private/Commands/PvtCreatePrimCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtCopyPrimCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtRemovePrimCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtRenamePrimCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtReparentPrimCmd.h>

namespace MaterialX
{

namespace RtCommand
{

void createPrim(RtStagePtr stage, const RtString& typeName, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtCreatePrimCmd::create(stage, typeName, RtPath("/"), RtString::EMPTY);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void createPrim(RtStagePtr stage, const RtString& typeName, const RtPath& path, RtCommandResult& result)
{
    RtPath parentPath(path);
    parentPath.pop();
    PvtCommandPtr cmd = PvtCreatePrimCmd::create(stage, typeName, parentPath, path.getName());
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void createPrim(RtStagePtr stage, const RtString& typeName, const RtPath& parentPath, const RtString& name, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtCreatePrimCmd::create(stage, typeName, parentPath, name);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void copyPrim(RtStagePtr stage, const RtPrim& prim, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtCopyPrimCmd::create(stage, prim, prim.getParent().getPath());
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void copyPrim(RtStagePtr stage, const RtPrim& prim, const RtPath& parentPath, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtCopyPrimCmd::create(stage, prim, parentPath);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void removePrim(RtStagePtr stage, const RtPath& path, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtRemovePrimCmd::create(stage, path);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void renamePrim(RtStagePtr stage, const RtPath& path, const RtString& newName, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtRenamePrimCmd::create(stage, path, newName);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void reparentPrim(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtReparentPrimCmd::create(stage, path, newParentPath);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

}

}
