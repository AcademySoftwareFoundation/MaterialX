//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODEGRAPH_H
#define MATERIALX_RTNODEGRAPH_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeDef.h>

namespace MaterialX
{

class RtPrimIterator;

/// @class RtNodeGraph
/// Schema for nodegraph prims.
class RtNodeGraph : public RtNode
{
    DECLARE_TYPED_SCHEMA(RtNodeGraph)

public:
    RtNodeGraph(const RtPrim& prim) : RtNode(prim) {}

    /// Add an input attribute to the graph.
    RtInput createInput(const RtIdentifier& name, const RtIdentifier& type, uint32_t flags = 0);

    /// Remove an input attribute from the graph.
    void removeInput(const RtIdentifier& name);

    /// Rename an input attribute in the graph. Return the actual new name which may not
    /// match the provided newName as it may already exist as a child node, input or output
    /// name.
    RtIdentifier renameInput(const RtIdentifier& name, const RtIdentifier& newName);

    /// Add an output attribute to the graph.
    RtOutput createOutput(const RtIdentifier& name, const RtIdentifier& type, uint32_t flags = 0);

    /// Remove an output attribute from the graph.
    void removeOutput(const RtIdentifier& name);

    /// Rename an output attribute in the graph. Return the actual new name which may not
    /// match the provided newName as it may already exist as a child node, input or output
    /// name.
    RtIdentifier renameOutput(const RtIdentifier& name, const RtIdentifier& newName);

    /// Return the internal socket that corresponds
    /// to the named input port.
    RtOutput getInputSocket(const RtIdentifier& name) const;

    /// Return the internal socket that corresponds
    /// to the named output port.
    RtInput getOutputSocket(const RtIdentifier& name) const;

    /// Return a node layout struct for this graph.
    /// Containing its input ordering and uifolder hierarchy.
    RtNodeLayout getNodeLayout();

    /// Set the node layout for this graph, reordering its inputs
    /// and uifolder hierarchy according to the given layout struct.
    void setNodeLayout(const RtNodeLayout& layout);

    /// Return a node by name.
    RtPrim getNode(const RtIdentifier& name) const;

    /// Return an iterator over the nodes in the graph.
    RtPrimIterator getNodes() const;

    /// Set the associated nodedef name.
    void setDefinition(const RtIdentifier& nodedef);

    /// Return any associated nodedef name.
    const RtIdentifier& getDefinition() const;

    /// Set the namespace for the nodegraph.
    void setNamespace(const RtIdentifier& nodedef);

    /// Return the namespace name.
    const RtIdentifier& getNamespace() const;

    /// Convert this graph to a string in the DOT language syntax. This can be
    /// used to visualise the graph using GraphViz (http://www.graphviz.org).
    string asStringDot() const;
};

}

#endif
