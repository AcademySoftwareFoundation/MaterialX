//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyGlslShaderGenerator(py::module& mod);
void bindPyGlslResourceBindingContext(py::module &mod);
void bindPyEsslShaderGenerator(py::module& mod);
void bindPyVkShaderGenerator(py::module& mod);

PYBIND11_MODULE(PyMaterialXGenGlsl, mod)
{
    mod.doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Shader generation using OpenGL Shading Language.

    :see: https://www.opengl.org
    :see: https://www.vulkan.org

    GLSL Shader Generation Classes
    ------------------------------

    **Class Hierarchy**

    * `PyMaterialXGenShader.ShaderGenerator`
        * `PyMaterialXGenShader.HwShaderGenerator`
            * `GlslShaderGenerator`
                * `EsslShaderGenerator`
                * `VkShaderGenerator`
    * `PyMaterialXGenShader.GenUserData`
        * `PyMaterialXGenShader.HwResourceBindingContext`
            * `GlslResourceBindingContext`

    **Class Index**

    .. autosummary::
        :toctree: glsl-shader-generators

        GlslShaderGenerator
        EsslShaderGenerator
        VkShaderGenerator
        GlslResourceBindingContext
)docstring");

    // PyMaterialXGenGlsl depends on types defined in PyMaterialXGenShader
    PYMATERIALX_IMPORT_MODULE(PyMaterialXGenShader);

    bindPyGlslShaderGenerator(mod);
    bindPyGlslResourceBindingContext(mod);

    bindPyEsslShaderGenerator(mod);
    bindPyVkShaderGenerator(mod);
}
