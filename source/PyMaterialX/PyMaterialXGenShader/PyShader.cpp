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
    py::class_<mx::Shader, mx::ShaderPtr>(mod, "Shader")

        .def(py::init<const std::string&, mx::ShaderGraphPtr>(),
             py::arg("name"),
             py::arg("graph"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class using the given name and shader graph.
)docstring"))

        .def("getName", &mx::Shader::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the shader name.
)docstring"))

        .def("hasStage", &mx::Shader::hasStage,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return whether a stage of the given `name` exists.
)docstring"))

        .def("numStages", &mx::Shader::numStages,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the number of shader stages for this shader.
)docstring"))

        .def("getStage",
             static_cast<mx::ShaderStage& (mx::Shader::*)(size_t)>(&mx::Shader::getStage),
             py::return_value_policy::reference,
             py::arg("index"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a stage by index.
)docstring"))

        .def("getStage",
             static_cast<mx::ShaderStage& (mx::Shader::*)(const std::string&)>(&mx::Shader::getStage),
             py::return_value_policy::reference,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a stage by name.
)docstring"))

        .def("getSourceCode", &mx::Shader::getSourceCode,
             py::arg_v("stage",
                       mx::Stage::PIXEL,
                       "MaterialX.PyMaterialXGenShader.PIXEL_STAGE"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the shader source code for a given shader stage.
)docstring"))

        .def("hasAttribute", &mx::Shader::hasAttribute,
             py::arg("attrib"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the shader has a given named attribute.
)docstring"))

        .def("getAttribute", &mx::Shader::getAttribute,
             py::arg("attrib"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the value for a named attribute, or `None` if no such attribute is
    found.
)docstring"))

        .def("setAttribute",
             static_cast<void (mx::Shader::*)(const std::string&)>(&mx::Shader::setAttribute),
             py::arg("attrib"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set a flag attribute on the shader.
)docstring"))

        .def("setAttribute",
             static_cast<void (mx::Shader::*)(const std::string&, mx::ValuePtr)>(&mx::Shader::setAttribute),
             py::arg("attrib"),
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set a value attribute on the shader.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class containing all data needed during shader generation.
    After generation is completed it will contain the resulting source code
    emitted by shader generators.

    The class contains a default implementation using a single shader stage.
    Derived shaders can override this, as well as overriding all methods
    that add code to the shader.

    :see: https://materialx.org/docs/api/class_shader.html
)docstring");
}
