//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Material.h>
#include <MaterialXCore/Look.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyMaterial(py::module& mod)
{
    mod.def("getShaderNodes", &mx::getShaderNodes,
            py::arg("materialNode"),
            py::arg("nodeType") = mx::SURFACE_SHADER_TYPE_STRING,
            py::arg("target") = mx::EMPTY_STRING,
            R"docstring(
    Return a list of all shader nodes connected to the given `materialNode`'s inputs,
    filtered by the given shader `nodeType` and `target`.

    By default, all surface shader nodes are returned.

    :param materialNode: The node to examine.
    :param nodeType: The shader node type to return.  Defaults to the surface shader type.
    :param target: An optional target name, which will be used to filter the returned nodes.
)docstring");

    mod.def("getConnectedOutputs", &mx::getConnectedOutputs,
            py::arg("node"),
            R"docstring(
    Return a list of all outputs connected to the given `node`'s inputs.
)docstring");
}
