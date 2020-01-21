//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTRELATIONSHIP_H
#define MATERIALX_PVTRELATIONSHIP_H

#include <MaterialXRuntime/Private/PvtPathItem.h>
#include <MaterialXRuntime/Private/PvtAttribute.h>

#include <MaterialXRuntime/RtTraversal.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtRelationship : public PvtPathItem
{
public:
    static const RtObjType typeId;
    static const RtToken typeName;

public:
    PvtRelationship(const RtToken& name, PvtPrim* parent);

    RtObjType getObjType() const override
    {
        return typeId;
    }

    const RtToken& getObjTypeName() const override
    {
        return typeName;
    }

    bool hasTargets() const
    {
        return !_targets.empty();
    }

    void addTarget(const PvtPathItem* target)
    {
        _targets.push_back(target->hnd());
    }

    void removeTarget(const PvtPathItem* target);

    void clearTargets()
    {
        _targets.clear();
    }

    RtConnectionIterator getTargets() const
    {
        return RtConnectionIterator(this->obj());
    }

protected:
    PvtDataHandleVec _targets;
 
    friend class RtConnectionIterator;
};

}

#endif
