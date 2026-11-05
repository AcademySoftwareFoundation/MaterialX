//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenHlsl/HlslResourceBindingContext.h>
#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

// HLSL shader generator bindings

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in python
    mx::ShaderGeneratorPtr HlslShaderGenerator_create()
    {
        return mx::HlslShaderGenerator::create();
    }
}

void bindPyHlslShaderGenerator(py::module& mod)
{
    py::class_<mx::HlslShaderGenerator, mx::HwShaderGenerator, mx::HlslShaderGeneratorPtr>(mod, "HlslShaderGenerator")
        .def_static("create", &HlslShaderGenerator_create)
        .def("generate", &mx::HlslShaderGenerator::generate)
        .def("getTarget", &mx::HlslShaderGenerator::getTarget)
        .def("getVersion", &mx::HlslShaderGenerator::getVersion);
}

void bindPyHlslResourceBindingContext(py::module& mod)
{
    py::class_<mx::HlslResourceBindingContext, mx::HwResourceBindingContext, mx::HlslResourceBindingContextPtr>(mod, "HlslResourceBindingContext")
        .def_static("create", &mx::HlslResourceBindingContext::create,
                    py::arg("cbufferRegister") = 0,
                    py::arg("textureRegister") = 0,
                    py::arg("samplerRegister") = 0)
        .def(py::init<size_t, size_t, size_t>(),
             py::arg("cbufferRegister") = 0,
             py::arg("textureRegister") = 0,
             py::arg("samplerRegister") = 0)
        .def("emitDirectives", &mx::HlslResourceBindingContext::emitDirectives)
        .def("emitResourceBindings", &mx::HlslResourceBindingContext::emitResourceBindings)
        .def("setRegisterSpace", &mx::HlslResourceBindingContext::setRegisterSpace)
        .def("getRegisterSpace", &mx::HlslResourceBindingContext::getRegisterSpace);
}
