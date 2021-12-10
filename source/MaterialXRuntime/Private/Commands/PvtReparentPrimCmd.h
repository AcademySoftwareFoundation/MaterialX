//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTREPARENTPRIMCMD_H
#define MATERIALX_PVTREPARENTPRIMCMD_H

#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtPath.h>

MATERIALX_NAMESPACE_BEGIN


class PvtReparentPrimCmd : public PvtCommand
{
public:
    PvtReparentPrimCmd(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath) :
        _stage(stage),
        _path(path),
        _newParentPath(newParentPath),
        _originalName(path.getName())
    {}

    static PvtCommandPtr create(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtStagePtr _stage;
    RtPath _path;
    RtPath _newParentPath;
    RtString _originalName;
};

MATERIALX_NAMESPACE_END

#endif
