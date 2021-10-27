//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_MATERIAL_H
#define MATERIALX_MATERIAL_H

/// @file
/// Material node helper functions

#include <MaterialXCore/Export.h>

#include <MaterialXCore/Node.h>
#include <set>

namespace MaterialX
{

/// Return a vector of all shader nodes connected to the given material node's inputs,
/// filtered by the given shader type and target.  By default, all surface shader nodes
/// are returned.
/// @param materialNode The node to examine.
/// @param nodeType THe shader node type to return.  Defaults to the surface shader type.
/// @param target An optional target name, which will be used to filter the returned nodes.
MX_CORE_API vector<NodePtr> getShaderNodes(NodePtr materialNode,
                                           const string& nodeType = SURFACE_SHADER_TYPE_STRING,
                                           const string& target = EMPTY_STRING);

/// Return a vector of all outputs connected to the given node's inputs.
MX_CORE_API vector<OutputPtr> getConnectedOutputs(NodePtr node);

/// Return a list upstream material nodes which are connected to a given root element.
/// The root elements which are considered are materialassigns, documents, and nodegraphs
/// @param root element to start from
/// @param addUnconnectedNodes Whether to return material nodes which are not connected to an output when the
/// root provided is either a nodegraph or a document. This option has no effect if a materialassign is
/// passed as a root.
/// @param skipIncludes Skip nodes that are from an included document. 
MX_CORE_API std::vector<NodePtr> getMaterialNodes(ElementPtr root, bool addUnconnectedNodes, bool skipIncludes);

} // namespace MaterialX

#endif
