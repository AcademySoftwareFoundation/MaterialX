//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderGlsl/GlslRenderer.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGlslRenderer(py::module& mod)
{
    py::class_<mx::GlslRenderer, mx::ShaderRenderer, mx::GlslRendererPtr>(mod, "GlslRenderer")

        .def_static("create", &mx::GlslRenderer::create,
                    py::arg("width") = 512,
                    py::arg("height") = 512,
                    py::arg_v("baseType",
                              mx::Image::BaseType::UINT8,
                              "BaseType.UINT8"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a GLSL renderer instance for rendering an image of the specified
    resolution.
)docstring"))

        .def("initialize", &mx::GlslRenderer::initialize,
             py::arg("renderContextHandle") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize the renderer.

    Takes care of the internal initialization of stages and OpenGL constructs
    required for program validation and rendering.

    :param renderContextHandle: Allows initializing the GlslRenderer with a
        Shared OpenGL Context.
    :type renderContextHandle: RenderContextHandle
    :raises Exception: In case of initialization errors, listing them.
)docstring"))

        .def("createProgram",
             static_cast<void (mx::GlslRenderer::*)(const mx::ShaderPtr)>(&mx::GlslRenderer::createProgram),
             py::arg("shader"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create GLSL program based on an input shader

    :param shader: Input `HwShader`.
    :type shader: HwShader
)docstring"))

        .def("createProgram",
             static_cast<void (mx::GlslRenderer::*)(const mx::GlslRenderer::StageMap&)>(&mx::GlslRenderer::createProgram),
             py::arg("stages"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create GLSL program based on shader stage source code.

    :param stages: Map of name and source code for the shader stages.
    :type stages: Dict[str, str]
)docstring"))

        .def("validateInputs", &mx::GlslRenderer::validateInputs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Validate inputs for the program.
)docstring"))

        .def("render", &mx::GlslRenderer::render,
             PYMATERIALX_DOCSTRING(R"docstring(
    Render the current program to an offscreen buffer.
)docstring"))

        .def("renderTextureSpace", &mx::GlslRenderer::renderTextureSpace,
             py::arg("uvMin"),
             py::arg("uvMax"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Render the current program in texture space to an off-screen buffer.
)docstring"))

        .def("captureImage", &mx::GlslRenderer::captureImage,
             py::arg("image") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Capture the current contents of the off-screen hardware buffer as an image.
)docstring"))

        .def("getProgram", &mx::GlslRenderer::getProgram,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the GLSL program.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Helper class for rendering generated GLSL code to produce images.

    There are two main interfaces which can be used:
        - One which takes in a `HwShader`, and
        - One which allows for explicit setting of shader stage code.

    :see: `createProgram()`

    The main services provided are:
        - Validation: All shader stages are compiled and attached to a GLSL
          shader program.
        - Introspection: The compiled shader program is examined for uniforms
          and attributes.
        - Binding: Uniforms and attributes which match the predefined variables
          generated the GLSL code generator will have values assigned to this.
          This includes matrices, attribute streams, and textures.
        - Rendering: The program with bound inputs will be used to drawing
          geometry to an offscreen buffer.
          An interface is provided to save this offscreen buffer to disk using
          an externally defined image handler.

    :see: https://materialx.org/docs/api/class_glsl_renderer.html
)docstring");
}
