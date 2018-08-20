//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_NODE_H
#define MATERIALX_NODE_H

/// @file
/// Node element subclasses

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Definition.h>

namespace MaterialX
{

class Node;
class GraphElement;
class NodeGraph;

/// A shared pointer to a Node
using NodePtr = shared_ptr<Node>;
/// A shared pointer to a const Node
using ConstNodePtr = shared_ptr<const Node>;

/// A shared pointer to a GraphElement
using GraphElementPtr = shared_ptr<GraphElement>;
/// A shared pointer to a const GraphElement
using ConstGraphElementPtr = shared_ptr<const GraphElement>;

/// A shared pointer to a NodeGraph
using NodeGraphPtr = shared_ptr<NodeGraph>;
/// A shared pointer to a const NodeGraph
using ConstNodeGraphPtr = shared_ptr<const NodeGraph>;

/// @class Node
/// A node element within a NodeGraph or Document.
///
/// A Node represents an instance of a NodeDef within a graph, and its Parameter
/// and Input elements apply specific values and connections to that instance.
class Node : public InterfaceElement
{
  public:
    Node(ElementPtr parent, const string& name) :
        InterfaceElement(parent, CATEGORY, name)
    {
    }
    virtual ~Node() { }

    /// @}
    /// @name Connections
    /// @{

    /// Set the Node connected to the given input, creating a child element
    /// for the input if needed.
    InputPtr setConnectedNode(const string& inputName, NodePtr node);

    /// Return the Node connected to the given input.  If the given input is
    /// not present, then an empty NodePtr is returned.
    NodePtr getConnectedNode(const string& inputName) const;

    /// Set the name of the Node connected to the given input, creating a child
    /// element for the input if needed.
    InputPtr setConnectedNodeName(const string& inputName, const string& nodeName);

    /// Return the name of the Node connected to the given input.  If the given
    /// input is not present, then an empty string is returned.
    string getConnectedNodeName(const string& inputName) const;

    /// @}
    /// @name NodeDef References
    /// @{

    /// Return the first NodeDef that declares this node, optionally filtered
    /// by the given target name.
    /// @param target An optional target name, which will be used to filter
    ///    the nodedefs that are considered.
    /// @return A NodeDef for this node, or an empty shared pointer if none
    ///    was found.
    NodeDefPtr getNodeDef(const string& target = EMPTY_STRING) const;

    /// @}
    /// @name Implementation References
    /// @{

    /// Return the first implementation for this node, optionally filtered by
    /// the given target and language names.
    /// @param target An optional target name, which will be used to filter
    ///    the implementations that are considered.
    /// @param language An optional language name, which will be used to filter
    ///    the implementations that are considered.
    /// @return An implementation for this node, or an empty shared pointer if
    ///    none was found.  Note that a node implementation may be either an
    ///    Implementation element or a NodeGraph element.
    InterfaceElementPtr getImplementation(const string& target = EMPTY_STRING,
                                          const string& language = EMPTY_STRING) const
    {
        NodeDefPtr nodeDef = getNodeDef(target);
        return nodeDef ? nodeDef->getImplementation(target, language) : InterfaceElementPtr();
    }

    /// @}
    /// @name Traversal
    /// @{

    /// Return the Edge with the given index that lies directly upstream from
    /// this element in the dataflow graph.
    Edge getUpstreamEdge(ConstMaterialPtr material = nullptr,
                         size_t index = 0) const override;

    /// Return the number of queriable upstream edges for this element.
    size_t getUpstreamEdgeCount() const override
    {
        return getInputCount();
    }

    /// Return a vector of all downstream ports that connect to this node.
    vector<PortElementPtr> getDownstreamPorts() const;

    /// @}
    /// @name Validation
    /// @{

    /// Validate that the given element tree, including all descendants, is
    /// consistent with the MaterialX specification.
    bool validate(string* message = nullptr) const override;

    /// @}

  public:
    static const string CATEGORY;
};

/// @class GraphElement
/// The base class for graph elements such as NodeGraph and Document.
class GraphElement : public InterfaceElement
{
  protected:
    GraphElement(ElementPtr parent, const string& category, const string& name) :
        InterfaceElement(parent, category, name)
    {
    }
  public:
    virtual ~GraphElement() { }

    /// @name Node Elements
    /// @{

    /// Add a Node to the graph.
    /// @param category The category of the new Node.
    /// @param name The name of the new Node.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @param type An optional type string.
    /// @return A shared pointer to the new Node.
    NodePtr addNode(const string& category,
                    const string& name = EMPTY_STRING,
                    const string& type = DEFAULT_TYPE_STRING)
    {
        NodePtr node = addChild<Node>(name);
        node->setCategory(category);
        node->setType(type);
        return node;
    }

    /// Add a Node that is an instance of the given NodeDef.
    NodePtr addNodeInstance(ConstNodeDefPtr nodeDef, const string& name = EMPTY_STRING)
    {
        return addNode(nodeDef->getNodeString(), name, nodeDef->getType());
    }

    /// Return the Node, if any, with the given name.
    NodePtr getNode(const string& name) const
    {
        return getChildOfType<Node>(name);
    }

    /// Return a vector of all Nodes in the graph, optionally filtered by the
    /// given category string.
    vector<NodePtr> getNodes(const string& category = EMPTY_STRING) const
    {
        return getChildrenOfType<Node>(category);
    }

    /// Remove the Node, if any, with the given name.
    void removeNode(const string& name)
    {
        removeChildOfType<Node>(name);
    }

    /// @}
    /// @name Utility
    /// @{

    /// Flatten any references to graph-based node definitions within this
    /// node graph, replacing each reference with the equivalent node network.
    void flattenSubgraphs(const string& target = EMPTY_STRING);

    /// Return a vector of all children (nodes and outputs) sorted in
    /// topological order.
    /// @throws ExceptionFoundCycle if a cycle is encountered.
    vector<ElementPtr> topologicalSort() const;

    /// Convert this graph to a string in the DOT language syntax.  This can be
    /// used to visualise the graph using GraphViz (http://www.graphviz.org).
    ///
    /// If declarations for the contained nodes are provided as nodedefs in
    /// the owning document, then they will be used to provide additional
    /// formatting details.
    string asStringDot() const;

    /// @}
};

/// @class NodeGraph
/// A node graph element within a Document.
class NodeGraph : public GraphElement
{
  public:
    NodeGraph(ElementPtr parent, const string& name) :
        GraphElement(parent, CATEGORY, name)
    {
    }
    virtual ~NodeGraph() { }

  public:
    static const string CATEGORY;
};

} // namespace MaterialX

#endif
