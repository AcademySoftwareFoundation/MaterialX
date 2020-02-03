//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTTRAVERSAL_H
#define MATERIALX_RTTRAVERSAL_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>

namespace MaterialX
{

class PvtPrim;
class RtPrim;
class RtStage;

/// Traversal predicate for specific object types.
template<typename T>
struct RtObjTypePredicate
{
    bool operator()(const RtObject& obj)
    {
        return obj.isA<T>();
    }
};

/// @class RtAttrIterator
/// Iterator for traversing over the attributes of a prim.
/// Using a predicate this iterator can be used to find all
/// attributes of a specific kind or type, etc.
class RtAttrIterator
{
public:
    /// Empty constructor.
    RtAttrIterator() :
        _prim(nullptr),
        _current(-1)
    {}

    /// Constructor, setting the prim to iterate on,
    /// and an optional predicate function.
    RtAttrIterator(const RtPrim& prim, RtObjectPredicate predicate = nullptr);

    /// Copy constructor.
    RtAttrIterator(const RtAttrIterator& other) :
        _prim(other._prim),
        _current(other._current),
        _predicate(other._predicate)
    {}

    /// Assignment operator.
    RtAttrIterator& operator=(const RtAttrIterator& other)
    {
        _prim = other._prim;
        _current = other._current;
        _predicate = other._predicate;
        return *this;
    }

    /// Equality operator.
    bool operator==(const RtAttrIterator& other) const
    {
        return _current == other._current &&
            _prim == other._prim;
    }

    /// Inequality operator.
    bool operator!=(const RtAttrIterator& other) const
    {
        return !(*this == other);
    }

    /// Dereference this iterator, returning the current attribute.
    RtAttribute operator*() const;

    /// Iterate to the next sibling.
    RtAttrIterator& operator++();

    /// Return true if there are no more attribute in the iteration.
    bool isDone() const;

    /// Force the iterator to terminate the traversal.
    void abort()
    {
        *this = end();
    }

    /// Interpret this object as an iteration range,
    /// and return its begin iterator.
    RtAttrIterator& begin()
    {
        return *this;
    }

    /// Return the sentinel end iterator for this class.
    static const RtAttrIterator& end();

private:
    const PvtPrim* _prim;
    int _current;
    RtObjectPredicate _predicate;
};


/// @class RtPrimIterator
/// Iterator for traversing over the child prims (siblings) of a prim.
/// Using a predicate this iterator can be used to find all child prims
/// of a specific object type, or all child prims supporting a
/// specific API, etc.
class RtPrimIterator
{
public:
    /// Empty constructor.
    RtPrimIterator() :
        _prim(nullptr),
        _current(-1)
    {}

    /// Constructor, setting the prim to iterate on,
    /// and an optional predicate function.
    RtPrimIterator(const RtPrim& prim, RtObjectPredicate predicate = nullptr);

    /// Copy constructor.
    RtPrimIterator(const RtPrimIterator& other) :
        _prim(other._prim),
        _current(other._current),
        _predicate(other._predicate)
    {}

    /// Assignment operator.
    RtPrimIterator& operator=(const RtPrimIterator& other)
    {
        _prim = other._prim;
        _current = other._current;
        _predicate = other._predicate;
        return *this;
    }

    /// Equality operator.
    bool operator==(const RtPrimIterator& other) const
    {
        return _current == other._current &&
            _prim == other._prim;
    }

    /// Inequality operator.
    bool operator!=(const RtPrimIterator& other) const
    {
        return !(*this == other);
    }

    /// Dereference this iterator, returning the current siblings.
    RtPrim operator*() const;

    /// Iterate to the next sibling.
    RtPrimIterator& operator++();

    /// Return true if there are no more siblings in the iteration.
    bool isDone() const;

    /// Force the iterator to terminate the traversal.
    void abort()
    {
        *this = end();
    }

    /// Interpret this object as an iteration range,
    /// and return its begin iterator.
    RtPrimIterator& begin()
    {
        return *this;
    }

    /// Return the sentinel end iterator for this class.
    static const RtPrimIterator& end();

private:
    const PvtPrim* _prim;
    int _current;
    RtObjectPredicate _predicate;
};


/// @class RtConnectionIterator
/// Iterator for traversing the connections on an output or relationship.
class RtConnectionIterator
{
public:
    /// Empty constructor.
    RtConnectionIterator() :
        _ptr(nullptr),
        _current(-1)
    {}

    /// Constructor, setting the output or relationship to iterate on.
    RtConnectionIterator(const RtObject& obj);

    /// Copy constructor.
    RtConnectionIterator(const RtConnectionIterator& other) :
        _ptr(other._ptr),
        _current(other._current)
    {}

    /// Assignment operator.
    RtConnectionIterator& operator=(const RtConnectionIterator& other)
    {
        _ptr = other._ptr;
        _current = other._current;
        return *this;
    }

    /// Equality operator.
    bool operator==(const RtConnectionIterator& other) const
    {
        return _current == other._current &&
            _ptr == other._ptr;
    }

    /// Inequality operator.
    bool operator!=(const RtConnectionIterator& other) const
    {
        return !(*this == other);
    }

    /// Dereference this iterator, returning the current object.
    RtObject operator*() const;

    /// Iterate to the next sibling.
    RtConnectionIterator& operator++();

    /// Return true if there are no more attribute in the iteration.
    bool isDone() const;

    /// Force the iterator to terminate the traversal.
    void abort()
    {
        *this = end();
    }

    /// Interpret this object as an iteration range,
    /// and return its begin iterator.
    RtConnectionIterator& begin()
    {
        return *this;
    }

    /// Return the sentinel end iterator for this class.
    static const RtConnectionIterator& end();

private:
    void* _ptr;
    int _current;
};

/// @class RtStageIterator
/// API for iterating over prims in a stage, including referenced stages.
/// Only stage level prims are returned. Using a predicate this iterator can be
/// used to find all prims of a specific object type, or all
/// prims supporting a specific API, etc.
class RtStageIterator
{
public:
    /// Empty constructor.
    RtStageIterator();

    /// Constructor, setting the stage to iterate on and optionally
    /// a predicate restricting the set of returned objects.
    RtStageIterator(const RtStagePtr& stage, RtObjectPredicate predicate = nullptr);

    /// Copy constructor.
    RtStageIterator(const RtStageIterator& other);

    /// Assignment operator.
    RtStageIterator& operator=(const RtStageIterator& other);

    /// Destructor.
    ~RtStageIterator();

    /// Equality operator.
    bool operator==(const RtStageIterator& other) const;

    /// Inequality operator.
    bool operator!=(const RtStageIterator& other) const;

    /// Iterate to the next element in the traversal.
    RtStageIterator& operator++();

    /// Dereference this iterator, returning the current object
    /// in the traversal.
    RtPrim operator*() const;

    /// Return true if there are no more objects in the traversal.
    bool isDone() const;

    /// Force the iterator to terminate the traversal.
    void abort();

    /// Interpret this object as an iteration range,
    /// and return its begin iterator.
    RtStageIterator& begin()
    {
        return *this;
    }

    /// Return the sentinel end iterator for this class.
    static const RtStageIterator& end();

private:
    void* _ptr;
};

}

#endif
