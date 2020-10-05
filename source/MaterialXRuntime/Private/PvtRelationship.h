//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTRELATIONSHIP_H
#define MATERIALX_PVTRELATIONSHIP_H

#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtAttribute.h>

#include <MaterialXRuntime/RtTraversal.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtRelationship : public PvtObject
{
    RT_DECLARE_RUNTIME_OBJECT(PvtRelationship)

public:
    PvtRelationship(const RtToken& name, PvtPrim* parent);

    bool hasTargets() const
    {
        return !_targets.empty();
    }

    size_t targetCount() const
    {
        return _targets.size();
    }

    void addTarget(const PvtObject* target);

    void removeTarget(const PvtObject* target);

    void clearTargets()
    {
        _targets.clear();
    }

    RtConnectionIterator getTargets() const
    {
        return RtConnectionIterator(this->obj());
    }

    const PvtDataHandleVec& getAllTargets() const
    {
        return _targets;
    }

protected:
    PvtDataHandleVec _targets;
    friend class RtConnectionIterator;
};

}

#endif
