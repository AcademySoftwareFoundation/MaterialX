//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GENSHADER2_SHADERGRAPHBUILDER_H
#define MATERIALX_GENSHADER2_SHADERGRAPHBUILDER_H

/// @file
/// Constructs a ShaderGraph from an IShaderSource.
///
/// ShaderGraphBuilder replaces the internal ShaderGraph::addUpstreamDependencies()
/// / traverseGraph() traversal with an explicit BFS driven entirely by
/// IShaderSource queries.
///
/// ## Current bridge dependencies
///
/// Most of the graph construction is now driven through pure IShaderSource
/// queries.  Two bridge methods on IShaderSource are still required:
///
///   • getMxNodeDef() — needed to obtain a ConstNodeDefPtr for every node
///     (both root and upstream).  NodeDefs live in the loaded shader library;
///     any backend that can drive generation will have them available.
///
///   • getMxDocument() — no longer required by ShaderGraphBuilder.  The graph
///     is built entirely through IShaderSource queries (Phase 4b/4c).
///     Retained in IShaderSource for compatibility with buildShader().
///
/// getMxNode() is NO LONGER required; upstream node initialization and
/// transform application now go through ShaderGraph2::initializeNode2() and
/// ShaderGraph2::applyInputTransforms2() respectively.
///
/// Known limitation: ShaderNodeImpl::setValues(const Node&, ...) is not
/// called for upstream nodes because it requires a ConstNodePtr.  The only
/// known override is HwImageNode::setValues (UDIM UV normalization).  That
/// path is not exercised by any standard MaterialX shader.

#include <MaterialXGenShader2/Export.h>
#include <MaterialXGenShader2/IShaderSource.h>
#include <MaterialXGenShader2/ShaderGraph2.h>

#include <MaterialXGenShader/GenContext.h>

#include <set>
#include <string>

MATERIALX_NAMESPACE_BEGIN

/// @class ShaderGraphBuilder
/// Builds a ShaderGraph (specifically a ShaderGraph2) from an IShaderSource.
///
/// Usage:
/// @code
///   MxElementAdapter adapter(doc, element);
///   GenContext context(GlslShaderGenerator::create());
///   context.registerSourceCodeSearchPath(searchPath);
///
///   ShaderGraphBuilder builder(adapter, context);
///   ShaderGraph2Ptr graph = builder.build("myShader");
/// @endcode
class MX_GENSHADER2_API ShaderGraphBuilder
{
  public:
    ShaderGraphBuilder(const IShaderSource& source, GenContext& context);

    /// Build a ShaderGraph from the root element provided by IShaderSource.
    /// Returns a finalized ShaderGraph2 equivalent to what
    /// ShaderGraph::create(nullptr, name, element, context) would produce.
    ShaderGraph2Ptr build(const string& name);

  private:
    // ── Graph traversal (replaces ShaderGraph::addUpstreamDependencies) ───────

    /// BFS upstream from rootElem, calling createConnectedNodes for every edge.
    void addUpstreamDependencies(ShaderGraph2& graph, DataHandle rootElem);

    /// Replicates ShaderGraph::createConnectedNodes for a single (downstream,
    /// upstream, connectingInput) triple discovered during BFS.
    void createConnectedNodes(ShaderGraph2& graph,
                              DataHandle downstreamElem,
                              DataHandle upstreamNode,
                              DataHandle connectingInput);

    // ── Helpers ───────────────────────────────────────────────────────────────

    /// Build the root-node case: root is a Node element (the common case for
    /// surface/material shaders).  Populates sockets, creates the root
    /// ShaderNode, and wires everything to graph I/O.
    void buildNodeRoot(ShaderGraph2& graph, DataHandle rootNode, ConstNodeDefPtr nodeDef);

    /// Build the output-socket case: root is an Output element inside a
    /// NodeGraph or floating in the Document.
    void buildOutputRoot(ShaderGraph2& graph, DataHandle rootOutput);

    const IShaderSource& _source;
    GenContext&          _context;
};

MATERIALX_NAMESPACE_END

#endif // MATERIALX_GENSHADER2_SHADERGRAPHBUILDER_H
