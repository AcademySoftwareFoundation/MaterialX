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
#include <MaterialXRuntime/RtStage.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PvtStage;

class PvtStageIterator
{
public:
    PvtStageIterator() :
        _current(nullptr)
    {
    }

    PvtStageIterator(PvtStage* stage, RtObjectPredicate predicate = nullptr) :
        _current(nullptr),
        _predicate(predicate)
    {
        _stack.push_back(std::make_tuple(stage, -1, -1));
        this->operator++();
    }

    bool operator==(const PvtStageIterator& other) const
    {
        return _current == other._current;
    }

    bool operator!=(const PvtStageIterator& other) const
    {
        return _current != other._current;
    }

    PvtStageIterator& operator++();

    const PvtDataHandle& operator*() const
    {
        return _current;
    }

    bool isDone() const
    {
        return _current != nullptr;
    }

    void abort()
    {
        _current = nullptr;
    }

    PvtStageIterator& begin()
    {
        return *this;
    }

    static const PvtStageIterator& end()
    {
        static const PvtStageIterator NULL_STAGE_ITERATOR;
        return NULL_STAGE_ITERATOR;
    }

private:
    using StackFrame = std::tuple<PvtStage*, int, int>;
    PvtDataHandle _current;
    RtObjectPredicate _predicate;
    vector<StackFrame> _stack;
};


using RtStageVec = vector<RtStagePtr>;
using RtStageMap = RtTokenMap<RtStagePtr>;

class PvtStage
{
public:
    PvtStage(const RtToken& name, RtStageWeakPtr owner);

    static inline PvtStage* ptr(const RtStagePtr& s)
    {
        return static_cast<PvtStage*>(s->_ptr);
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

    const RtTokenList& getSourceUri() const
    {
        return _sourceUri;
    }

    void addSourceUri(const RtToken& uri)
    {
        _sourceUri.push_back(uri);
    }

    void addReference(RtStagePtr stage);

    void removeReference(const RtToken& name);

    void removeReferences();

    PvtStage* findReference(const RtToken& name) const;

    const RtStageVec& getAllReferences() const
    {
        return _refStagesOrder;
    }

    PvtStageIterator traverse(RtObjectPredicate predicate = nullptr)
    {
        return PvtStageIterator(this, predicate);
    }

protected:
    PvtPrim* getPrimAtPathLocal(const PvtPath& path);

    class RootPrim : public PvtNodeGraph
    {
    public:
        RootPrim(RtStageWeakPtr stage) :
            PvtNodeGraph(PvtPath::ROOT_NAME, nullptr),
            _stage(stage)
        {}

        RtStageWeakPtr getStage() const { return _stage; }

    protected:
        RtStageWeakPtr _stage;
    };

    RtToken _name;
    PvtDataHandle _root;

    size_t _selfRefCount;
    RtStageMap _refStagesMap;
    RtStageVec _refStagesOrder;

    RtTokenList _sourceUri;

    friend class PvtPathItem;
};

}

#endif
