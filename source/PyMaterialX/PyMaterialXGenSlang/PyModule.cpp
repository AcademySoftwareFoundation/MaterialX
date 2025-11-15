//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPySlangShaderGenerator(py::module& mod);

PYBIND11_MODULE(PyMaterialXGenSlang, mod)
{
    mod.doc() = "Shader generation using the Slang shading language.";

    // PyMaterialXGenSlang depends on types defined in PyMaterialXGenShader
    PYMATERIALX_IMPORT_MODULE(PyMaterialXGenShader);

    bindPySlangShaderGenerator(mod);
}
