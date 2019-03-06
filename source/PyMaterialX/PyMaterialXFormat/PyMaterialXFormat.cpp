//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyXmlIo(py::module& mod);
void bindPyFile(py::module& mod);

PYBIND11_MODULE(PyMaterialXFormat, mod)
{
    mod.doc() = "Module containing Python bindings for MaterialX Format C++";

    bindPyXmlIo(mod);
    bindPyFile(mod);
}

