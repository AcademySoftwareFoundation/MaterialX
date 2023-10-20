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
    mod.doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Shader generation using Metal Shading Language.

    :see: https://developer.apple.com/metal/
    :see: https://developer.apple.com/documentation/metal

    MSL Shader Generation Classes
    -----------------------------

    **Class Hierarchy**

    * `PyMaterialXGenShader.ShaderGenerator`
        * `PyMaterialXGenShader.HwShaderGenerator`
            * `MslShaderGenerator`
    * `PyMaterialXGenShader.GenUserData`
        * `PyMaterialXGenShader.HwResourceBindingContext`
            * `MslResourceBindingContext`

    **Class Index**

    .. autosummary::
        :toctree: msl-shader-generators

        MslShaderGenerator
        MslResourceBindingContext
)docstring");

    // PyMaterialXGenMsl depends on types defined in PyMaterialXGenShader
    PYMATERIALX_IMPORT_MODULE(PyMaterialXGenShader);

    bindPyMslShaderGenerator(mod);
    bindPyMslResourceBindingContext(mod);
}
