//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenEssl/EsslShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyEsslShaderGenerator(py::module& mod)
{
    py::class_<mx::EsslShaderGenerator, mx::HwShaderGenerator, mx::EsslShaderGeneratorPtr>(mod, "EsslShaderGenerator")
        .def_static("create", &mx::EsslShaderGenerator::create)
        .def(py::init<>())
        .def("generate", &mx::EsslShaderGenerator::generate)
        .def("getTarget", &mx::EsslShaderGenerator::getTarget)
        .def("getVersion", &mx::EsslShaderGenerator::getVersion);
}
