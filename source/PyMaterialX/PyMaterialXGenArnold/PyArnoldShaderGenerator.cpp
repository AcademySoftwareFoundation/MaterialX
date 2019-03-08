//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenArnold/ArnoldShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;


void bindPyArnoldShaderGenerator(py::module& mod)
{
    py::class_<mx::ArnoldShaderGenerator, mx::OslShaderGenerator, mx::ArnoldShaderGeneratorPtr>(mod, "ArnoldShaderGenerator")
        .def_static("create", &mx::ArnoldShaderGenerator::create)
        .def(py::init<>())
        .def("getTarget", &mx::ArnoldShaderGenerator::getTarget);
}
