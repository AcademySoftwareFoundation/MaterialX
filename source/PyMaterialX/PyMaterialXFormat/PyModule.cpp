//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyFile(py::module& mod);
void bindPyXmlIo(py::module& mod);
void bindPyUtil(py::module& mod);

PYBIND11_MODULE(PyMaterialXFormat, mod)
{
    mod.doc() = "Cross-platform support for working with files, paths, and environment variables";

    bindPyFile(mod);
    bindPyXmlIo(mod);
    bindPyUtil(mod);
}
