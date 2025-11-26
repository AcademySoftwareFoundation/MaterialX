//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in python
    mx::ShaderGeneratorPtr MdlShaderGenerator_create()
    {
        return mx::MdlShaderGenerator::create();
    }
}

void bindPyMdlShaderGenerator(py::module& mod)
{
    py::class_<mx::MdlShaderGenerator, mx::ShaderGenerator, mx::MdlShaderGeneratorPtr>(mod, "MdlShaderGenerator", "Shader generator for MDL (Material Definition Language).")
        .def_static("create", &MdlShaderGenerator_create, "Creator function.\n\nIf a TypeSystem is not provided it will be created internally. Optionally pass in an externally created TypeSystem here, if you want to keep type descriptions alive after the lifetime of the shader generator.")
        .def("getTarget", &mx::MdlShaderGenerator::getTarget, "Return a unique identifier for the target this generator is for.");
}
