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
    mod.doc() = "Shader generation using Metal Shading Language";

    bindPyMslShaderGenerator(mod);
    bindPyMslResourceBindingContext(mod);
}
