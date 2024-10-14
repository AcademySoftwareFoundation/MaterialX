//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderGlsl/GLTextureHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGLTextureHandler(py::module& mod)
{
    py::class_<mx::GLTextureHandler, mx::ImageHandler, mx::GLTextureHandlerPtr>(mod, "GLTextureHandler")
        .def_static("create", &mx::GLTextureHandler::create)
        .def("bindImage", &mx::GLTextureHandler::bindImage)
        .def("unbindImage", &mx::GLTextureHandler::unbindImage)
        .def("createRenderResources", &mx::GLTextureHandler::createRenderResources)
        .def("releaseRenderResources", &mx::GLTextureHandler::releaseRenderResources,
            py::arg("image") = nullptr);
    mod.attr("GLTextureHandler").doc() = R"docstring(
    An OpenGL texture handler class.

    :see: https://materialx.org/docs/api/class_g_l_texture_handler.html)docstring";
}
