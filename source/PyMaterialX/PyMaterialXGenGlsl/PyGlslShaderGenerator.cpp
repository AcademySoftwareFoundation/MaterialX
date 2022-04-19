//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenGlsl/GlslResourceBindingContext.h>
#include <MaterialXGenGlsl/EsslShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

// GLSL shader generator bindings

void bindPyGlslShaderGenerator(py::module& mod)
{
    py::class_<mx::GlslShaderGenerator, mx::HwShaderGenerator, mx::GlslShaderGeneratorPtr>(mod, "GlslShaderGenerator")
        .def_static("create", &mx::GlslShaderGenerator::create)
        .def(py::init<>())
        .def("generate", &mx::GlslShaderGenerator::generate)
        .def("getTarget", &mx::GlslShaderGenerator::getTarget)
        .def("getVersion", &mx::GlslShaderGenerator::getVersion);
}

void bindPyGlslResourceBindingContext(py::module &mod)
{
    py::class_<mx::GlslResourceBindingContext, mx::HwResourceBindingContext, mx::GlslResourceBindingContextPtr>(mod, "GlslResourceBindingContext")
        .def_static("create", &mx::GlslResourceBindingContext::create)
        .def(py::init<size_t, size_t>())
        .def("emitDirectives", &mx::GlslResourceBindingContext::emitDirectives)
        .def("emitResourceBindings", &mx::GlslResourceBindingContext::emitResourceBindings);
}

// Essl shader generator bindings

void bindPyEsslShaderGenerator(py::module& mod)
{
    py::class_<mx::EsslShaderGenerator, mx::HwShaderGenerator, mx::EsslShaderGeneratorPtr>(mod, "EsslShaderGenerator")
        .def_static("create", &mx::EsslShaderGenerator::create)
        .def(py::init<>())
        .def("generate", &mx::EsslShaderGenerator::generate)
        .def("getTarget", &mx::EsslShaderGenerator::getTarget)
        .def("getVersion", &mx::EsslShaderGenerator::getVersion);
}
