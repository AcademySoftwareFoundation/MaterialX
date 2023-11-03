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

        .def_static("create", &mx::GLTextureHandler::create,
                    py::arg("imageLoader"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class, initialized with the given image loader.
)docstring"))

        .def("bindImage", &mx::GLTextureHandler::bindImage,
             py::arg("image"),
             py::arg("samplingProperties"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind an image. This method will bind the texture to an active texture
    unit as defined by the corresponding image description. The method
    will fail if there are not enough available image units to bind to.
)docstring"))

        .def("unbindImage", &mx::GLTextureHandler::unbindImage,
             py::arg("image"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Unbind an image.
)docstring"))

        .def("createRenderResources", &mx::GLTextureHandler::createRenderResources,
             py::arg("image"),
             py::arg("generateMipMaps"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create rendering resources for the given `image`.
)docstring"))

        .def("releaseRenderResources", &mx::GLTextureHandler::releaseRenderResources,
             py::arg("image") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Release rendering resources for the given `image`, or for all cached images
    if `image` is `None`.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    An OpenGL texture handler class.

    :see: https://materialx.org/docs/api/class_g_l_texture_handler.html
)docstring");
}
