//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODEGRAPH_H
#define MATERIALX_RTNODEGRAPH_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtNode.h>

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
    RtInput createInput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove an input attribute from the graph.
    void removeInput(const RtToken& name);

    /// Rename an input attribute in the graph.
    void renameInput(const RtToken& name, const RtToken& newName);

    /// Add an output attribute to the graph.
    RtOutput createOutput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove an output attribute from the graph.
    void removeOutput(const RtToken& name);

    /// Rename an output attribute in the graph.
    void renameOutput(const RtToken& name, const RtToken& newName);

    /// Return the internal socket that corresponds
    /// to the named input attribute.
    RtOutput getInputSocket(const RtToken& name) const;

    /// Return the internal socket that corresponds
    /// to the named output attribute.
    RtInput getOutputSocket(const RtToken& name) const;

    /// Return a node by name.
    RtPrim getNode(const RtToken& name) const;

    /// Return an iterator over the nodes in the graph.
    RtPrimIterator getNodes() const;

    /// Convert this graph to a string in the DOT language syntax. This can be
    /// used to visualise the graph using GraphViz (http://www.graphviz.org).
    string asStringDot() const;
};

}

#endif
