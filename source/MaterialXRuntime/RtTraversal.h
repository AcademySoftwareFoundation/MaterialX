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

/// Struct holding attribute information as returned from the RtAttributeIterator.
struct RtAttribute
{
    RtString name;
    RtTypedValue* value;
    RtAttribute(RtString n, RtTypedValue* v) : name(n), value(v) {}
};

/// @class RtAttributeIterator
/// API for iterating over all attributes on an object.
class RtAttributeIterator
{
public:
    /// Empty constructor.
    RtAttributeIterator();

    /// Constructor, setting the object to iterate on.
    RtAttributeIterator(const RtObject& obj);

    /// Copy constructor.
    RtAttributeIterator(const RtAttributeIterator& other);

    /// Assignment operator.
    RtAttributeIterator& operator=(const RtAttributeIterator& other);

    /// Destructor.
    ~RtAttributeIterator();

    /// Equality operator.
    bool operator==(const RtAttributeIterator& other) const;

    /// Inequality operator.
    bool operator!=(const RtAttributeIterator& other) const
    {
        return !(*this == other);
    }

    /// Iterate to the next attribute in the traversal.
    RtAttributeIterator& operator++();

    /// Dereference this iterator, returning the current attribute
    /// in the traversal.
    RtAttribute operator*() const;

    /// Return true if there are no more attributes in the traversal.
    bool isDone() const;

    /// Force the iterator to terminate the traversal.
    void abort();

    /// Interpret this object as an iteration range,
    /// and return its begin iterator.
    RtAttributeIterator& begin()
    {
        return *this;
    }

    /// Return the sentinel end iterator for this class.
    static const RtAttributeIterator& end();

private:
    void* _ptr;
};


/// Traversal predicate for specific object types.
template<typename T>
struct RtObjTypePredicate
{
    bool operator()(const RtObject& obj)
    {
        return obj.isA<T>();
    }
};

/// @class RtObjectIterator
/// Base class for objects iterators.
template<class T>
class RtObjectIterator
{
public:
    /// Copy constructor.
    RtObjectIterator(const RtObjectIterator& other) :
        _ptr(other._ptr),
        _current(other._current),
        _predicate(other._predicate)
    {}

    /// Assignment operator.
    RtObjectIterator& operator=(const RtObjectIterator& other)
    {
        _ptr = other._ptr;
        _current = other._current;
        _predicate = other._predicate;
        return *this;
    }

    /// Equality operator.
    bool operator==(const RtObjectIterator& other) const
    {
        return _current == other._current &&
            _ptr == other._ptr;
    }

    /// Inequality operator.
    bool operator!=(const RtObjectIterator& other) const
    {
        return !(*this == other);
    }

    /// Dereference this iterator, returning the current object.
    T operator*() const;

    /// Iterate to the next sibling.
    RtObjectIterator& operator++();

    /// Return true if there are no more attribute in the iteration.
    bool isDone() const;

    /// Force the iterator to terminate the traversal.
    void abort()
    {
        *this = end();
    }

    /// Interpret this object as an iteration range,
    /// and return its begin iterator.
    RtObjectIterator& begin()
    {
        return *this;
    }

    /// Return the sentinel end iterator for this class.
    static const RtObjectIterator& end();

protected:
    /// Empty constructor.
    RtObjectIterator(RtObjectPredicate predicate = nullptr) :
        _ptr(nullptr),
        _current(-1),
        _predicate(predicate)
    {}

    void* _ptr;
    int _current;
    RtObjectPredicate _predicate;
};

class RtPrimIterator : public RtObjectIterator<RtPrim>
{
public:
    RtPrimIterator(const RtObject& obj, RtObjectPredicate predicate = nullptr);
};

class RtInputIterator : public RtObjectIterator<RtInput>
{
public:
    RtInputIterator(const RtObject& obj);
};

class RtOutputIterator : public RtObjectIterator<RtOutput>
{
public:
    RtOutputIterator(const RtObject& obj);
};

class RtRelationshipIterator : public RtObjectIterator<RtRelationship>
{
public:
    RtRelationshipIterator(const RtObject& obj);
};


/// @class RtConnectionIterator
/// API for iterating over connections on an output or relationship.
class RtConnectionIterator
{
public:
    /// Empty constructor.
    RtConnectionIterator();

    /// Constructor, setting the output or relationship to iterate on.
    RtConnectionIterator(const RtObject& obj);

    /// Copy constructor.
    RtConnectionIterator(const RtConnectionIterator& other):
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

    /// Iterate to the next element in the traversal.
    RtConnectionIterator& operator++();

    /// Dereference this iterator, returning the current object
    /// in the traversal.
    RtObject operator*() const;

    /// Return true if there are no more objects in the traversal.
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
    bool operator!=(const RtStageIterator& other) const
    {
        return !(*this == other);
    }

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
