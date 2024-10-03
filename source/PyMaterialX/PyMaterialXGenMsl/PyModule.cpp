//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyMslShaderGenerator(py::module& mod);
void bindPyMslResourceBindingContext(py::module &mod);

PYBIND11_MODULE(PyMaterialXGenMsl, mod)
{
    mod.doc() = "Shader generation using the Metal Shading Language.";

    // PyMaterialXGenMsl depends on types defined in PyMaterialXGenShader
    PYMATERIALX_IMPORT_MODULE(PyMaterialXGenShader);

    bindPyMslShaderGenerator(mod);
    bindPyMslResourceBindingContext(mod);
}
