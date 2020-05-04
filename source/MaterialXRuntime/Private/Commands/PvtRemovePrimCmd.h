//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTREMOVEPRIMCMD_H
#define MATERIALX_PVTREMOVEPRIMCMD_H

#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>

namespace MaterialX
{

class PvtRemovePrimCmd : public PvtCommandList
{
public:
    PvtRemovePrimCmd(RtStagePtr stage, const RtPath& path) :
        _stage(stage),
        _path(path)
    {}

    static PvtCommandPtr create(RtStagePtr stage, const RtPath& path);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtStagePtr _stage;
    const RtPath _path;
    RtPrim _prim;
};

}

#endif
