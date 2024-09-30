//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// Docstring for the PyMaterialXRenderGlsl module

#define PyMaterialXRenderGlsl_DOCSTRING PYMATERIALX_DOCSTRING(R"docstring(
Rendering materials using the OpenGL Shading Language.

:see: https://www.opengl.org
:see: https://www.vulkan.org

GLSL Rendering Classes
----------------------

**Class Hierarchy**

* `PyMaterialXRender.ShaderRenderer`
    * `GlslRenderer`
        * `TextureBaker`
* `PyMaterialXRender.ImageHandler`
    * `GLTextureHandler`

**Class Index**

.. autosummary::
    :toctree: glsl-rendering

    GlslRenderer
    GlslProgram
    GLTextureHandler
    Input
    TextureBaker
)docstring");
