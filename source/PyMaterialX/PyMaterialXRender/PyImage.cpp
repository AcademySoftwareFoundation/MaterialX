//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/Image.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyImage(py::module& mod)
{
    py::class_<mx::ImageBufferDeallocator>(mod, "ImageBufferDeallocator");

    py::class_<mx::Image>(mod, "Image")
        .def_static("create", &mx::Image::create)
        .def("getWidth", &mx::Image::getWidth)
        .def("getHeight", &mx::Image::getHeight)
        .def("getChannelCount", &mx::Image::getChannelCount)
        .def("getBaseType", &mx::Image::getBaseType)
        .def("getBaseStride", &mx::Image::getBaseStride)
        .def("getMaxMipCount", &mx::Image::getMaxMipCount)
        .def("setResourceBuffer", &mx::Image::setResourceBuffer)
        .def("getResourceBuffer", &mx::Image::getResourceBuffer)
        .def("createResourceBuffer", &mx::Image::createResourceBuffer)
        .def("releaseResourceBuffer", &mx::Image::releaseResourceBuffer)
        .def("setResourceBufferDeallocator", &mx::Image::setResourceBufferDeallocator)
        .def("getResourceBufferDeallocator", &mx::Image::getResourceBufferDeallocator)
        .def("getTexelColor", &mx::Image::getTexelColor);

        mod.def("createUniformImage", &mx::createUniformImage);
        mod.def("createImageStrip", &mx::createImageStrip);
}
