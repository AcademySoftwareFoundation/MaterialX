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
    py::enum_<mx::Image::BaseType>(mod, "BaseType")
        .value("UINT8", mx::Image::BaseType::UINT8)
        .value("UINT16", mx::Image::BaseType::UINT16)
        .value("HALF", mx::Image::BaseType::HALF)
        .value("FLOAT", mx::Image::BaseType::FLOAT)
        .export_values();

    py::class_<mx::ImageBufferDeallocator>(mod, "ImageBufferDeallocator");

    py::class_<mx::Image, mx::ImagePtr>(mod, "Image", "Class representing an image in system memory.")
        .def_static("create", &mx::Image::create, "Create an empty image with the given properties.")
        .def("getWidth", &mx::Image::getWidth, "Return the width of the image.")
        .def("getHeight", &mx::Image::getHeight, "Return the height of the image.")
        .def("getChannelCount", &mx::Image::getChannelCount, "Return the channel count of the image.")
        .def("getBaseType", &mx::Image::getBaseType, "Return the base type of the image.")
        .def("getBaseStride", &mx::Image::getBaseStride, "Return the stride of our base type in bytes.")
        .def("getMaxMipCount", &mx::Image::getMaxMipCount, "Return the maximum number of mipmaps for this image.")
        .def("setTexelColor", &mx::Image::setTexelColor, "Set the texel color at the given coordinates.\n\nIf the coordinates or image resource buffer are invalid, then an exception is thrown.")
        .def("getTexelColor", &mx::Image::getTexelColor, "Return the texel color at the given coordinates.\n\nIf the coordinates or image resource buffer are invalid, then an exception is thrown.")
        .def("isUniformColor", &mx::Image::isUniformColor, "Return true if all texels of this image are identical in color.\n\nArgs:\n    uniformColor: Return the uniform color of the image, if any.")
        .def("setUniformColor", &mx::Image::setUniformColor, "Set all texels of this image to a uniform color.")
        .def("applyMatrixTransform", &mx::Image::applyMatrixTransform, "Apply the given matrix transform to all texels of this image.")
        .def("applyGammaTransform", &mx::Image::applyGammaTransform, "Apply the given gamma transform to all texels of this image.")
        .def("copy", &mx::Image::copy, "Create a copy of this image with the given channel count and base type.")
        .def("applyBoxBlur", &mx::Image::applyBoxBlur, "Apply a 3x3 box blur to this image, returning a new blurred image.")
        .def("applyGaussianBlur", &mx::Image::applyGaussianBlur, "Apply a 7x7 Gaussian blur to this image, returning a new blurred image.")
        .def("applyBoxDownsample", &mx::Image::applyBoxDownsample, "Downsample this image by an integer factor using a box filter, returning the new reduced image.")
        .def("splitByLuminance", &mx::Image::splitByLuminance, "Split this image by the given luminance threshold, returning the resulting underflow and overflow images.")
        .def("setResourceBuffer", &mx::Image::setResourceBuffer, "Set the resource buffer for this image.")
        .def("getResourceBuffer", &mx::Image::getResourceBuffer, "Return the resource buffer for this image.")
        .def("createResourceBuffer", &mx::Image::createResourceBuffer, "Allocate a resource buffer for this image that matches its properties.")
        .def("releaseResourceBuffer", &mx::Image::releaseResourceBuffer, "Release the resource buffer for this image.")
        .def("setResourceBufferDeallocator", &mx::Image::setResourceBufferDeallocator, "Set the resource buffer deallocator for this image.")
        .def("getResourceBufferDeallocator", &mx::Image::getResourceBufferDeallocator, "Return the resource buffer deallocator for this image.");

        mod.def("createUniformImage", &mx::createUniformImage, "Create a uniform-color image with the given properties.");
        mod.def("createImageStrip", &mx::createImageStrip, "Create a horizontal image strip from a vector of images with identical resolutions and formats.");
        mod.def("getMaxDimensions", &mx::getMaxDimensions, "Compute the maximum width and height of all images in the given vector.");
}
