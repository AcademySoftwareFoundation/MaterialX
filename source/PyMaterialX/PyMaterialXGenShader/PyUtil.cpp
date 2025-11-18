//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

std::vector<mx::TypedElementPtr> findRenderableMaterialNodes(mx::ConstDocumentPtr doc)
{
    return mx::findRenderableMaterialNodes(doc);
}

std::vector<mx::TypedElementPtr> findRenderableElements(mx::ConstDocumentPtr doc, bool includeReferencedGraphs)
{
    (void) includeReferencedGraphs;
    return mx::findRenderableElements(doc);
}

void bindPyUtil(py::module& mod)
{
    mod.def("isTransparentSurface", &mx::isTransparentSurface, "Returns true if the given element is a surface shader with the potential of being transparent.\n\nThis can be used by HW shader generators to determine if a shader will require transparency handling.\n\nNote: This function will check some common cases for how a surface shader can be transparent. It is not covering all possible cases for how transparency can be done and target applications might need to do additional checks to track transparency correctly. For example, custom surface shader nodes implemented in source code will not be tracked by this function and transparency for such nodes must be tracked separately by the target application.");
    mod.def("mapValueToColor", &mx::mapValueToColor, "Maps a value to a four channel color if it is of the appropriate type.\n\nSupported types include float, Vector2, Vector3, Vector4, and Color4. If not mapping is possible the color value is set to opaque black.");
    mod.def("requiresImplementation", &mx::requiresImplementation, "Return whether a nodedef requires an implementation.");
    mod.def("elementRequiresShading", &mx::elementRequiresShading, "Determine if a given element requires shading / lighting for rendering.");
    mod.def("findRenderableMaterialNodes", &findRenderableMaterialNodes, "");
    mod.def("findRenderableElements", &findRenderableElements, py::arg("doc"), py::arg("includeReferencedGraphs") = false, "");
    mod.def("getNodeDefInput", &mx::getNodeDefInput, "Given a node input, return the corresponding input within its matching nodedef.\n\nThe optional target string can be used to guide the selection of nodedef declarations.");
    mod.def("tokenSubstitution", &mx::tokenSubstitution, "Perform token substitutions on the given source string, using the given substitution map.\n\nTokens are required to start with '$' and can only consist of alphanumeric characters. The full token name, including '$' and all following alphanumeric character, will be replaced by the corresponding string in the substitution map, if the token exists in the map.");
    mod.def("getUdimCoordinates", &mx::getUdimCoordinates, "Compute the UDIM coordinates for a set of UDIM identifiers.\n\nReturns:\n    List of UDIM coordinates");
    mod.def("getUdimScaleAndOffset", &mx::getUdimScaleAndOffset, "Get the UV scale and offset to transform uv coordinates from UDIM uv space to 0..1 space.");
    mod.def("connectsToWorldSpaceNode", &mx::connectsToWorldSpaceNode, "Determine whether the given output is directly connected to a node that generates world-space coordinates (e.g.\n\nArgs:\n    output: Output to check\n\nReturns:\n    Return the node if found.");
    mod.def("hasElementAttributes", &mx::hasElementAttributes, "Returns true if there is are any value elements with a given set of attributes either on the starting node or any graph upsstream of that node.\n\nArgs:\n    output: Starting node\n    attributes: Attributes to test for");
}
