//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;


void bindPyMdlShaderGenerator(py::module& mod)
{
    py::class_<mx::MdlShaderGenerator, mx::ShaderGenerator, mx::MdlShaderGeneratorPtr>(mod, "MdlShaderGenerator")
        .def_static("create", &mx::MdlShaderGenerator::create)
        .def(py::init<>())
        .def("getTarget", &mx::MdlShaderGenerator::getTarget);
}
