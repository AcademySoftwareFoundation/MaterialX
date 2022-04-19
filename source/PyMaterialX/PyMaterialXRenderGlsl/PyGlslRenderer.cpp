//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderGlsl/GlslRenderer.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGlslRenderer(py::module& mod)
{
    py::class_<mx::GlslRenderer, mx::ShaderRenderer, mx::GlslRendererPtr>(mod, "GlslRenderer")
        .def_static("create", &mx::GlslRenderer::create)
        .def("initialize", &mx::GlslRenderer::initialize)
        .def("createProgram", static_cast<void (mx::GlslRenderer::*)(const mx::ShaderPtr)>(&mx::GlslRenderer::createProgram))
        .def("createProgram", static_cast<void (mx::GlslRenderer::*)(const mx::GlslRenderer::StageMap&)>(&mx::GlslRenderer::createProgram))
        .def("validateInputs", &mx::GlslRenderer::validateInputs)
        .def("render", &mx::GlslRenderer::render)
        .def("renderTextureSpace", &mx::GlslRenderer::renderTextureSpace)
        .def("captureImage", &mx::GlslRenderer::captureImage)
        .def("getProgram", &mx::GlslRenderer::getProgram);
}
