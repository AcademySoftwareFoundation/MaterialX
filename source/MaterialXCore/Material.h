//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_MATERIAL_H
#define MATERIALX_MATERIAL_H

/// @file
/// Material element subclasses

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Geom.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Value.h>

#include <unordered_set>

namespace MaterialX
{

/// Return a vector of all nodes connected to a Material node's inputs. The default behavior
/// is to return connected surface shader nodes.
/// @param materialNode Node to examine.
/// @param nodeType Type of node to return. If an empty string is specified then
///                 all node types are returned. The default argument value is to return surface shaders.
/// @param target Target attribute filter for nodes to return. The default argument value is an empty string
///               indicating to include nodes which match any target.
std::unordered_set<NodePtr> getShaderNodes(NodePtr materialNode,
                                           const string& nodeType = SURFACE_SHADER_TYPE_STRING,
                                           const string& target = EMPTY_STRING);

/// Return a vector of all outputs that this nodes inputs are connected to.
vector<OutputPtr> getConnectedOutputs(const NodePtr& node);

} // namespace MaterialX

#endif
