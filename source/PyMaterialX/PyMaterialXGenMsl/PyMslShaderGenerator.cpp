//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenMsl/MslShaderGenerator.h>
#include <MaterialXGenMsl/MslResourceBindingContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

// MSL shader generator bindings

void bindPyMslShaderGenerator(py::module& mod)
{
    py::class_<mx::MslShaderGenerator, mx::HwShaderGenerator, mx::MslShaderGeneratorPtr>(mod, "MslShaderGenerator")
        .def_static("create", &mx::MslShaderGenerator::create)
        .def(py::init<>())
        .def("generate", &mx::MslShaderGenerator::generate)
        .def("getTarget", &mx::MslShaderGenerator::getTarget)
        .def("getVersion", &mx::MslShaderGenerator::getVersion);
    mod.attr("MslShaderGenerator").doc() = R"docstring(
    Base class for MSL (Metal Shading Language) code generation.

    A generator for a specific MSL target should be derived from this class.)docstring";
}

void bindPyMslResourceBindingContext(py::module &mod)
{
    py::class_<mx::MslResourceBindingContext, mx::HwResourceBindingContext, mx::MslResourceBindingContextPtr>(mod, "MslResourceBindingContext")
        .def_static("create", &mx::MslResourceBindingContext::create)
        .def(py::init<size_t, size_t>())
        .def("emitDirectives", &mx::MslResourceBindingContext::emitDirectives)
        .def("emitResourceBindings", &mx::MslResourceBindingContext::emitResourceBindings);
    mod.attr("MslResourceBindingContext").doc() = R"docstring(
    Class representing a resource binding for MSL shader resources.)docstring";
}
