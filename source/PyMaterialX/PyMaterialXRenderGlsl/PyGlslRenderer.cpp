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
    py::class_<mx::GlslRenderer, mx::ShaderRenderer, mx::GlslRendererPtr>(mod, "GlslRenderer", "Helper class for rendering generated GLSL code to produce images.\n\nThere are two main interfaces which can be used. One which takes in a HwShader and one which allows for explicit setting of shader stage code.\n\nThe main services provided are: Validation: All shader stages are compiled and atteched to a GLSL shader program. Introspection: The compiled shader program is examined for uniforms and attributes. Binding: Uniforms and attributes which match the predefined variables generated the GLSL code generator will have values assigned to this. This includes matrices, attribute streams, and textures. Rendering: The program with bound inputs will be used to drawing geometry to an offscreen buffer. An interface is provided to save this offscreen buffer to disk using an externally defined image handler.")
        .def_static("create", &mx::GlslRenderer::create, "Create a GLSL renderer instance.")
        .def("initialize", &mx::GlslRenderer::initialize, py::arg("renderContextHandle") = nullptr, "Internal initialization of stages and OpenGL constructs required for program validation and rendering.\n\nArgs:\n    renderContextHandle: allows initializing the GlslRenderer with a Shared OpenGL Context")
        .def("createProgram", static_cast<void (mx::GlslRenderer::*)(const mx::ShaderPtr)>(&mx::GlslRenderer::createProgram), "Create GLSL program based on shader stage source code.\n\nArgs:\n    stages: Map of name and source code for the shader stages.")
        .def("createProgram", static_cast<void (mx::GlslRenderer::*)(const mx::GlslRenderer::StageMap&)>(&mx::GlslRenderer::createProgram), "Create GLSL program based on shader stage source code.\n\nArgs:\n    stages: Map of name and source code for the shader stages.")
        .def("validateInputs", &mx::GlslRenderer::validateInputs, "Validate inputs for the program.")
        .def("render", &mx::GlslRenderer::render, "Render the current program to an offscreen buffer.")
        .def("renderTextureSpace", &mx::GlslRenderer::renderTextureSpace, "Render the current program in texture space to an off-screen buffer.")
        .def("captureImage", &mx::GlslRenderer::captureImage, "Capture the current contents of the off-screen hardware buffer as an image.")
        .def("getProgram", &mx::GlslRenderer::getProgram, "Return the GLSL program.");
}
