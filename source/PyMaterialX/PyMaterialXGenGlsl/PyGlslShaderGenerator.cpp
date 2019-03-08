//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGlslShaderGenerator(py::module& mod)
{
    py::class_<mx::GlslShaderGenerator, mx::HwShaderGenerator, mx::GlslShaderGeneratorPtr>(mod, "GlslShaderGenerator")
        .def_static("create", &mx::GlslShaderGenerator::create)
        .def(py::init<>())
        .def("generate", &mx::GlslShaderGenerator::generate)
        .def("getLanguage", &mx::GlslShaderGenerator::getLanguage)
        .def("getTarget", &mx::GlslShaderGenerator::getTarget)
        .def("getVersion", &mx::GlslShaderGenerator::getVersion);
}
