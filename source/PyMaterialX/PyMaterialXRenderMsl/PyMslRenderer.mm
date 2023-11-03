//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderMsl/MslRenderer.h>
#include <MaterialXRenderMsl/MetalFramebuffer.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyMslRenderer(py::module& mod)
{
    py::class_<mx::MslRenderer, mx::ShaderRenderer, mx::MslRendererPtr>(mod, "MslRenderer")

        .def_static("create", &mx::MslRenderer::create,
                    py::arg("width") = 512,
                    py::arg("height") = 512,
                    py::arg_v("baseType",
                              mx::Image::BaseType::UINT8,
                              "BaseType.UINT8"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a MSL renderer instance for rendering an image of the specified
    resolution.
)docstring"))

        .def("initialize", &mx::MslRenderer::initialize,
             py::arg("renderContextHandle") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize the renderer.

    Takes care of the internal initialization of stages and MSL constructs
    required for program validation and rendering.

    :param renderContextHandle: Not currently used by an MSL renderer.
    :type renderContextHandle: RenderContextHandle
    :raises Exception: In case of initialization errors, listing them.
)docstring"))

        .def("createProgram",
             static_cast<void (mx::MslRenderer::*)(const mx::ShaderPtr)>(&mx::MslRenderer::createProgram),
             py::arg("shader"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create MSL program based on an input shader

    :param shader: Input `HwShader`.
    :type shader: HwShader
)docstring"))

        .def("createProgram",
             static_cast<void (mx::MslRenderer::*)(const mx::MslRenderer::StageMap&)>(&mx::MslRenderer::createProgram),
             py::arg("stages"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create MSL program based on shader stage source code.

    :param stages: Map of name and source code for the shader stages.
    :type stages: Dict[str, str]
)docstring"))

        .def("validateInputs", &mx::MslRenderer::validateInputs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Validate inputs for the program.
)docstring"))

        .def("render", &mx::MslRenderer::render,
             PYMATERIALX_DOCSTRING(R"docstring(
    Render the current program to an offscreen buffer.
)docstring"))

        .def("renderTextureSpace", &mx::MslRenderer::renderTextureSpace,
             py::arg("uvMin"),
             py::arg("uvMax"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Render the current program in texture space to an off-screen buffer.
)docstring"))

        .def("captureImage", &mx::MslRenderer::captureImage,
             py::arg("image"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Capture the current contents of the off-screen hardware buffer as an image.
)docstring"))

        .def("getProgram", &mx::MslRenderer::getProgram,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the MSL program.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Helper class for rendering generated MSL code to produce images.

    There are two main interfaces which can be used: one which takes in a
    `HwShader`, and one which allows for explicit setting of shader stage code.

    The main services provided are:
        - Validation: All shader stages are compiled and atteched to an MSL
          shader program.
        - Introspection: The compiled shader program is examined for uniforms
          and attributes.
        - Binding: Uniforms and attributes which match the predefined variables
          generated the MSL code generator will have values assigned to this.
          This includes matrices, attribute streams, and textures.
        - Rendering: The program with bound inputs will be used to drawing
          geometry to an offscreen buffer.
          An interface is provided to save this offscreen buffer to disk using
          an externally defined image handler.
)docstring");
}
