//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyOslShaderGenerator(py::module& mod)
{
    mod.attr("OSL_UNIFORMS") = mx::OSL::UNIFORMS;
    mod.attr("OSL_INPUTS") = mx::OSL::INPUTS;
    mod.attr("OSL_OUTPUTS") = mx::OSL::OUTPUTS;

    py::class_<mx::OslShaderGenerator, mx::ShaderGenerator, mx::OslShaderGeneratorPtr>(mod, "OslShaderGenerator")
        .def_static("create", &mx::OslShaderGenerator::create)
        .def(py::init<>())
        .def("getTarget", &mx::OslShaderGenerator::getTarget)
        .def("generate", &mx::OslShaderGenerator::generate);
}
