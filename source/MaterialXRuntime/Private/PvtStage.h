//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTSTAGE_H
#define MATERIALX_PVTSTAGE_H

#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPath.h>

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

    const PvtObjHandle& operator*() const
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
    PvtObjHandle _current;
    RtObjectPredicate _predicate;
    vector<StackFrame> _stack;
};


using RtStageVec = vector<RtStagePtr>;
using RtStageSet = std::set<const RtStage*>;

class PvtStage
{
public:
    PvtStage(const RtString& name, RtStageWeakPtr owner);

    static RtStagePtr createNew(const RtString& name);

    static inline PvtStage* cast(const RtStagePtr& s)
    {
        return static_cast<PvtStage*>(s->_ptr);
    }

    static inline PvtStage* cast(RtStage* s)
    {
        return static_cast<PvtStage*>(s->_ptr);
    }

    const RtString& getName() const
    {
        return _name;
    }

    PvtPrim* createPrim(const PvtPath& path, const RtString& typeName);

    PvtPrim* createPrim(const PvtPath& parentPath, const RtString& name, const RtString& typeName);

    void removePrim(const PvtPath& path);

    void disposePrim(const PvtPath& path);

    void restorePrim(const PvtPath& parentPath, const RtPrim& prim);

    RtString renamePrim(const PvtPath& path, const RtString& newName);

    RtString reparentPrim(const PvtPath& path, const PvtPath& newParentPath);

    PvtPrim* getPrimAtPath(const PvtPath& path);

    RtPrimIterator getPrims(RtObjectPredicate predicate = nullptr);

    PvtPrim* getRootPrim()
    {
        return _root->asA<PvtPrim>();
    }

    PvtPath getPath()  const
    {
        return _root->asA<PvtPrim>()->getPath();
    }

    const FilePathVec& getSourceUri() const
    {
        return _sourceUri;
    }

    void addSourceUri(const FilePath& uri)
    {
        _sourceUri.push_back(uri);
    }

    void addReference(RtStagePtr stage);

    void removeReference(const RtString& name);

    void removeReferences();

    RtStagePtr getReference(const RtString& name) const;

    const RtStageVec& getAllReferences() const
    {
        return _refStages;
    }

    PvtStageIterator traverse(RtObjectPredicate predicate = nullptr)
    {
        return PvtStageIterator(this, predicate);
    }

protected:
    PvtPrim* getPrimAtPathLocal(const PvtPath& path);

    void setName(const RtString& name)
    {
        _name = name;
    }

    class RootPrim : public PvtPrim
    {
    public:
        RootPrim(RtStageWeakPtr stage);

        RtStageWeakPtr getStage() const { return _stage; }

    protected:
        RtStageWeakPtr _stage;
        static const RtTypeInfo _typeInfo;
    };

    RtString _name;
    PvtObjHandle _root;

    size_t _selfRefCount;
    RtStageVec _refStages;
    RtStageSet _refStagesSet;

    FilePathVec _sourceUri;

    friend class RtStage;
    friend class PvtObject;
};

}

#endif
