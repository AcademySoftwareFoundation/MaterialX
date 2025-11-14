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
    py::class_<mx::GLTextureHandler, mx::ImageHandler, mx::GLTextureHandlerPtr>(mod, "GLTextureHandler", "An OpenGL texture handler class.")
        .def_static("create", &mx::GLTextureHandler::create)
        .def("bindImage", &mx::GLTextureHandler::bindImage, "Bind a single image.")
        .def("unbindImage", &mx::GLTextureHandler::unbindImage, "Unbind an image.")
        .def("createRenderResources", &mx::GLTextureHandler::createRenderResources, "Create rendering resources for the given image.")
        .def("releaseRenderResources", &mx::GLTextureHandler::releaseRenderResources,
            py::arg("image") = nullptr, "Release rendering resources for the given image, or for all cached images if no image pointer is specified.");
}
