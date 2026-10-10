//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderHlsl/HlslRenderer.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyHlslRenderer(py::module& mod)
{
    py::class_<mx::HlslRenderer, mx::ShaderRenderer, mx::HlslRendererPtr>(mod, "HlslRenderer")
        .def_static("create", &mx::HlslRenderer::create,
                    py::arg("width") = 512,
                    py::arg("height") = 512,
                    py::arg("baseType") = mx::Image::BaseType::UINT8)
        .def("initialize",   &mx::HlslRenderer::initialize, py::arg("renderContextHandle") = nullptr)
        .def("createProgram",
             static_cast<void (mx::HlslRenderer::*)(mx::ShaderPtr)>(&mx::HlslRenderer::createProgram))
        .def("createProgram",
             static_cast<void (mx::HlslRenderer::*)(const mx::HlslRenderer::StageMap&)>(&mx::HlslRenderer::createProgram))
        .def("validateInputs", &mx::HlslRenderer::validateInputs)
        .def("setSize",        &mx::HlslRenderer::setSize)
        .def("render",         &mx::HlslRenderer::render)
        .def("captureImage",   &mx::HlslRenderer::captureImage, py::arg("image") = nullptr)
        .def("setScreenColor", &mx::HlslRenderer::setScreenColor)
        .def("getScreenColor", &mx::HlslRenderer::getScreenColor)
        .def("setActiveMeshes",   &mx::HlslRenderer::setActiveMeshes)
        .def("setClearOnRender",  &mx::HlslRenderer::setClearOnRender)
        .def("bindImage",      &mx::HlslRenderer::bindImage)
        .def("getContext",        &mx::HlslRenderer::getContext)
        .def("getFramebuffer",    &mx::HlslRenderer::getFramebuffer)
        .def("getProgram",        &mx::HlslRenderer::getProgram)
        .def("getMaterial",       &mx::HlslRenderer::getMaterial)
        .def("getTextureHandler", &mx::HlslRenderer::getTextureHandler);
}
