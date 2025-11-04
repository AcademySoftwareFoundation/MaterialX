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
    py::class_<mx::ShaderPort, mx::ShaderPortPtr>(mod, "ShaderPort", "An input or output port on a ShaderNode.")
        .def("setType", &mx::ShaderPort::setType, "Set the data type for this port.")
        .def("getType", &mx::ShaderPort::getType, "Get stream attribute name.")
        .def("setName", &mx::ShaderPort::setName, "Set the element's name string.")
        .def("getName", &mx::ShaderPort::getName, "Return the ColorManagementSystem name.")
        .def("getFullName", &mx::ShaderPort::getFullName, "Return the name of this port.")
        .def("setVariable", &mx::ShaderPort::setVariable, "Set the variable name of this port.")
        .def("getVariable", &mx::ShaderPort::getVariable, "Return the variable name of this port.")
        .def("setSemantic", &mx::ShaderPort::setSemantic, "Set the variable semantic of this port.")
        .def("getSemantic", &mx::ShaderPort::getSemantic, "Return the variable semantic of this port.")
        .def("setValue", &mx::ShaderPort::setValue, "Set the typed value of an element from a C-style string.")
        .def("getValue", &mx::ShaderPort::getValue, "Returns a value formatted according to this type syntax.\n\nThe value is constructed from the given value object.")
        .def("getValueString", &mx::ShaderPort::getValueString, "Return value string.")
        .def("setGeomProp", &mx::ShaderPort::setGeomProp, "Set the geometric property string of this element.")
        .def("getGeomProp", &mx::ShaderPort::getGeomProp, "Return the GeomProp, if any, with the given name.")
        .def("setPath", &mx::ShaderPort::setPath, "Set the path to this port.")
        .def("getPath", &mx::ShaderPort::getPath, "Return the path to this port.")
        .def("setUnit", &mx::ShaderPort::setUnit, "Set a unit type for the value on this port.")
        .def("getUnit", &mx::ShaderPort::getUnit, "Return the unit type for the value on this port.")
        .def("setColorSpace", &mx::ShaderPort::setColorSpace, "Set the element's color space string.")
        .def("getColorSpace", &mx::ShaderPort::getColorSpace, "Return the element's color space string.")
        .def("isUniform", &mx::ShaderPort::isUniform, "Return the uniform flag on this port.")
        .def("isEmitted", &mx::ShaderPort::isEmitted, "Return the emitted state of this port.");
}
