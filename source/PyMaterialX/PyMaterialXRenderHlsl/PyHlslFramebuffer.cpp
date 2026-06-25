//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderHlsl/HlslFramebuffer.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyHlslFramebuffer(py::module& mod)
{
    py::class_<mx::HlslFramebuffer, mx::HlslFramebufferPtr>(mod, "HlslFramebuffer")
        .def_static("create", &mx::HlslFramebuffer::create)
        .def("getWidth",   &mx::HlslFramebuffer::getWidth)
        .def("getHeight",  &mx::HlslFramebuffer::getHeight)
        .def("bind",       &mx::HlslFramebuffer::bind)
        .def("unbind",     &mx::HlslFramebuffer::unbind)
        .def("clear",      &mx::HlslFramebuffer::clear)
        .def("readColor",  &mx::HlslFramebuffer::readColor);
}
