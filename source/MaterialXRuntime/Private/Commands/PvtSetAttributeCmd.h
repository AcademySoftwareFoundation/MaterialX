//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTSETATTRIBUTECMD_H
#define MATERIALX_PVTSETATTRIBUTECMD_H

#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtAttribute.h>
#include <MaterialXRuntime/RtValue.h>

namespace MaterialX
{

class PvtSetAttributeCmd : public PvtCommand
{
public:
    PvtSetAttributeCmd(const RtAttribute& attr, const RtValue& value);

    static PvtCommandPtr create(const RtAttribute& attr, const RtValue& value);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;
    void redo(RtCommandResult& result) override;

private:
    RtAttribute _attr;
    RtValue _value;
    RtValue _oldValue;
};

}

#endif
