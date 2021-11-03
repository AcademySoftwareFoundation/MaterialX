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

// Return all materials associated with a given root element. 
// The root element types considered are: materialassign, document, and nodegraph.
// The returned material elements are either nodes or nodegraphs of output type 'material'.
/// @param root element to start from.
/// @param skipIncludes Skip nodes that are from an included document. 
MX_CORE_API std::vector<InterfaceElementPtr> getMaterials(ElementPtr root, bool skipIncludes);

} // namespace MaterialX

#endif
