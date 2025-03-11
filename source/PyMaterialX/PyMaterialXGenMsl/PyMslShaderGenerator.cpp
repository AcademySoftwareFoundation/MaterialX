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

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in python
    mx::ShaderGeneratorPtr MslShaderGenerator_create()
    {
        return mx::MslShaderGenerator::create();
    }
}

void bindPyMslShaderGenerator(py::module& mod)
{
    py::class_<mx::MslShaderGenerator, mx::HwShaderGenerator, mx::MslShaderGeneratorPtr>(mod, "MslShaderGenerator")
        .def_static("create", &MslShaderGenerator_create)
        .def("generate", &mx::MslShaderGenerator::generate)
        .def("getTarget", &mx::MslShaderGenerator::getTarget)
        .def("getVersion", &mx::MslShaderGenerator::getVersion);
}

void bindPyMslResourceBindingContext(py::module &mod)
{
    py::class_<mx::MslResourceBindingContext, mx::HwResourceBindingContext, mx::MslResourceBindingContextPtr>(mod, "MslResourceBindingContext")
        .def_static("create", &mx::MslResourceBindingContext::create)
        .def(py::init<size_t, size_t>())
        .def("emitDirectives", &mx::MslResourceBindingContext::emitDirectives)
        .def("emitResourceBindings", &mx::MslResourceBindingContext::emitResourceBindings);
}
