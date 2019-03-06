//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//
#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/HwShader.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyHwShader(py::module& mod)
{
    py::class_<mx::HwShader, mx::Shader, mx::HwShaderPtr>(mod, "HwShader")
        .def("getVertexDataBlock", &mx::HwShader::getVertexDataBlock)
        .def("hasTransparency", &mx::HwShader::hasTransparency);
}
