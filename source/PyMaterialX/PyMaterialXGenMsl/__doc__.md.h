//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// Docstring for the PyMaterialXGenMsl module

#define PyMaterialXGenMsl_DOCSTRING PYMATERIALX_DOCSTRING(R"docstring(
Shader generation using the Metal Shading Language.

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
