//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTSETPORTVALUECMD_H
#define MATERIALX_PVTSETPORTVALUECMD_H

#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtPort.h>
#include <MaterialXRuntime/RtValue.h>

namespace MaterialX
{

class PvtSetPortValueCmd : public PvtCommand
{
public:
    PvtSetPortValueCmd(const RtPort& port, const RtValue& value);

    static PvtCommandPtr create(const RtPort& port, const RtValue& value);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;
    void redo(RtCommandResult& result) override;

private:
    RtPort _port;
    RtValue _value;
    RtValue _oldValue;
};

}

#endif
