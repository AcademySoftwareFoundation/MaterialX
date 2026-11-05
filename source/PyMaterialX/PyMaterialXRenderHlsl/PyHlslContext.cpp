//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderHlsl/HlslContext.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyHlslContext(py::module& mod)
{
    py::class_<mx::HlslContext, mx::HlslContextPtr>(mod, "HlslContext")
        .def_static("create", &mx::HlslContext::create)
        .def("isHardware", &mx::HlslContext::isHardware);
}
