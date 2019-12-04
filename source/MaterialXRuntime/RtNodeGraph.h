//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODEGRAPH_H
#define MATERIALX_RTNODEGRAPH_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtNode.h>

namespace MaterialX
{

/// @class RtNodeGraph
/// API for creating and editing nodegraphs. This API can only be
/// attached to objects of type NODEGRAPH.
class RtNodeGraph : public RtNode
{
public:
    /// Constructor attaching and object to the API.
    RtNodeGraph(const RtObject& obj);

    /// Create a new nodegraph in the given parent.
    /// The parent must be a stage or another nodegraph.
    static RtObject createNew(RtObject parent, const RtToken& name = EMPTY_TOKEN);

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Add a node to the graph.
    void addNode(RtObject node);

    /// Remove a node from the graph.
    void removeNode(RtObject node);

    /// Remove a port from the graph.
    void removePort(RtObject portdef);

    /// Return the node count.
    size_t numNodes() const;

    /// Return a node by index, or a null object 
    /// if no such node exists.
    RtObject getNode(size_t index) const;

    /// Find a node by name. Return a null object 
    /// if no such node is found.
    RtObject findNode(const RtToken& name) const;

    /// Return an output socket by index, or a null object if no such port exists.
    /// Sockets are the internal ports which nodes inside the graph can connect
    /// to in order to interface with the outside.
    /// The given index should be in range [0, numOutputs].
    RtPort getOutputSocket(size_t index) const;

    /// Return an input socket by index, or a null object if no such port exists.
    /// Sockets are the internal ports which nodes inside the graph can connect
    /// to in order to interface with the outside.
    /// The given index should be in range [0, numInputs].
    RtPort getInputSocket(size_t index) const;

    /// Find an output socket by name, or a null object if no such port is found.
    /// Sockets are the internal ports which nodes inside the graph can connect
    /// to in order to interface with the outside.
    RtPort findOutputSocket(const RtToken& name) const;

    /// Find an input socket by name, or a null object if no such port is found.
    /// Sockets are the internal ports which nodes inside the graph can connect
    /// to in order to interface with the outside.
    RtPort findInputSocket(const RtToken& name) const;

    /// Convert this graph to a string in the DOT language syntax. This can be
    /// used to visualise the graph using GraphViz (http://www.graphviz.org).
    string asStringDot() const;
};

}

#endif
