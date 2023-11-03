//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGenContext(py::module& mod)
{
    py::class_<mx::ApplicationVariableHandler>(mod, "ApplicationVariableHandler")

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a function to allow for handling of application
    variables for a given node.

    Is expected to take a `ShaderNode` and a `GenContext`, and return `None`.
)docstring");

    py::class_<mx::GenContext, mx::GenContextPtr>(mod, "GenContext")

        .def(py::init<mx::ShaderGeneratorPtr>(),
             py::arg("shaderGenerator"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class using the given shader generator.
)docstring"))

        .def("getShaderGenerator", &mx::GenContext::getShaderGenerator,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the context's shader generator.
)docstring"))

        .def("getOptions",
             static_cast<mx::GenOptions& (mx::GenContext::*)()>(&mx::GenContext::getOptions),
             py::return_value_policy::reference,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return shader generation options.
)docstring"))

        .def("registerSourceCodeSearchPath",
             static_cast<void (mx::GenContext::*)(const mx::FilePath&)>(&mx::GenContext::registerSourceCodeSearchPath),
             py::arg("path"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Register a user search path for finding source code during code generation.

    :see: `resolveSourceFile()`
)docstring"))

        .def("registerSourceCodeSearchPath",
             static_cast<void (mx::GenContext::*)(const mx::FileSearchPath&)>(&mx::GenContext::registerSourceCodeSearchPath),
             py::arg("path"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Register a user search path for finding source code during code generation.

    :see: `resolveSourceFile()`
)docstring"))

        .def("resolveSourceFile", &mx::GenContext::resolveSourceFile,
             py::arg("filename"),
             py::arg("localPath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Resolve a source code filename, first checking the given local path,
    then checking any file paths registered by the user.

    :see: `registerSourceCodeSearchPath()`
)docstring"))

        .def("pushUserData", &mx::GenContext::pushUserData,
             py::arg("name"),
             py::arg("data"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add user data to the context to make it available during shader generation.
)docstring"))

        .def("setApplicationVariableHandler",
             &mx::GenContext::setApplicationVariableHandler,
             py::arg("handler"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set handler for application variables.
)docstring"))

        .def("getApplicationVariableHandler",
             &mx::GenContext::getApplicationVariableHandler,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return handler for application variables.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A context class for shader generation.

    Used for thread-local storage of data needed during shader generation.

    :see: https://materialx.org/docs/api/class_gen_context.html
)docstring");
}

void bindPyGenUserData(py::module& mod)
{
    py::class_<mx::GenUserData, mx::GenUserDataPtr>(mod, "GenUserData")

        .def("getSelf",
             static_cast<mx::GenUserDataPtr(mx::GenUserData::*)()>(&mx::GenUserData::getSelf),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return this object.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for custom user data needed during shader generation.

    :see: https://materialx.org/docs/api/class_gen_user_data.html
)docstring");
}