//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Material.h>
#include <MaterialXCore/MaterialNode.h>
#include <MaterialXCore/Look.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyMaterial(py::module& mod)
{
    py::class_<mx::Material, mx::MaterialPtr, mx::Element>(mod, "Material")
        .def("getShaderNodeDefs", &mx::Material::getShaderNodeDefs,
            py::arg("target") = mx::EMPTY_STRING, py::arg("type") = mx::EMPTY_STRING)
        .def("getPrimaryShaderNodeDef", &mx::Material::getPrimaryShaderNodeDef,
            py::arg("target") = mx::EMPTY_STRING, py::arg("type") = mx::EMPTY_STRING)
        .def("getPrimaryShaderName", &mx::Material::getPrimaryShaderName,
            py::arg("target") = mx::EMPTY_STRING, py::arg("type") = mx::EMPTY_STRING)
        .def("getPrimaryShaderInputs", &mx::Material::getPrimaryShaderInputs,
            py::arg("target") = mx::EMPTY_STRING, py::arg("type") = mx::EMPTY_STRING)
        .def("getPrimaryShaderTokens", &mx::Material::getPrimaryShaderTokens,
            py::arg("target") = mx::EMPTY_STRING, py::arg("type") = mx::EMPTY_STRING)
        .def("getGeometryBindings", &mx::Material::getGeometryBindings,
            py::arg("geom") = mx::UNIVERSAL_GEOM_NAME)
        .def_readonly_static("CATEGORY", &mx::Material::CATEGORY);

    mod.def("getShaderNodes", &mx::getShaderNodes);
    mod.def("getGeometryBindings", &mx::getGeometryBindings);
    mod.def("getConnectedOutputs", &mx::getConnectedOutputs);
}

