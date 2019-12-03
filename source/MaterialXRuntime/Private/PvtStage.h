//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTSTAGE_H
#define MATERIALX_PVTSTAGE_H

#include <MaterialXRuntime/Private/PvtElement.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtStage : public PvtAllocatingElement
{
public:
    PvtStage(const RtToken& name);

    static PvtObjectHandle createNew(const RtToken& name);

    const RtTokenList& getSourceUri() const
    {
        return _sourceUri;
    }

    void addSourceUri(const RtToken& uri)
    {
        _sourceUri.push_back(uri);
    }

    void addReference(PvtObjectHandle stage);

    void removeReference(const RtToken& name);

    void removeReferences();

    size_t numReferences() const;

    PvtObjectHandle getReference(size_t index) const;

    PvtObjectHandle findReference(const RtToken& name) const;

    const PvtObjectHandleVec& getReferencedStages() const
    {
        return _refStages;
    }

    PvtObjectHandle findChildByName(const RtToken& name) const override;

    PvtObjectHandle findChildByPath(const string& path) const override;

protected:
    size_t _selfRefCount;
    PvtObjectHandleVec _refStages;
    PvtObjectHandleSet _refStagesSet;
    RtTokenList _sourceUri;
};

}

#endif
