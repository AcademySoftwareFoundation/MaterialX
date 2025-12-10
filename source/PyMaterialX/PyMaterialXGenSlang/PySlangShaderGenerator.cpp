//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenSlang/SlangShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

// Slang shader generator bindings

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in python
    mx::ShaderGeneratorPtr SlangShaderGenerator_create()
    {
        return mx::SlangShaderGenerator::create();
    }
}

void bindPySlangShaderGenerator(py::module& mod)
{
    py::class_<mx::SlangShaderGenerator, mx::HwShaderGenerator, mx::SlangShaderGeneratorPtr>(mod, "SlangShaderGenerator")
        .def_static("create", &SlangShaderGenerator_create)
        .def("generate", &mx::SlangShaderGenerator::generate)
        .def("getTarget", &mx::SlangShaderGenerator::getTarget)
        .def("getVersion", &mx::SlangShaderGenerator::getVersion);
}
