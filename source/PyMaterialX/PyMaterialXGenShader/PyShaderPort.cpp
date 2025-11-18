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
        .def("getType", &mx::ShaderPort::getType, "Return the data type for this port.")
        .def("setName", &mx::ShaderPort::setName, "Set the name of this port.")
        .def("getName", &mx::ShaderPort::getName, "Return the name of this port.")
        .def("getFullName", &mx::ShaderPort::getFullName, "Return the name of this port.")
        .def("setVariable", &mx::ShaderPort::setVariable, "Set the variable name of this port.")
        .def("getVariable", &mx::ShaderPort::getVariable, "Return the variable name of this port.")
        .def("setSemantic", &mx::ShaderPort::setSemantic, "Set the variable semantic of this port.")
        .def("getSemantic", &mx::ShaderPort::getSemantic, "Return the variable semantic of this port.")
        .def("setValue", &mx::ShaderPort::setValue, "Set a value on this port.")
        .def("getValue", &mx::ShaderPort::getValue, "Return the value set on this port.")
        .def("getValueString", &mx::ShaderPort::getValueString, "Return the value set on this port as a string, or an empty string if there is no value.")
        .def("setGeomProp", &mx::ShaderPort::setGeomProp, "Set geomprop name if the input has a default geomprop to be assigned when it is unconnected.")
        .def("getGeomProp", &mx::ShaderPort::getGeomProp, "Get geomprop name.")
        .def("setPath", &mx::ShaderPort::setPath, "Set the path to this port.")
        .def("getPath", &mx::ShaderPort::getPath, "Return the path to this port.")
        .def("setUnit", &mx::ShaderPort::setUnit, "Set a unit type for the value on this port.")
        .def("getUnit", &mx::ShaderPort::getUnit, "Return the unit type for the value on this port.")
        .def("setColorSpace", &mx::ShaderPort::setColorSpace, "Set a source color space for the value on this port.")
        .def("getColorSpace", &mx::ShaderPort::getColorSpace, "Return the source color space for the value on this port.")
        .def("isUniform", &mx::ShaderPort::isUniform, "Return the uniform flag on this port.")
        .def("isEmitted", &mx::ShaderPort::isEmitted, "Return the emitted state of this port.");
}
