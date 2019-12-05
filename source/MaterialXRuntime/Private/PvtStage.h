//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTSTAGE_H
#define MATERIALX_PVTSTAGE_H

#include <MaterialXRuntime/Private/PvtElement.h>

#include <MaterialXRuntime/Private/PvtPath.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtStage : public PvtAllocatingElement
{
public:
    PvtStage(const RtToken& name);

    static PvtDataHandle createNew(const RtToken& name);

    const RtTokenList& getSourceUri() const
    {
        return _sourceUri;
    }

    void addSourceUri(const RtToken& uri)
    {
        _sourceUri.push_back(uri);
    }

    void addReference(PvtDataHandle stage);

    void removeReference(const RtToken& name);

    void removeReferences();

    size_t numReferences() const;

    PvtDataHandle getReference(size_t index) const;

    PvtDataHandle findReference(const RtToken& name) const;

    const PvtDataHandleVec& getReferencedStages() const
    {
        return _refStages;
    }

    PvtDataHandle findChildByName(const RtToken& name) const override;

    PvtDataHandle findChildByPath(const string& path) const override;

    void removeChildByPath(const PvtPath& path);

protected:
    size_t _selfRefCount;
    PvtDataHandleVec _refStages;
    PvtDataHandleSet _refStagesSet;
    RtTokenList _sourceUri;
};

}

#endif
