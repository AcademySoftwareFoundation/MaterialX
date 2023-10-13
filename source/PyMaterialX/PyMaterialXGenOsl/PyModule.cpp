//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyOslShaderGenerator(py::module& mod);

PYBIND11_MODULE(PyMaterialXGenOsl, mod)
{
    mod.doc() = "Shader generation using Open Shading Language";

    bindPyOslShaderGenerator(mod);
}
