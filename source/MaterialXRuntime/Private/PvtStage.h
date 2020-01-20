//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTSTAGE_H
#define MATERIALX_PVTSTAGE_H

#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtNodeGraph.h>

#include <MaterialXRuntime/RtTraversal.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtStage : public PvtObject
{
public:
    static RtObjType typeId() { return _typeId; }

    static const RtToken& typeName() { return _typeName; }

    static PvtDataHandle createNew(const RtToken& name);

    RtObjType getObjType() const override
    {
        return _typeId;
    }

    const RtToken& getObjTypeName() const override
    {
        return _typeName;
    }

    const RtToken& getName() const
    {
        return _name;
    }

    PvtPrim* createPrim(const PvtPath& path, const RtToken& typeName, PvtObject* def = nullptr);

    PvtPrim* createPrim(const PvtPath& parentPath, const RtToken& name, const RtToken& typeName, PvtObject* def = nullptr);

    void removePrim(const PvtPath& path);

    RtToken renamePrim(const PvtPath& path, const RtToken& newName);

    RtToken reparentPrim(const PvtPath& path, const PvtPath& newParentPath);

    PvtPrim* getPrimAtPath(const PvtPath& path);

    RtPrimIterator getPrims(RtObjectPredicate predicate = nullptr)
    {
        return RtPrimIterator(_root->asA<PvtPrim>()->obj(), predicate);
    }

    PvtPrim* getRootPrim()
    {
        return _root->asA<PvtPrim>();
    }

    PvtPath getPath()  const
    {
        return _root->asA<PvtPrim>()->getPath();
    }

    RtStageIterator traverse(RtObjectPredicate predicate)
    {
        return RtStageIterator(obj(), predicate);
    }

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

    PvtStage* getReference(size_t index) const;

    PvtStage* findReference(const RtToken& name) const;

    const PvtDataHandleVec& getAllReferences() const
    {
        return _refStages;
    }

private:
    static const RtObjType _typeId;
    static const RtToken _typeName;

protected:
    PvtStage(const RtToken& name);

    PvtPrim* getPrimAtPathLocal(const PvtPath& path);

    class RootPrim : public PvtNodeGraph
    {
    public:
        RootPrim(PvtStage* stage) :
            PvtNodeGraph(PvtPath::ROOT_NAME, nullptr),
            _stage(stage)
        {}

        PvtStage* getStage() const { return _stage; }

    protected:
        PvtStage* _stage;
    };

    RtToken _name;
    PvtDataHandle _root;

    size_t _selfRefCount;
    PvtDataHandleVec _refStages;
    PvtDataHandleSet _refStagesSet;

    RtTokenList _sourceUri;

    friend class PvtPathItem;
    friend class RtStageIterator;
};

}

#endif
