//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyOslShaderGenerator(py::module& mod);

PYBIND11_MODULE(PyMaterialXGenOsl, mod)
{
    mod.doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Shader generation using Open Shading Language.

    :see: https://openshadinglanguage.org/
    :see: https://open-shading-language.readthedocs.io/

    OSL Shader Generation Classes
    -----------------------------

    **Class Hierarchy**

    * `PyMaterialXGenShader.ShaderGenerator`
        * `OslShaderGenerator`

    **Class Index**

    .. autosummary::
        :toctree: osl-shader-generators

        OslShaderGenerator
)docstring");

    // PyMaterialXGenOsl depends on types defined in PyMaterialXGenShader
    PYMATERIALX_IMPORT_MODULE(PyMaterialXGenShader);

    bindPyOslShaderGenerator(mod);
}
