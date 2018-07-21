//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TRAVERSAL_H
#define MATERIALX_TRAVERSAL_H

/// @file
/// Graph traversal classes

#include <MaterialXCore/Library.h>

namespace MaterialX
{

class Element;
class Material;

using ElementPtr = shared_ptr<Element>;
using ConstElementPtr = shared_ptr<const Element>;
using ConstMaterialPtr = shared_ptr<const Material>;

/// @class Edge
/// An edge between two connected Elements, returned during graph traversal.
///
/// A valid Edge consists of a downstream element, an upstream element, and
/// optionally a connecting element that binds them.  As an example, the edge
/// between two Node elements will contain a connecting element for the Input
/// of the downstream Node.
/// @sa Element::traverseGraph
class Edge
{
  public:
    Edge(ElementPtr elemDown, ElementPtr elemConnect, ElementPtr elemUp) :
        _elemDown(elemDown),
        _elemConnect(elemConnect),
        _elemUp(elemUp)
    {
    }
    ~Edge() { }

    bool operator==(const Edge& rhs) const
    {
        return _elemDown == rhs._elemDown &&
               _elemConnect == rhs._elemConnect &&
               _elemUp == rhs._elemUp;
    }
    bool operator!=(const Edge& rhs) const
    {
        return !(*this == rhs);
    }
    bool operator<(const Edge& rhs) const
    {
        return std::tie(_elemDown, _elemConnect, _elemUp) < std::tie(rhs._elemDown, rhs._elemConnect, rhs._elemUp);
    }

    operator bool() const;

    /// Return the downstream element of the edge.
    ElementPtr getDownstreamElement() const
    {
        return _elemDown;
    }

    /// Return the connecting element of the edge, if any.
    ElementPtr getConnectingElement() const
    {
        return _elemConnect;
    }

    /// Return the upstream element of the edge.
    ElementPtr getUpstreamElement() const
    {
        return _elemUp;
    }

    /// Return the name of this edge, if any.
    string getName() const;

  private:
    ElementPtr _elemDown;
    ElementPtr _elemConnect;
    ElementPtr _elemUp;
};

/// @class TreeIterator
/// An iterator object representing the state of a tree traversal.
///
/// @sa Element::traverseTree
class TreeIterator
{
  public:
    explicit TreeIterator(ElementPtr elem):
        _elem(elem),
        _prune(false),
        _holdCount(0)
    {
    }
    ~TreeIterator() { }

  private:
    using StackFrame = std::pair<ElementPtr, size_t>;

  public:
    bool operator==(const TreeIterator& rhs) const
    {
        return _elem == rhs._elem &&
               _stack == rhs._stack &&
               _prune == rhs._prune;
    }
    bool operator!=(const TreeIterator& rhs) const
    {
        return !(*this == rhs);
    }

    /// Dereference this iterator, returning the current element in the
    /// traversal.
    ElementPtr operator*() const
    {
        return _elem;
    }

    /// Iterate to the next element in the traversal.
    TreeIterator& operator++();

    /// @name Elements
    /// @{

    /// Return the current element in the traversal.
    ElementPtr getElement() const
    {
        return _elem;
    }

    /// @}
    /// @name Depth
    /// @{

    /// Return the element depth of the current traversal, where the starting
    /// element represents a depth of zero.
    size_t getElementDepth() const
    {
        return _stack.size();
    }

    /// @}
    /// @name Pruning
    /// @{

    /// Set the prune subtree flag, which controls whether the current subtree
    /// is pruned from traversal.
    /// @param prune If set to true, then the current subtree will be pruned.
    void setPruneSubtree(bool prune)
    {
        _prune = prune;
    }

    /// Return the prune subtree flag, which controls whether the current
    /// subtree is pruned from traversal.
    bool getPruneSubtree() const
    {
        return _prune;
    }

    /// @}
    /// @name Range Methods
    /// @{

    /// Interpret this object as an iteration range, and return its begin
    /// iterator.
    TreeIterator& begin(size_t holdCount = 0)
    {
        _holdCount = holdCount;
        return *this;
    }

    /// Return the sentinel end iterator for this class.
    static const TreeIterator& end();

    /// @}

  private:
    ElementPtr _elem;
    vector<StackFrame> _stack;
    bool _prune;
    size_t _holdCount;
};

/// @class GraphIterator
/// An iterator object representing the state of an upstream graph traversal.
///
/// @sa Element::traverseGraph
class GraphIterator
{
  public:
    explicit GraphIterator(ElementPtr elem, ConstMaterialPtr material = nullptr):
        _upstreamElem(elem),
        _material(material),
        _prune(false),
        _holdCount(0)
    {
        _pathElems.insert(elem);
    }
    ~GraphIterator() { }

  private:
    using ElementSet = std::set<ElementPtr>;
    using StackFrame = std::pair<ElementPtr, size_t>;

  public:
    bool operator==(const GraphIterator& rhs) const
    {
        return _upstreamElem == rhs._upstreamElem &&
               _material == rhs._material &&
               _stack == rhs._stack &&
               _prune == rhs._prune;
    }
    bool operator!=(const GraphIterator& rhs) const
    {
        return !(*this == rhs);
    }

    /// Dereference this iterator, returning the current edge in the traversal.
    Edge operator*() const
    {
        return Edge(getDownstreamElement(),
                    getConnectingElement(),
                    getUpstreamElement());
    }

    /// Iterate to the next edge in the traversal.
    /// @throws ExceptionFoundCycle if a cycle is encountered.
    GraphIterator& operator++();

    /// @name Elements
    /// @{

    /// Return the downstream element of the current edge.
    ElementPtr getDownstreamElement() const
    {
        return !_stack.empty() ? _stack.back().first : ElementPtr();
    }

    /// Return the connecting element, if any, of the current edge.
    ElementPtr getConnectingElement() const
    {
        return _connectingElem;
    }

    /// Return the upstream element of the current edge.
    ElementPtr getUpstreamElement() const
    {
        return _upstreamElem;
    }

    /// Return the index of the current edge within the range of upstream edges
    /// available to the downstream element.
    size_t getUpstreamIndex() const
    {
        return !_stack.empty() ? _stack.back().second : 0;
    }

    /// @}
    /// @name Depth
    /// @{

    /// Return the element depth of the current traversal, where a single edge
    /// between two elements represents a depth of one.
    size_t getElementDepth() const
    {
        return _stack.size();
    }

    /// Return the node depth of the current traversal, where a single edge
    /// between two nodes represents a depth of one.
    size_t getNodeDepth() const;

    /// @}
    /// @name Pruning
    /// @{

    /// Set the prune subgraph flag, which controls whether the current subgraph
    /// is pruned from traversal.
    /// @param prune If set to true, then the current subgraph will be pruned.
    void setPruneSubgraph(bool prune)
    {
        _prune = prune;
    }

    /// Return the prune subgraph flag, which controls whether the current
    /// subgraph is pruned from traversal.
    bool getPruneSubgraph() const
    {
        return _prune;
    }

    /// @}
    /// @name Range Methods
    /// @{

    /// Interpret this object as an iteration range, and return its begin
    /// iterator.
    GraphIterator& begin(size_t holdCount = 0)
    {
        // Increment once to generate a valid edge.
        if (_stack.empty())
        {
            operator++();
        }

        _holdCount = holdCount;
        return *this;
    }

    /// Return the sentinel end iterator for this class.
    static const GraphIterator& end();

    /// @}

  private:
    void extendPathUpstream(ElementPtr upstreamElem, ElementPtr connectingElem);
    void returnPathDownstream(ElementPtr upstreamElem);

  private:
    ElementPtr _upstreamElem;
    ElementPtr _connectingElem;
    ElementSet _pathElems;
    ConstMaterialPtr _material;
    vector<StackFrame> _stack;
    bool _prune;
    size_t _holdCount;
};

/// @class InheritanceIterator
/// An iterator object representing the current state of an inheritance traversal.
///
/// @sa Element::traverseInheritance
class InheritanceIterator
{
  public:
    explicit InheritanceIterator(ConstElementPtr elem) :
        _elem(elem),
        _holdCount(0)
    {
        _pathElems.insert(elem);
    }
    ~InheritanceIterator() { }

  private:
    using ConstElementSet = std::set<ConstElementPtr>;

  public:
    bool operator==(const InheritanceIterator& rhs) const
    {
        return _elem == rhs._elem;
    }
    bool operator!=(const InheritanceIterator& rhs) const
    {
        return !(*this == rhs);
    }

    /// Dereference this iterator, returning the current element in the
    /// traversal.
    ConstElementPtr operator*() const
    {
        return _elem;
    }

    /// Iterate to the next element in the traversal.
    /// @throws ExceptionFoundCycle if a cycle is encountered.
    InheritanceIterator& operator++();

    /// Interpret this object as an iteration range, and return its begin
    /// iterator.
    InheritanceIterator& begin(size_t holdCount = 0)
    {
        _holdCount = holdCount;
        return *this;
    }

    /// Return the sentinel end iterator for this class.
    static const InheritanceIterator& end();

  private:
    ConstElementPtr _elem;
    ConstElementSet _pathElems;
    size_t _holdCount;
};

/// @class ExceptionFoundCycle
/// An exception that is thrown when a traversal call encounters a cycle.
class ExceptionFoundCycle : public Exception
{
  public:
    using Exception::Exception;
};

extern const Edge NULL_EDGE;

extern const TreeIterator NULL_TREE_ITERATOR;
extern const GraphIterator NULL_GRAPH_ITERATOR;
extern const InheritanceIterator NULL_INHERITANCE_ITERATOR;

} // namespace MaterialX

#endif
