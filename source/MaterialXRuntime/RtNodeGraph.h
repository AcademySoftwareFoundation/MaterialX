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
    /// Constructor attaching an object to the API.
    RtNodeGraph(const RtObject& obj);

    /// Return the type name for nodegraphs.
    static const RtToken& typeName();

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Add an attribute to the definition.
    RtObject createAttribute(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove an attribute from the definition.
    void removeAttribute(const RtToken& name);

    /// Convert this graph to a string in the DOT language syntax. This can be
    /// used to visualise the graph using GraphViz (http://www.graphviz.org).
    string asStringDot() const;
};

}

#endif
