//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTRELATIONSHIPCMD_H
#define MATERIALX_PVTRELATIONSHIPCMD_H

#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtAttribute.h>

namespace MaterialX
{

class PvtRelationshipCmd : public PvtCommand
{
public:
    PvtRelationshipCmd(const RtRelationship& rel, const RtObject& target, ConnectionChange change) :
        _rel(rel),
        _target(target),
        _change(change)
    {}

    static PvtCommandPtr create(const RtRelationship& rel, const RtObject& target, ConnectionChange change);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    void makeConnection(RtCommandResult& result);
    void breakConnection(RtCommandResult& result);

    RtRelationship _rel;
    RtObject _target;
    ConnectionChange _change;
};

}

#endif
