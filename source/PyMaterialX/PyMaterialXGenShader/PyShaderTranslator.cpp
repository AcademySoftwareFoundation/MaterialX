//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderTranslator.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShaderTranslator(py::module& mod)
{
    py::class_<mx::ShaderTranslator, mx::ShaderTranslatorPtr>(mod, "ShaderTranslator")
        .def_static("create", &mx::ShaderTranslator::create)
        .def("translateShader", &mx::ShaderTranslator::translateShader)
        .def("translateAllMaterials", &mx::ShaderTranslator::translateAllMaterials);
}
