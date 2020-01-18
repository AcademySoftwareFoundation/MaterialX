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
class RtInput;

/// Predicate used for filtering objects during traversal.
using RtObjectPredicate = std::function<bool(const RtObject& obj)>;

/// Traversal predicate for specific object types.
template<RtObjType T>
struct RtObjTypePredicate
{
    bool operator()(const RtObject& obj)
    {
        return obj.getObjType() == T;
    }
};

/// Traversal predicate for specific API support.
template<RtApiType T>
struct RtApiTypePredicate
{
    bool operator()(const RtObject& obj)
    {
        return obj.hasApi(T);
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
        _current(0)
    {}

    /// Constructor, setting the prim to iterate on,
    /// and an optional predicate function.
    RtAttrIterator(const RtObject& prim, RtObjectPredicate predicate = nullptr);

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
    RtObject operator*() const;

    /// Iterate to the next sibling.
    RtAttrIterator& operator++();

    /// Return true if there are no more attribute in the iteration.
    bool isDone() const;

    /// Force the iterator to terminate the traversal.
    void abort()
    {
        _prim = nullptr;
        _current = 0;
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
    size_t _current;
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
        _current(0)
    {}

    /// Constructor, setting the prim to iterate on,
    /// and an optional predicate function.
    RtPrimIterator(const RtObject& prim, RtObjectPredicate predicate = nullptr);

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
    RtObject operator*() const;

    /// Iterate to the next sibling.
    RtPrimIterator& operator++();

    /// Return true if there are no more siblings in the iteration.
    bool isDone() const;

    /// Force the iterator to terminate the traversal.
    void abort()
    {
        _prim = nullptr;
        _current = 0;
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
    size_t _current;
    RtObjectPredicate _predicate;
};


using RtEdge = std::pair<RtObject, RtObject>;

/// @class RtConnectionIterator
/// Iterator for traversing the inputs connected to an output attribute.
class RtConnectionIterator
{
public:
    /// Empty constructor.
    RtConnectionIterator() :
        _ptr(nullptr),
        _current(0)
    {}

    /// Constructor, setting the prim to iterate on,
    /// and an optional predicate function.
    RtConnectionIterator(const RtObject& attr);

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

    /// Dereference this iterator, returning the current input.
    RtInput operator*() const;

    /// Iterate to the next sibling.
    RtConnectionIterator& operator++();

    /// Return true if there are no more attribute in the iteration.
    bool isDone() const;

    /// Force the iterator to terminate the traversal.
    void abort()
    {
        _ptr = nullptr;
        _current = 0;
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
    size_t _current;
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
    RtStageIterator(const RtObject& stage, RtObjectPredicate predicate = nullptr);

    /// Copy constructor.
    RtStageIterator(const RtStageIterator& other);

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
    RtObject operator*() const;

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

/*

/// An edge in a node network. First entry is the upstream source
/// port and second entry is the downstream destination port.
/*
class RtPort;
using RtEdge = std::pair<RtPort, RtPort>;

/// @class RtGraphIterator
/// API for traversing port connections. Traversal starts at a given 
/// port and moves upstream, visiting all edges the DAG formed by a
/// node network. Using a predicate the graph can be pruned, terminating
/// the traversal upstream from specific nodes visited.
///
/// TODO: Implement support for the predicate to prune edges in the graph.
///
class RtGraphIterator : public RtApiBase
{
public:
    /// Constructor.
    RtGraphIterator(RtPort root, RtObjectPredicate predicate = nullptr);

    /// Copy constructor.
    RtGraphIterator(const RtGraphIterator& other);

    /// Destructor.
    ~RtGraphIterator();

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Equality operator.
    bool operator==(const RtGraphIterator& other) const;

    /// Inequality operator.
    bool operator!=(const RtGraphIterator& other) const;

    /// Iterate to the next edge in the traversal.
    RtGraphIterator& operator++();

    /// Dereference this iterator, returning the current edge in the
    /// traversal.
    RtEdge operator*() const;

    /// Return true if there are no more edges in the interation.
    bool isDone() const;

    /// Force the iterator to terminate the traversal.
    void abort();

private:
    void* _ptr;
};
*/

}

#endif
