//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTSETMETADATACMD_H
#define MATERIALX_PVTSETMETADATACMD_H

#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtValue.h>

namespace MaterialX
{

class PvtSetMetadataCmd : public PvtCommand
{
public:
    PvtSetMetadataCmd(const RtObject& obj, const RtToken& name, const RtValue& value);

    static PvtCommandPtr create(const RtObject& obj, const RtToken& name, const RtValue& value);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;
    void redo(RtCommandResult& result) override;

private:
    RtObject _obj;
    RtToken _name;
    RtValue _value;
    RtValue _oldValue;
    bool _metadataCreated;
};

}

#endif
