//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTBREAKCONNECTIONCMD_H
#define MATERIALX_PVTBREAKCONNECTIONCMD_H

#include <MaterialXRuntime/Private/PvtCommandEngine.h>

#include <MaterialXRuntime/RtAttribute.h>

namespace MaterialX
{

class PvtBreakConnectionCmd : public PvtCommand
{
public:
    PvtBreakConnectionCmd(const RtOutput& src, const RtInput& dest) :
        _src(src),
        _dest(dest)
    {}

    static PvtCommandPtr create(const RtOutput& src, const RtInput& dest);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtOutput _src;
    RtInput _dest;
};

}

#endif
