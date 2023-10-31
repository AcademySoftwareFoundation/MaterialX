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

        .value("UINT8", mx::Image::BaseType::UINT8,
               PYMATERIALX_DOCSTRING(R"docstring(
    8-bit unsigned integer number.
)docstring"))

        .value("UINT16", mx::Image::BaseType::UINT16,
               PYMATERIALX_DOCSTRING(R"docstring(
    16-bit unsigned integer number.
)docstring"))

        .value("HALF", mx::Image::BaseType::HALF,
               PYMATERIALX_DOCSTRING(R"docstring(
    Half-precision floating-point number.
)docstring"))

        .value("FLOAT", mx::Image::BaseType::FLOAT,
               PYMATERIALX_DOCSTRING(R"docstring(
    Full-precision floating-point number.
)docstring"))

        .export_values();

    py::class_<mx::ImageBufferDeallocator>(mod, "ImageBufferDeallocator")

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a function to perform image buffer deallocation.
)docstring");

    py::class_<mx::Image, mx::ImagePtr>(mod, "Image")

        .def_static("create", &mx::Image::create,
                    py::arg("width"),
                    py::arg("height"),
                    py::arg("channelCount"),
                    py::arg_v("baseType",
                              mx::Image::BaseType::UINT8,
                              "BaseType.UINT8"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an empty image with the given properties.
)docstring"))

        .def("getWidth", &mx::Image::getWidth,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the width of the image.
)docstring"))

        .def("getHeight", &mx::Image::getHeight,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the height of the image.
)docstring"))

        .def("getChannelCount", &mx::Image::getChannelCount,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the channel count of the image.
)docstring"))

        .def("getBaseType", &mx::Image::getBaseType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the base type of the image.
)docstring"))

        .def("getBaseStride", &mx::Image::getBaseStride,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the stride of our base type in bytes.
)docstring"))

        .def("getMaxMipCount", &mx::Image::getMaxMipCount,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the maximum number of mipmaps for this image.
)docstring"))

        .def("setTexelColor", &mx::Image::setTexelColor,
             py::arg("x"),
             py::arg("y"),
             py::arg("color"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the texel color at the given coordinates.

    If the coordinates or image resource buffer are invalid, then an exception
    is raised.
)docstring"))

        .def("getTexelColor", &mx::Image::getTexelColor,
             py::arg("x"),
             py::arg("y"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the texel color at the given coordinates.

    If the coordinates or image resource buffer are invalid, then an exception
    is raised.
)docstring"))

        .def("isUniformColor", &mx::Image::isUniformColor,
             py::arg("uniformColor"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if all texels of this image are identical in color.

    :type uniformColor: Color4
    :param uniformColor: Return the uniform color of the image, if any.
    :raises LookupError: If the image resource buffer is invalid.
)docstring"))

        .def("setUniformColor", &mx::Image::setUniformColor,
             py::arg("color"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set all texels of this image to a uniform color.
)docstring"))

        .def("applyMatrixTransform", &mx::Image::applyMatrixTransform,
             py::arg("mat"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Apply the given matrix transform to all texels of this image.
)docstring"))

        .def("applyGammaTransform", &mx::Image::applyGammaTransform,
             py::arg("gamma"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Apply the given gamma transform to all texels of this image.
)docstring"))

        .def("copy", &mx::Image::copy,
             py::arg("channelCount"),
             py::arg("baseType"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create a copy of this image with the given channel count and base type.
)docstring"))

        .def("applyBoxBlur", &mx::Image::applyBoxBlur,
             PYMATERIALX_DOCSTRING(R"docstring(
    Apply a 3x3 box blur to this image, returning a new blurred image.
)docstring"))

        .def("applyGaussianBlur", &mx::Image::applyGaussianBlur,
             PYMATERIALX_DOCSTRING(R"docstring(
    Apply a 7x7 Gaussian blur to this image, returning a new blurred image.
)docstring"))

        .def("splitByLuminance", &mx::Image::splitByLuminance,
             py::arg("luminance"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Split this image by the given `luminance` threshold, returning the
    resulting underflow and overflow images.
)docstring"))

        .def("setResourceBuffer", &mx::Image::setResourceBuffer,
             py::arg("buffer"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the resource buffer for this image.
)docstring"))

        .def("getResourceBuffer", &mx::Image::getResourceBuffer,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the resource buffer for this image.
)docstring"))

        .def("createResourceBuffer", &mx::Image::createResourceBuffer,
             PYMATERIALX_DOCSTRING(R"docstring(
    Allocate a resource buffer for this image that matches its properties.
)docstring"))

        .def("releaseResourceBuffer", &mx::Image::releaseResourceBuffer,
             PYMATERIALX_DOCSTRING(R"docstring(
    Release the resource buffer for this image.
)docstring"))

        .def("setResourceBufferDeallocator", &mx::Image::setResourceBufferDeallocator,
             py::arg("deallocator"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the resource buffer deallocator for this image.
)docstring"))

        .def("getResourceBufferDeallocator", &mx::Image::getResourceBufferDeallocator,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the resource buffer deallocator for this image.
)docstring"))

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
