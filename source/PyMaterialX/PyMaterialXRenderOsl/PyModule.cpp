//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>
#include "__doc__.md.h"

namespace py = pybind11;

void bindPyOslRenderer(py::module& mod);

PYBIND11_MODULE(PyMaterialXRenderOsl, mod)
{
    mod.doc() = PyMaterialXRenderOsl_DOCSTRING;

    // PyMaterialXRenderOsl depends on types defined in PyMaterialXRender
    PYMATERIALX_IMPORT_MODULE(PyMaterialXRender);

    bindPyOslRenderer(mod);
}
