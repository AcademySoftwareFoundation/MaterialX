//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTCREATEPRIMCMD_H
#define MATERIALX_PVTCREATEPRIMCMD_H

#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>

namespace MaterialX
{

class PvtCreatePrimCmd : public PvtCommand
{
public:
    PvtCreatePrimCmd(RtStagePtr stage, const RtString& typeName, const RtPath& parentPath, const RtString& name) :
        _stage(stage),
        _typeName(typeName),
        _parentPath(parentPath),
        _name(name)
    {}

    static PvtCommandPtr create(RtStagePtr stage, const RtString& typeName, const RtPath& parentPath, const RtString& name);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;
    void redo(RtCommandResult& result) override;

private:
    RtStagePtr _stage;
    const RtString _typeName;
    const RtPath _parentPath;
    RtString _name;
    RtPrim _prim;
};

}

#endif
