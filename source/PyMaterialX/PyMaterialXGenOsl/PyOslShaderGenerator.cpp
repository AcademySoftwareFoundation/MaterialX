//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>

namespace py = pybind11;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in python
    mx::ShaderGeneratorPtr OslShaderGenerator_create()
    {
        return mx::OslShaderGenerator::create();
    }
}

void bindPyOslShaderGenerator(py::module& mod)
{
    mod.attr("OSL_UNIFORMS") = mx::OSL::UNIFORMS;
    mod.attr("OSL_INPUTS") = mx::OSL::INPUTS;
    mod.attr("OSL_OUTPUTS") = mx::OSL::OUTPUTS;

    py::class_<mx::OslShaderGenerator, mx::ShaderGenerator, mx::OslShaderGeneratorPtr>(mod, "OslShaderGenerator", "Base class for OSL (Open Shading Language) shader generators.\n\nA generator for a specific OSL target should be derived from this class.")
        .def_static("create", &OslShaderGenerator_create, "Creator function.\n\nIf a TypeSystem is not provided it will be created internally. Optionally pass in an externally created TypeSystem here, if you want to keep type descriptions alive after the lifetime of the shader generator.")
        .def("getTarget", &mx::OslShaderGenerator::getTarget, "Return a unique identifier for the target this generator is for.")
        .def("generate", &mx::OslShaderGenerator::generate, "Generate a shader starting from the given element, translating the element and all dependencies upstream into shader code.");
}
