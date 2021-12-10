//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTCOPYPRIMCMD_H
#define MATERIALX_PVTCOPYPRIMCMD_H

#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>

MATERIALX_NAMESPACE_BEGIN


class PvtCopyPrimCmd : public PvtCommand
{
public:
    PvtCopyPrimCmd(RtStagePtr stage, const RtPrim& prim, const RtPath& parentPath) :
        _stage(stage),
        _prim(prim),
        _parentPath(parentPath)
    {}

    static PvtCommandPtr create(RtStagePtr stage, const RtPrim& prim, const RtPath& parentPath);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;
    void redo(RtCommandResult& result) override;

private:
    RtPrim createPrimCopy(const RtPrim& prim, const RtPath& parentPath);
    void copyMetadata(const PvtObject* src, PvtObject* dest);

    RtStagePtr _stage;
    const RtPrim _prim;
    const RtPath _parentPath;
    RtPrim _copy;
};

MATERIALX_NAMESPACE_END

#endif
