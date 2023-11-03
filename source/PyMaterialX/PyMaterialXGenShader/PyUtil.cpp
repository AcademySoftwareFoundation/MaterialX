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
    mod.def("isTransparentSurface", &mx::isTransparentSurface,
            py::arg("element"),
            py::arg("target") = "",
            PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the given `element` is a surface shader with the potential
    of being transparent. This can be used by HW shader generators to determine
    if a shader will require transparency handling.

    :note: This function will check some common cases for how a surface shader
        can be transparent. It is not covering all possible cases for how
        transparency can be done and target applications might need to do
        additional checks to track transparency correctly. For example, custom
        surface shader nodes implemented in source code will not be tracked by
        this function and transprency for such nodes must be tracked separately
        by the target application.
)docstring"));

    mod.def("mapValueToColor", &mx::mapValueToColor,
            py::arg("value"),
            py::arg("color"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Maps a value to a four channel color if it is of the appropriate type.
    Supported types include `float`, `Vector2`, `Vector3`, `Vector4`,
    and `Color4`. If no mapping is possible the color value is
    set to opaque black.
)docstring"));

    mod.def("requiresImplementation", &mx::requiresImplementation,
            py::arg("nodeDef"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Return whether the given node definition requires an implementation.
)docstring"));

    mod.def("elementRequiresShading", &mx::elementRequiresShading,
            py::arg("element"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Determine if a given `element` requires shading / lighting for rendering.
)docstring"));

    mod.def("findRenderableMaterialNodes", &findRenderableMaterialNodes,
            py::arg("doc"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Find all renderable material nodes in the given document.

    :param doc: The document to examine.
    :type doc: Document
    :return: A list of renderable material nodes.
)docstring"));

    mod.def("findRenderableElements", &findRenderableElements,
            py::arg("doc"),
            py::arg("includeReferencedGraphs") = false,
            PYMATERIALX_DOCSTRING(R"docstring(
    Find all renderable elements in the given document, including material
    nodes if present, or graph outputs of renderable types if no material nodes
    are found.

    :param doc: The document to examine.
    :type doc: Document
    :return: A list of renderable elements.
)docstring"));

    mod.def("getNodeDefInput", &mx::getNodeDefInput,
            py::arg("nodeInput"),
            py::arg("target"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Given a node input, return the corresponding input within its matching
    nodedef.

    The optional `target` string can be used to guide the selection of nodedef
    declarations.
)docstring"));

    mod.def("tokenSubstitution", &mx::tokenSubstitution,
            py::arg("substitutions"),
            py::arg("source"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Perform token substitutions on the given `source` string, using the given
    `substituation` map.

    Tokens are required to start with `"$"` and can only consist of
    alphanumeric characters.

    The full token name, including `"$"` and all following alphanumeric
    characters, will be replaced by the corresponding string in the
    substitution map, if the token exists in the map.
)docstring"));

    mod.def("getUdimCoordinates", &mx::getUdimCoordinates,
            py::arg("udimIdentifiers"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Compute the UDIM coordinates for a set of UDIM identifiers.

    :return: A list of UDIM coordinates.
)docstring"));

    mod.def("getUdimScaleAndOffset", &mx::getUdimScaleAndOffset,
            py::arg("udimCoordinates"),
            py::arg("scaleUV"),
            py::arg("offsetUV"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Get the UV scale and offset to transform uv coordinates from UDIM uv space
    to `0..1` space.
)docstring"));

    mod.def("connectsToWorldSpaceNode", &mx::connectsToWorldSpaceNode,
            py::arg("output"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Determine whether the given `output` is directly connected to a node that
    generates world-space coordinates (e.g. the "normalmap" node).

    :param output: The output to check.
    :type output: Output
    :return: The node if found.
)docstring"));

    mod.def("hasElementAttributes", &mx::hasElementAttributes,
            py::arg("output"),
            py::arg("attributes"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Returns `True` if there are any value elements with a given set of
    attributes either on the starting node or any graph upstream of that node.

    :param output: The starting node.
    :type output: Node
    :param attributes: Attributes to test for.
    :type attributes: List[str]
)docstring"));
}
