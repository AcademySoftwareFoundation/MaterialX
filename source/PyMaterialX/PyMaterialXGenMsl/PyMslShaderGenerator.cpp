//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenMsl/MslShaderGenerator.h>
#include <MaterialXGenMsl/MslResourceBindingContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

// MSL shader generator bindings

void bindPyMslShaderGenerator(py::module& mod)
{
    py::class_<mx::MslShaderGenerator, mx::HwShaderGenerator, mx::MslShaderGeneratorPtr>(mod, "MslShaderGenerator")

        .def_static("create", &mx::MslShaderGenerator::create,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class.
)docstring"))

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .def("generate", &mx::MslShaderGenerator::generate,
             py::arg("name"),
             py::arg("element"),
             py::arg("context"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Generate a shader starting from the given `element`, translating the
    element and all dependencies upstream into shader code.
)docstring"))

        .def("getTarget", &mx::MslShaderGenerator::getTarget,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a unique identifier for the target this generator is for.
)docstring"))

        .def("getVersion", &mx::MslShaderGenerator::getVersion,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the version string for the MSL version this generator is for.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for MSL (Metal Shading Language) code generation.
    A generator for a specific MSL target should be derived from this class.
)docstring");
}

void bindPyMslResourceBindingContext(py::module &mod)
{
    py::class_<mx::MslResourceBindingContext, mx::HwResourceBindingContext, mx::MslResourceBindingContextPtr>(mod, "MslResourceBindingContext")

        .def_static("create", &mx::MslResourceBindingContext::create,
                    py::arg("uniformBindingLocation"),
                    py::arg("samplerBindingLocation"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class, initialized using the given binding
    locations.
)docstring"))

        .def(py::init<size_t, size_t>(),
             py::arg("uniformBindingLocation"),
             py::arg("samplerBindingLocation"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class using the given binding locations.
)docstring"))

        .def("emitDirectives", &mx::MslResourceBindingContext::emitDirectives,
             py::arg("context"),
             py::arg("stage"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Emit directives required for binding support.
)docstring"))

        .def("emitResourceBindings", &mx::MslResourceBindingContext::emitResourceBindings,
             py::arg("context"),
             py::arg("uniforms"),
             py::arg("stage"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Emit uniforms with binding information.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a resource binding for MSL shader resources.
)docstring");
}
