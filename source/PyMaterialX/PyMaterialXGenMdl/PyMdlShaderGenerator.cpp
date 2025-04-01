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
    py::class_<mx::MdlShaderGenerator, mx::ShaderGenerator, mx::MdlShaderGeneratorPtr>(mod, "MdlShaderGenerator")
        .def_static("create", &MdlShaderGenerator_create)
        .def("getTarget", &mx::MdlShaderGenerator::getTarget);
}
