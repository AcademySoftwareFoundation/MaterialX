//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTREMOVEATTRIBUTECMD_H
#define MATERIALX_PVTREMOVEATTRIBUTECMD_H

#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtValue.h>

namespace MaterialX
{

class PvtRemoveAttributeCmd : public PvtCommand
{
public:
    PvtRemoveAttributeCmd(const RtObject& obj, const RtToken& name);

    static PvtCommandPtr create(const RtObject& obj, const RtToken& name);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;
    void redo(RtCommandResult& result) override;

private:
    RtObject _obj;
    RtToken _name;
    RtValue _oldValue;
};

}

#endif
