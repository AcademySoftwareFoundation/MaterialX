//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenOslNodes/OslNodesShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

namespace py = pybind11;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in python
    mx::ShaderGeneratorPtr OslNodesShaderGenerator_create()
    {
        return mx::OslNodesShaderGenerator::create();
    }
}

void bindPyOslNodesShaderGenerator(py::module& mod)
{
    mod.attr("OSL_UNIFORMS") = mx::OSLNodes::UNIFORMS;
    mod.attr("OSL_INPUTS") = mx::OSLNodes::INPUTS;
    mod.attr("OSL_OUTPUTS") = mx::OSLNodes::OUTPUTS;

    py::class_<mx::OslNodesShaderGenerator, mx::ShaderGenerator, mx::OslNodesShaderGeneratorPtr>(mod, "OslNodesShaderGenerator")
        .def_static("create", &OslNodesShaderGenerator_create)
        .def("getTarget", &mx::OslNodesShaderGenerator::getTarget)
        .def("generate", &mx::OslNodesShaderGenerator::generate);
}
