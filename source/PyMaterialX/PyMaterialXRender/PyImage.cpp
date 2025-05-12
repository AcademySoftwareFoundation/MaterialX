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
                                   "Enumeration of `Image` base types.\n\n"
                                   ":see: https://materialx.org/docs/api/class_image.html#pub-types")
        .value("UINT8", mx::Image::BaseType::UINT8, "8-bit unsigned integer number.")
        .value("UINT16", mx::Image::BaseType::UINT16, "16-bit unsigned integer number.")
        .value("HALF", mx::Image::BaseType::HALF, "Half-precision floating-point number.")
        .value("FLOAT", mx::Image::BaseType::FLOAT, "Full-precision floating-point number.")
        .export_values();

    py::class_<mx::ImageBufferDeallocator>(mod, "ImageBufferDeallocator");
    mod.attr("ImageBufferDeallocator").doc() = R"docstring(
    A function to perform image buffer deallocation.)docstring";

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
        .def("applyBoxDownsample", &mx::Image::applyBoxDownsample)
        .def("splitByLuminance", &mx::Image::splitByLuminance)
        .def("setResourceBuffer", &mx::Image::setResourceBuffer)
        .def("getResourceBuffer", &mx::Image::getResourceBuffer)
        .def("createResourceBuffer", &mx::Image::createResourceBuffer)
        .def("releaseResourceBuffer", &mx::Image::releaseResourceBuffer)
        .def("setResourceBufferDeallocator", &mx::Image::setResourceBufferDeallocator)
        .def("getResourceBufferDeallocator", &mx::Image::getResourceBufferDeallocator);
    mod.attr("Image").doc() = R"docstring(
    Class representing an image in system memory.

    :see: https://materialx.org/docs/api/class_image.html)docstring";

    mod.def("createUniformImage", &mx::createUniformImage);
    mod.def("createImageStrip", &mx::createImageStrip);
    mod.def("getMaxDimensions", &mx::getMaxDimensions);
}
