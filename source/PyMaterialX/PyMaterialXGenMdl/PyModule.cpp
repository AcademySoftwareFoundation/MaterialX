//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyMdlShaderGenerator(py::module& mod);

PYBIND11_MODULE(PyMaterialXGenMdl, mod)
{
    mod.doc() = R"docstring(
    Shader generation using Material Definition Language.

    :see: https://www.nvidia.com/en-us/design-visualization/technologies/material-definition-language/
    :see: https://raytracing-docs.nvidia.com/mdl/index.html

    MDL Shader Generation Classes
    -----------------------------

    **Class Hierarchy**

    * `PyMaterialXGenShader.ShaderGenerator`
        * `MdlShaderGenerator`

    **Class Index**

    .. autosummary::
        :toctree: mdl-shader-generators

        MdlShaderGenerator
)docstring";

    // PyMaterialXGenMdl depends on types defined in PyMaterialXGenShader
    pybind11::module::import("PyMaterialXGenShader");

    bindPyMdlShaderGenerator(mod);
};
