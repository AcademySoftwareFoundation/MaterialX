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
        .def_static("create", &mx::MslRenderer::create)
        .def("initialize", &mx::MslRenderer::initialize, py::arg("renderContextHandle") = nullptr)
        .def("createProgram", static_cast<void (mx::MslRenderer::*)(const mx::ShaderPtr)>(&mx::MslRenderer::createProgram))
        .def("createProgram", static_cast<void (mx::MslRenderer::*)(const mx::MslRenderer::StageMap&)>(&mx::MslRenderer::createProgram))
        .def("validateInputs", &mx::MslRenderer::validateInputs)
        .def("render", &mx::MslRenderer::render)
        .def("renderTextureSpace", &mx::MslRenderer::renderTextureSpace)
        .def("captureImage", &mx::MslRenderer::captureImage)
        .def("getProgram", &mx::MslRenderer::getProgram);
    mod.attr("MslRenderer").doc() = R"docstring(
    Helper class for rendering generated MSL code to produce images.

    There are two main interfaces which can be used:
    one which takes in a `HwShader` and
    one which allows for explicit setting of shader stage code.

    The main services provided are:
        - Validation: All shader stages are compiled and atteched to an MSL shader program.
        - Introspection: The compiled shader program is examined for uniforms and attributes.
        - Binding: Uniforms and attributes which match the predefined variables generated the MSL code generator
          will have values assigned to this. This includes matrices, attribute streams, and textures.
        - Rendering: The program with bound inputs will be used to drawing geometry to an offscreen buffer.
          An interface is provided to save this offscreen buffer to disk using an externally defined image handler.)docstring";
}
