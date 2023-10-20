//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyOslRenderer(py::module& mod);

PYBIND11_MODULE(PyMaterialXRenderOsl, mod)
{
    mod.doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Rendering materials using Open Shading Language.

    :see: https://openshadinglanguage.org/
    :see: https://open-shading-language.readthedocs.io/

    OSL Rendering Classes
    ---------------------

    .. autosummary::
        :toctree: osl-rendering

        OslRenderer
)docstring");

    // PyMaterialXRenderOsl depends on types defined in PyMaterialXRender
    PYMATERIALX_IMPORT_MODULE(PyMaterialXRender);

    bindPyOslRenderer(mod);
}
