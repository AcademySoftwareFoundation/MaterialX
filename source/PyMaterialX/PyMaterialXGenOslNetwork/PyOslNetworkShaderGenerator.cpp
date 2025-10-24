//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenOsl/OslNetworkShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

namespace py = pybind11;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in python
    mx::ShaderGeneratorPtr OslNetworkShaderGenerator_create()
    {
        return mx::OslNetworkShaderGenerator::create();
    }
}

void bindPyOslNetworkShaderGenerator(py::module& mod)
{
    mod.attr("OSL_UNIFORMS") = mx::OSLNetwork::UNIFORMS;
    mod.attr("OSL_INPUTS") = mx::OSLNetwork::INPUTS;
    mod.attr("OSL_OUTPUTS") = mx::OSLNetwork::OUTPUTS;

    py::class_<mx::OslNetworkShaderGenerator, mx::ShaderGenerator, mx::OslNetworkShaderGeneratorPtr>(mod, "OslNetworkShaderGenerator")
        .def_static("create", &OslNetworkShaderGenerator_create)
        .def("getTarget", &mx::OslNetworkShaderGenerator::getTarget)
        .def("generate", &mx::OslNetworkShaderGenerator::generate);
}
