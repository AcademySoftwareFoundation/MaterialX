//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShaderGenerator(py::module& mod)
{
    py::class_<mx::ShaderGenerator, mx::ShaderGeneratorPtr>(mod, "ShaderGenerator")
        .def("getTarget", &mx::ShaderGenerator::getTarget)
        .def("generate", &mx::ShaderGenerator::generate)
        .def("setColorManagementSystem", &mx::ShaderGenerator::setColorManagementSystem)
        .def("getColorManagementSystem", &mx::ShaderGenerator::getColorManagementSystem)
        .def("setUnitSystem", &mx::ShaderGenerator::setUnitSystem)
        .def("getUnitSystem", &mx::ShaderGenerator::getUnitSystem);
}
