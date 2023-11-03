//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderNode.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShaderPort(py::module& mod)
{
    py::class_<mx::ShaderPort, mx::ShaderPortPtr>(mod, "ShaderPort")

        .def("setType", &mx::ShaderPort::setType,
             py::arg("typeDesc"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the data type for this port.
)docstring"))

        .def("getType", &mx::ShaderPort::getType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the data type of this port.
)docstring"))

        .def("setName", &mx::ShaderPort::setName,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the name of this port.
)docstring"))

        .def("getName", &mx::ShaderPort::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the name of this port.
)docstring"))

        .def("getFullName", &mx::ShaderPort::getFullName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the full name of this port.
)docstring"))

        .def("setVariable", &mx::ShaderPort::setVariable,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the variable name of this port.
)docstring"))

        .def("getVariable", &mx::ShaderPort::getVariable,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the variable name of this port.
)docstring"))

        .def("setSemantic", &mx::ShaderPort::setSemantic,
             py::arg("semantic"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the variable semantic of this port.
)docstring"))

        .def("getSemantic", &mx::ShaderPort::getSemantic,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the variable semantic of this port.
)docstring"))

        .def("setValue", &mx::ShaderPort::setValue,
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set a value on this port.
)docstring"))

        .def("getValue", &mx::ShaderPort::getValue,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the value set on this port.
)docstring"))

        .def("getValueString", &mx::ShaderPort::getValueString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the value set on this port as a string, or an empty string if there
    is no value.
)docstring"))

        .def("setGeomProp", &mx::ShaderPort::setGeomProp,
             py::arg("geomprop"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set geomprop name if the input has a default geomprop to be assigned when
    it is unconnected.
)docstring"))

        .def("getGeomProp", &mx::ShaderPort::getGeomProp,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return geomprop name.
)docstring"))

        .def("setPath", &mx::ShaderPort::setPath,
             py::arg("path"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the path to this port.
)docstring"))

        .def("getPath", &mx::ShaderPort::getPath,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the path to this port.
)docstring"))

        .def("setUnit", &mx::ShaderPort::setUnit,
             py::arg("unit"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set a unit type for the value on this port.
)docstring"))

        .def("getUnit", &mx::ShaderPort::getUnit,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the unit type for the value on this port.
)docstring"))

        .def("setColorSpace", &mx::ShaderPort::setColorSpace,
             py::arg("colorspace"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set a source color space for the value on this port.
)docstring"))

        .def("getColorSpace", &mx::ShaderPort::getColorSpace,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the source color space for the value on this port.
)docstring"))

        .def("isUniform", &mx::ShaderPort::isUniform,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the uniform flag on this port.
)docstring"))

        .def("isEmitted", &mx::ShaderPort::isEmitted,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the emitted state of this port.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing an input or output port on a `ShaderNode`.

    :see: https://materialx.org/docs/api/class_shader_port.html
)docstring");
}
