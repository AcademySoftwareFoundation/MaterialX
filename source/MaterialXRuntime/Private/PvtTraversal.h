//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTTRAVERSAL_H
#define MATERIALX_PVTTRAVERSAL_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXRuntime/Private/PvtStage.h>
#include <MaterialXRuntime/Private/PvtNode.h>

namespace MaterialX
{

/// @class PvtStageIterator
/// TODO: Docs
class PvtStageIterator
{
public:
    /// Empty constructor.
    PvtStageIterator();

    /// Constructor, setting the stage to iterate on
    /// and optionally a filter function to restrict
    /// the set of returned objects.
    PvtStageIterator(PvtObjectHandle stage, RtTraversalFilter filter = nullptr);

    /// Copy constructor.
    PvtStageIterator(const PvtStageIterator& other);

    /// Equality operator.
    bool operator==(const PvtStageIterator& other) const
    {
        return _current == other._current &&
            _stack == other._stack;
    }

    /// Inequality operator.
    bool operator!=(const PvtStageIterator& other) const
    {
        return !(*this == other);
    }

    /// Dereference this iterator, returning the current element in the
    /// traversal.
    PvtObjectHandle operator*() const
    {
        return _current;
    }

    /// Iterate to the next element in the traversal.
    PvtStageIterator& operator++();

    /// Return true if there are no more elements in the iteration.
    bool isDone() const
    {
        return _current == nullptr;
    }

    /// Force the iterator to terminate the traversal.
    void abort()
    {
        _current = nullptr;
    }

private:
    using StackFrame = std::tuple<PvtStage*, int, int>;

    PvtObjectHandle _current;
    vector<StackFrame> _stack;
    RtTraversalFilter _filter;
};


/// @class PvtTreeIterator
/// TODO: Docs
class PvtTreeIterator
{
public:
    /// Empty constructor.
    PvtTreeIterator();

    /// Constructor, setting the root element to start
    /// the iteration from, and an optional filter function.
    PvtTreeIterator(PvtObjectHandle root, RtTraversalFilter filter = nullptr);

    /// Copy constructor.
    PvtTreeIterator(const PvtTreeIterator& other);

    /// Equality operator.
    bool operator==(const PvtTreeIterator& other) const
    {
        return _current == other._current &&
            _stack == other._stack;
    }

    /// Inequality operator.
    bool operator!=(const PvtTreeIterator& other) const
    {
        return !(*this == other);
    }

    /// Dereference this iterator, returning the current element in the
    /// traversal.
    PvtObjectHandle operator*() const
    {
        return _current;
    }

    /// Iterate to the next element in the traversal.
    PvtTreeIterator& operator++();

    /// Return true if there are no more elements in the iteration.
    bool isDone() const
    {
        return _current == nullptr;
    }

    /// Force the iterator to terminate the traversal.
    void abort()
    {
        _current = nullptr;
    }

private:
    using StackFrame = std::tuple<PvtElement*, int, int>;

    PvtObjectHandle _current;
    vector<StackFrame> _stack;
    RtTraversalFilter _filter;
};



/// @class PvtGraphIterator
/// TODO: Docs
class PvtGraphIterator
{
public:
    /// Empty constructor.
    PvtGraphIterator();

    /// Constructor, setting the root port to start
    /// the iteration on and an optional filter function.
    PvtGraphIterator(RtPort root, RtTraversalFilter filter = nullptr);

    /// Copy constructor.
    PvtGraphIterator(const PvtGraphIterator& other);

    /// Equality operator.
    bool operator==(const PvtGraphIterator& other) const
    {
        return _current == other._current &&
            _stack == other._stack;
    }

    /// Inequality operator.
    bool operator!=(const PvtGraphIterator& other) const
    {
        return !(*this == other);
    }

    /// Dereference this iterator, returning the current element in the
    /// traversal.
    RtEdge operator*() const
    {
        return _current;
    }

    /// Iterate to the next element in the traversal.
    PvtGraphIterator& operator++();

    /// Return true if there are no more elements in the iteration.
    bool isDone() const
    {
        return !_current.first.data();
    }

    /// Force the iterator to terminate the traversal.
    void abort()
    {
        _current.first = RtPort();
    }

private:
    void extendPathUpstream(const RtPort& upstream, const RtPort& downstream);
    void returnPathDownstream(const RtPort& upstream);

    using StackFrame = std::pair<RtPort, size_t>;

    RtEdge _current;
    vector<StackFrame> _stack;
    std::set<RtPort> _path;
    RtTraversalFilter _filter;
};

}

#endif
