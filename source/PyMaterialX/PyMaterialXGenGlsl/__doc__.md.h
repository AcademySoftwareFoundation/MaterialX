//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// Docstring for the PyMaterialXGenGlsl module

#define PyMaterialXGenGlsl_DOCSTRING PYMATERIALX_DOCSTRING(R"docstring(
Shader generation using the OpenGL Shading Language.

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
