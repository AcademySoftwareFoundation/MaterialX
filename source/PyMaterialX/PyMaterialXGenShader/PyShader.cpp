//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/Shader.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShader(py::module& mod)
{
    // Note: py::return_value_policy::reference was needed because getStage returns a
    // ShaderStage& and without this parameter it would return a copy and not a
    // reference
    py::class_<mx::Shader, mx::ShaderPtr>(mod, "Shader", "Class containing all data needed during shader generation.\n\nAfter generation is completed it will contain the resulting source code emitted by shader generators.\n\nThe class contains a default implementation using a single shader stage. Derived shaders can override this, as well as overriding all methods that add code to the shader.")
        .def(py::init<const std::string&, mx::ShaderGraphPtr>())
        .def("getName", &mx::Shader::getName, "Return the ColorManagementSystem name.")
        .def("hasStage", &mx::Shader::hasStage, "Return if stage exists.")
        .def("numStages", &mx::Shader::numStages, "Return the number of shader stages for this shader.")
        .def("getStage", static_cast<mx::ShaderStage& (mx::Shader::*)(size_t)>(&mx::Shader::getStage), py::return_value_policy::reference, "Return a stage by name.")
        .def("getStage", static_cast<mx::ShaderStage& (mx::Shader::*)(const std::string&)>(&mx::Shader::getStage), py::return_value_policy::reference, "Return a stage by name.")
        .def("getSourceCode", &mx::Shader::getSourceCode, "Return the shader source code for a given shader stage.")
        .def("hasAttribute", &mx::Shader::hasAttribute, "Return true if the given attribute is present.")
        .def("getAttribute", &mx::Shader::getAttribute, "Return the value string of the given attribute.\n\nIf the given attribute is not present, then an empty string is returned.")
        .def("setAttribute", static_cast<void (mx::Shader::*)(const std::string&)>(&mx::Shader::setAttribute), "Set the value string of the given attribute.")
        .def("setAttribute", static_cast<void (mx::Shader::*)(const std::string&, mx::ValuePtr)>(&mx::Shader::setAttribute), "Set the value string of the given attribute.");
}
