//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyHlslShaderGenerator(py::module& mod);
void bindPyHlslResourceBindingContext(py::module& mod);

PYBIND11_MODULE(PyMaterialXGenHlsl, mod)
{
    mod.doc() = "Shader generation using the HLSL shading language.";

    // PyMaterialXGenHlsl depends on types defined in PyMaterialXGenShader
    PYMATERIALX_IMPORT_MODULE(PyMaterialXGenShader);

    bindPyHlslShaderGenerator(mod);
    bindPyHlslResourceBindingContext(mod);
}
