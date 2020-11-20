//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_MATERIALNODE_H
#define MATERIALX_MATERIALNODE_H

/// @file
/// Material node utilities 

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Element.h>
#include <MaterialXCore/Interface.h>

#include <unordered_set>

namespace MaterialX
{

/// Convert usage of Material Elements to Material nodes
/// @param doc Document to convert
/// @return If any conversion occurred.
bool convertMaterialsToNodes(DocumentPtr doc);

/// Return a vector of all nodes connected to a Material node's inputs. The default behavior
/// is to return connected surface shader nodes.
/// @param materialNode Node to examine.
/// @param nodeType Type of node to return. If an empty string is specified then
///                 all node types are returned. The default argument value is to return surface shaders.
/// @param target Target attribute filter for nodes to return. The default argument value is an empty string
///               indicating to include nodes which match any target.
std::unordered_set<NodePtr> getShaderNodes(const NodePtr& materialNode,
                                           const string& nodeType = SURFACE_SHADER_TYPE_STRING,
                                           const string& target = EMPTY_STRING);

/// Return a vector of all MaterialAssign elements that bind this material node
/// to the given geometry string
/// @param materialNode Node to examine
/// @param geom The geometry for which material bindings should be returned.
///             By default, this argument is the universal geometry string "/",
///             and all material bindings are returned.
/// @return Vector of MaterialAssign elements
vector<MaterialAssignPtr> getGeometryBindings(const NodePtr& materialNode, const string& geom);


/// Return a vector of all outputs that this nodes inputs are connected to.
vector<OutputPtr> getConnectedOutputs(const NodePtr& node);

} // namespace MaterialX

#endif
