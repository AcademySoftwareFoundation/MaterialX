//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyWgslShaderGenerator(py::module& mod);

PYBIND11_MODULE(PyMaterialXGenWgsl, mod)
{
    mod.doc() = "Shader generation using the WebGPU Shading Language (WGSL).";

    PYMATERIALX_IMPORT_MODULE(PyMaterialXGenShader);

    bindPyWgslShaderGenerator(mod);
}
