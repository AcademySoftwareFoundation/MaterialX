//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// Docstring for the PyMaterialXRenderMsl module

#define PyMaterialXRenderMsl_DOCSTRING PYMATERIALX_DOCSTRING(R"docstring(
Rendering materials using the Metal Shading Language.

:see: https://developer.apple.com/metal/
:see: https://developer.apple.com/documentation/metal

Metal Rendering Classes
-----------------------

**Class Hierarchy**

* `PyMaterialXRender.ShaderRenderer`
    * `MslRenderer`
        * `TextureBaker`
* `PyMaterialXRender.ImageHandler`
    * `MetalTextureHandler`

**Class Index**

.. autosummary::
    :toctree: metal-rendering

    MslRenderer
    MslProgram
    MetalTextureHandler
    Input
    TextureBaker
)docstring");
