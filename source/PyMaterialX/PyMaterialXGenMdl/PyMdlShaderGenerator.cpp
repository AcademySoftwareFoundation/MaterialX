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
        .def_static("create", &mx::MdlShaderGenerator::create, py::arg("typeSystem") = mx::TypeSystem::create())
        .def(py::init<mx::TypeSystemPtr>(), py::arg("typeSystem") = mx::TypeSystem::create())
        .def("getTarget", &mx::MdlShaderGenerator::getTarget);
}
