//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Material.h>
#include <MaterialXCore/Look.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyMaterial(py::module& mod)
{
    mod.def("getShaderNodes", &mx::getShaderNodes,
        py::arg("materialNode"), py::arg("nodeType") = mx::SURFACE_SHADER_TYPE_STRING, py::arg("target") = mx::EMPTY_STRING);
    mod.def("getConnectedOutputs", &mx::getConnectedOutputs);
}
