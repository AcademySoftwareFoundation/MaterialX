//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenWgsl/WgslShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

namespace
{
    mx::ShaderGeneratorPtr WgslShaderGenerator_create()
    {
        return mx::WgslShaderGenerator::create();
    }
}

void bindPyWgslShaderGenerator(py::module& mod)
{
    py::class_<mx::WgslShaderGenerator, mx::HwShaderGenerator, mx::WgslShaderGeneratorPtr>(mod, "WgslShaderGenerator")
        .def_static("create", &WgslShaderGenerator_create)
        .def("generate", &mx::WgslShaderGenerator::generate)
        .def("getTarget", &mx::WgslShaderGenerator::getTarget)
        .def("getVersion", &mx::WgslShaderGenerator::getVersion);
}
