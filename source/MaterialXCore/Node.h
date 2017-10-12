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

/// A shared pointer to a Node
using NodePtr = shared_ptr<class Node>;
/// A shared pointer to a const Node
using ConstNodePtr = shared_ptr<const class Node>;

/// A shared pointer to a NodeGraph
using NodeGraphPtr = shared_ptr<class NodeGraph>;
/// A shared pointer to a const NodeGraph
using ConstNodeGraphPtr = shared_ptr<const class NodeGraph>;

/// @class Node
/// A node element within a NodeGraph.
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
    /// @name References
    /// @{

    /// Return the NodeDef, if any, that this Node references.
    NodeDefPtr getReferencedNodeDef() const;

    /// Return an implementation for this Node, if any, matching the given
    /// target string.  Note that a node implementation may be either an
    /// Implementation element or a NodeGraph element.
    /// @param target The specified target string, which defaults to the
    ///    empty string.
    ElementPtr getImplementation(const string& target = EMPTY_STRING) const;

    /// @}
    /// @name Traversal
    /// @{

    /// Return the Edge with the given index that lies directly upstream from
    /// this element in the dataflow graph.
    Edge getUpstreamEdge(ConstMaterialPtr material = ConstMaterialPtr(),
                         size_t index = 0) override;

    /// Return the number of queriable upstream edges for this element.
    size_t getUpstreamEdgeCount() override
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

/// @class NodeGraph
/// A node graph element within a Document.
class NodeGraph : public Element
{
  public:
    NodeGraph(ElementPtr parent, const string& name) :
        Element(parent, CATEGORY, name)
    {
    }
    virtual ~NodeGraph() { }

    /// @name NodeDef String
    /// @{

    /// Set the NodeDef string for the graph.
    void setNodeDef(const string& target)
    {
        setAttribute(NODE_DEF_ATTRIBUTE, target);
    }

    /// Return true if the given graph has a NodeDef string.
    bool hasNodeDef() const
    {
        return hasAttribute(NODE_DEF_ATTRIBUTE);
    }

    /// Return the NodeDef string for the graph.
    const string& getNodeDef() const
    {
        return getAttribute(NODE_DEF_ATTRIBUTE);
    }

    /// @}
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

    /// Return the Node, if any, with the given name.
    NodePtr getNode(const string& name) const
    {
        return getChildOfType<Node>(name);
    }

    /// Return a vector of all Nodes in the graph.
    vector<NodePtr> getNodes() const
    {
        return getChildrenOfType<Node>();
    }

    /// Remove the Node, if any, with the given name.
    void removeNode(const string& name)
    {
        removeChildOfType<Node>(name);
    }

    /// @}
    /// @name Output Elements
    /// @{

    /// Add a Output to the graph.
    /// @param name The name of the new Output.
    ///     If no name is specified, then a unique name will automatically be
    ///     generated.
    /// @param type An optional type string.
    /// @return A shared pointer to the new Output.
    OutputPtr addOutput(const string& name = EMPTY_STRING,
                        const string& type = DEFAULT_TYPE_STRING)
    {
        OutputPtr output = addChild<Output>(name);
        output->setType(type);
        return output;
    }

    /// Return the Output, if any, with the given name.
    OutputPtr getOutput(const string& name) const
    {
        return getChildOfType<Output>(name);
    }

    /// Return a vector of all Outputs of the graph.
    vector<OutputPtr> getOutputs() const
    {
        return getChildrenOfType<Output>();
    }

    /// Remove the Output, if any, with the given name.
    void removeOutput(const string& name)
    {
        removeChildOfType<Output>(name);
    }

    /// @}
    /// @name Utility
    /// @{

    /// Flatten any references to graph-based node definitions within this
    /// node graph, replacing each reference with the equivalent node network.
    void flattenSubgraphs(const string& target = EMPTY_STRING);

    /// Return a vector of all children (nodes and outputs) sorted in topological order.
    /// @throws ExceptionFoundCycle if a cycle is encountered.
    vector<ElementPtr> topologicalSort() const;

    /// @}

  public:
    static const string CATEGORY;
    static const string NODE_DEF_ATTRIBUTE;
};

} // namespace MaterialX

#endif
