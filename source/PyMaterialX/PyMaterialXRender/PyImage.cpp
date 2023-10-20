//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/Image.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyImage(py::module& mod)
{
    py::enum_<mx::Image::BaseType>(mod, "BaseType",
                                   PYMATERIALX_DOCSTRING(R"docstring(
    Enumeration of `Image` base types.

    :see: https://materialx.org/docs/api/class_image.html#pub-types
)docstring"))
        .value("UINT8", mx::Image::BaseType::UINT8)
        .value("UINT16", mx::Image::BaseType::UINT16)
        .value("HALF", mx::Image::BaseType::HALF)
        .value("FLOAT", mx::Image::BaseType::FLOAT)
        .export_values();

    py::class_<mx::ImageBufferDeallocator>(mod, "ImageBufferDeallocator")
        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a function to perform image buffer deallocation.
)docstring");

    py::class_<mx::Image, mx::ImagePtr>(mod, "Image")
        .def_static("create", &mx::Image::create)
        .def("getWidth", &mx::Image::getWidth)
        .def("getHeight", &mx::Image::getHeight)
        .def("getChannelCount", &mx::Image::getChannelCount)
        .def("getBaseType", &mx::Image::getBaseType)
        .def("getBaseStride", &mx::Image::getBaseStride)
        .def("getMaxMipCount", &mx::Image::getMaxMipCount)
        .def("setTexelColor", &mx::Image::setTexelColor)
        .def("getTexelColor", &mx::Image::getTexelColor)
        .def("isUniformColor", &mx::Image::isUniformColor)
        .def("setUniformColor", &mx::Image::setUniformColor)
        .def("applyMatrixTransform", &mx::Image::applyMatrixTransform)
        .def("applyGammaTransform", &mx::Image::applyGammaTransform)
        .def("copy", &mx::Image::copy)
        .def("applyBoxBlur", &mx::Image::applyBoxBlur)
        .def("applyGaussianBlur", &mx::Image::applyGaussianBlur)
        .def("splitByLuminance", &mx::Image::splitByLuminance)
        .def("setResourceBuffer", &mx::Image::setResourceBuffer)
        .def("getResourceBuffer", &mx::Image::getResourceBuffer)
        .def("createResourceBuffer", &mx::Image::createResourceBuffer)
        .def("releaseResourceBuffer", &mx::Image::releaseResourceBuffer)
        .def("setResourceBufferDeallocator", &mx::Image::setResourceBufferDeallocator)
        .def("getResourceBufferDeallocator", &mx::Image::getResourceBufferDeallocator)
        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing an image in system memory.

    :see: https://materialx.org/docs/api/class_image.html
)docstring");

        mod.def("createUniformImage", &mx::createUniformImage,
                py::arg("width"),
                py::arg("height"),
                py::arg("channelCount"),
                py::arg("baseType"),
                py::arg("color"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Create a uniform-color image with the given properties.
)docstring"));

        mod.def("createImageStrip", &mx::createImageStrip,
                py::arg("images"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Create a horizontal image strip from a list of images with identical
    resolutions and formats.
)docstring"));

        mod.def("getMaxDimensions", &mx::getMaxDimensions,
                py::arg("images"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Compute the maximum width and height of all images in the given list.
)docstring"));
}
