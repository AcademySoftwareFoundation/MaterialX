//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyMdlShaderGenerator(py::module& mod)
{
    py::class_<mx::MdlShaderGenerator, mx::ShaderGenerator, mx::MdlShaderGeneratorPtr>(mod, "MdlShaderGenerator")
        .def_static("create", &mx::MdlShaderGenerator::create)
        .def(py::init<>())
        .def("getTarget", &mx::MdlShaderGenerator::getTarget);
    mod.attr("MdlShaderGenerator").doc() = R"docstring(
    Shader generator for MDL (Material Definition Language).

    :see: https://materialx.org/docs/api/class_mdl_shader_generator.html)docstring";
}
