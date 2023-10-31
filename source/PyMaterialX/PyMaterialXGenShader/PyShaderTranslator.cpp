//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderTranslator.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShaderTranslator(py::module& mod)
{
    py::class_<mx::ShaderTranslator, mx::ShaderTranslatorPtr>(mod, "ShaderTranslator")
 
        .def_static("create", &mx::ShaderTranslator::create,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class.
)docstring"))

        .def("translateShader", &mx::ShaderTranslator::translateShader,
             py::arg("shader"),
             py::arg("destCategory"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Translate a shader node to the destination shading model.
)docstring"))

        .def("translateAllMaterials", &mx::ShaderTranslator::translateAllMaterials,
             py::arg("doc"),
             py::arg("destShader"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Translate each material in the input document to the destination shading
    model.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A helper class for translating content between shading models.

    :see: https://materialx.org/docs/api/class_shader_translator.html
)docstring");
}
