//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyOslRenderer(py::module& mod);

PYBIND11_MODULE(PyMaterialXRenderOsl, mod)
{
    mod.doc() = "Rendering materials using Open Shading Language";

    bindPyOslRenderer(mod);
}
