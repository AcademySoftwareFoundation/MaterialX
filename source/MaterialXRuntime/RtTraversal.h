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

/// Filter function type used for filtering objects during traversal.
using RtTraversalFilter = std::function<bool(const RtObject& obj)>;

/// Traversal filter for specific object types.
template<RtObjType T>
struct RtObjectFilter
{
    bool operator()(const RtObject& obj)
    {
        return obj.getObjType() == T;
    }
};

/// Traversal filter for specific API support.
template<RtApiType T>
struct RtApiFilter
{
    bool operator()(const RtObject& obj)
    {
        return obj.hasApi(T);
    }
};


/// @class RtStageIterator
/// API for iterating over elements in a stage. Only root level
/// elements are returned. Using a filter this iterator can be
/// used to find all elements of a specific object type, or all
/// objects supporting a specific API, etc.
class RtStageIterator : public RtApiBase
{
public:
    /// Empty constructor.
    RtStageIterator();

    /// Constructor, setting the stage to iterate on and optionally
    /// a filter restricting the set of returned objects.
    RtStageIterator(RtObject stage, RtTraversalFilter filter = nullptr);

    /// Copy constructor.
    RtStageIterator(const RtStageIterator& other);

    /// Assignment operator.
    RtStageIterator& operator=(const RtStageIterator& other);

    /// Destructor.
    ~RtStageIterator();

    /// Return the type for this API.
    RtApiType getApiType() const override;

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

private:
    void* _ptr;
};


/// @class RtTreeIterator
/// API for traversing over the complete tree of elements in a stage.
/// Both root level elements and reqursively their child element are
/// returned. Using a filter this iterator can be used to find all
/// elements of a specific object type, or all objects supporting a
/// specific API, etc.
class RtTreeIterator : public RtApiBase
{
public:
    /// Constructor.
    RtTreeIterator();

    /// Constructor, setting the root element to start traversal on,
    /// and optionally a filter restricting the set of returned objects.
    RtTreeIterator(RtObject root, RtTraversalFilter filter = nullptr);

    /// Copy constructor.
    RtTreeIterator(const RtTreeIterator& other);

    /// Assignment operator.
    RtTreeIterator& operator=(const RtTreeIterator& other);

    /// Destructor.
    ~RtTreeIterator();

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Equality operator.
    bool operator==(const RtTreeIterator& other) const;

    /// Inequality operator.
    bool operator!=(const RtTreeIterator& other) const;

    /// Iterate to the next element in the traversal.
    RtTreeIterator& operator++();

    /// Dereference this iterator, returning the current element in the
    /// traversal.
    RtObject operator*() const;

    /// Return true if there are no more elements in the interation.
    bool isDone() const;

    /// Force the iterator to terminate the traversal.
    void abort();

private:
    void* _ptr;
};

/// An edge in a node network. First entry is the upstream source
/// port and second entry is the downstream destination port.
class RtPort;
using RtEdge = std::pair<RtPort, RtPort>;

/// @class RtGraphIterator
/// API for traversing port connections. Traversal starts at a given 
/// port and moves upstream, visiting all edges the DAG formed by a
/// node network. Using a filter the graph can be pruned, terminating
/// the traversal upstream from specific nodes visited.
///
/// TODO: Implement support for the filter to prune edges in the graph.
///
class RtGraphIterator : public RtApiBase
{
public:
    /// Constructor.
    RtGraphIterator(RtPort root, RtTraversalFilter filter = nullptr);

    /// Copy constructor.
    RtGraphIterator(const RtGraphIterator& other);

    /// Assignment operator.
    RtGraphIterator& operator=(const RtGraphIterator& other);

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

}

#endif
