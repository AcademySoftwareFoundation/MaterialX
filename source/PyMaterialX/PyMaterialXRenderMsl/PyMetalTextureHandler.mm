//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderMsl/MetalTextureHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyMetalTextureHandler(py::module& mod)
{
    py::class_<mx::MetalTextureHandler, mx::ImageHandler, mx::MetalTextureHandlerPtr>(mod, "MetalTextureHandler")

        .def_static("create", &mx::MetalTextureHandler::create,
                    py::arg("device"),
                    py::arg("imageLoader"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class, initialized with the given MTL device and
    image loader.
)docstring"))

        .def("bindImage",
             static_cast<bool (mx::MetalTextureHandler::*)(mx::ImagePtr, const mx::ImageSamplingProperties&)>(&mx::MetalTextureHandler::bindImage),
             py::arg("image"),
             py::arg("samplingProperties"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind the given `image` and its corresponding `samplingProperties`, creating
    the underlying resource if needed.

    Actual binding of texture and sampler to command encoder happens
    automatically.
)docstring"))

        .def("unbindImage", &mx::MetalTextureHandler::unbindImage,
             py::arg("image"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Unbind an image.
)docstring"))

        .def("createRenderResources", &mx::MetalTextureHandler::createRenderResources,
             py::arg("image"),
             py::arg("generateMipMaps"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create rendering resources for the given `image`.
)docstring"))

        .def("releaseRenderResources", &mx::MetalTextureHandler::releaseRenderResources,
             py::arg("image") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Release rendering resources for the given `image`.

    If the given `image` is `None`, then no rendering resources are released.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A Metal texture handler class.
)docstring");
}
