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
        .def_static("create", &mx::GlslRenderer::create)
        .def("initialize", &mx::GlslRenderer::initialize, py::arg("renderContextHandle") = nullptr)
        .def("createProgram", static_cast<void (mx::GlslRenderer::*)(const mx::ShaderPtr)>(&mx::GlslRenderer::createProgram))
        .def("createProgram", static_cast<void (mx::GlslRenderer::*)(const mx::GlslRenderer::StageMap&)>(&mx::GlslRenderer::createProgram))
        .def("validateInputs", &mx::GlslRenderer::validateInputs)
        .def("render", &mx::GlslRenderer::render)
        .def("renderTextureSpace", &mx::GlslRenderer::renderTextureSpace)
        .def("captureImage", &mx::GlslRenderer::captureImage)
        .def("getProgram", &mx::GlslRenderer::getProgram);
    mod.attr("GlslRenderer").doc() = R"docstring(
    Helper class for rendering generated GLSL code to produce images.

    There are two main interfaces which can be used:
    one which takes in a `HwShader` and
    one which allows for explicit setting of shader stage code.

    The main services provided are:
        - Validation: All shader stages are compiled and atteched to a GLSL shader program.
        - Introspection: The compiled shader program is examined for uniforms and attributes.
        - Binding: Uniforms and attributes which match the predefined variables generated the GLSL code generator
          will have values assigned to this. This includes matrices, attribute streams, and textures.
        - Rendering: The program with bound inputs will be used to drawing geometry to an offscreen buffer.
          An interface is provided to save this offscreen buffer to disk using an externally defined image handler.

    :see: https://materialx.org/docs/api/class_glsl_renderer.html)docstring";
}
