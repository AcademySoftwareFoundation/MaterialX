//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenMsl/MslShaderGenerator.h>
#include <MaterialXGenMsl/MslResourceBindingContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

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
        .def_static("create", &MslShaderGenerator_create, "Creator function.\n\nIf a TypeSystem is not provided it will be created internally. Optionally pass in an externally created TypeSystem here, if you want to keep type descriptions alive after the lifetime of the shader generator.")
        .def("generate", &mx::MslShaderGenerator::generate, "Generate a shader starting from the given element, translating the element and all dependencies upstream into shader code.")
        .def("getTarget", &mx::MslShaderGenerator::getTarget, "Return a unique identifier for the target this generator is for.")
        .def("getVersion", &mx::MslShaderGenerator::getVersion, "Return the version string for the GLSL version this generator is for.");
}

void bindPyMslResourceBindingContext(py::module &mod)
{
    py::class_<mx::MslResourceBindingContext, mx::HwResourceBindingContext, mx::MslResourceBindingContextPtr>(mod, "MslResourceBindingContext")
        .def_static("create", &mx::MslResourceBindingContext::create, "Creator function.\n\nIf a TypeSystem is not provided it will be created internally. Optionally pass in an externally created TypeSystem here, if you want to keep type descriptions alive after the lifetime of the shader generator.")
        .def(py::init<size_t, size_t>())
        .def("emitDirectives", &mx::MslResourceBindingContext::emitDirectives)
        .def("emitResourceBindings", &mx::MslResourceBindingContext::emitResourceBindings);
}
