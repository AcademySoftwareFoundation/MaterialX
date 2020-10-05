//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/Commands/PvtRelationshipCmd.h>

namespace MaterialX
{

PvtCommandPtr PvtRelationshipCmd::create(const RtRelationship& rel, const RtObject& target, ConnectionChange change)
{
    return std::make_shared<PvtRelationshipCmd>(rel, target, change);
}

void PvtRelationshipCmd::execute(RtCommandResult& result)
{
    if (_change == ConnectionChange::MAKE_CONNECTION)
    {
        makeConnection(result);
    }
    else
    {
        breakConnection(result);
    }
}

void PvtRelationshipCmd::undo(RtCommandResult& result)
{
    if (_change == ConnectionChange::MAKE_CONNECTION)
    {
        breakConnection(result);
    }
    else
    {
        makeConnection(result);
    }
}

void PvtRelationshipCmd::makeConnection(RtCommandResult& result)
{
    try
    {
        // Validate the operation
        if (!(_rel && _target))
        {
            result = RtCommandResult(false, string("PvtRelationshipCmd: Command operands are no longer valid"));
            return;
        }

        // Make the change
        _rel.addTarget(_target);

        // Send message that the relationship has changes.
        msg().sendRelationshipMessage(_rel, _target, ConnectionChange::MAKE_CONNECTION);

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void PvtRelationshipCmd::breakConnection(RtCommandResult& result)
{
    try
    {
        // Validate the operation
        if (!(_rel && _target))
        {
            result = RtCommandResult(false, string("PvtRelationshipCmd: Command operands are no longer valid"));
            return;
        }

        // Make the change
        _rel.removeTarget(_target);

        // Send message that the relationship has changes.
        msg().sendRelationshipMessage(_rel, _target, ConnectionChange::BREAK_CONNECTION);

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

}
