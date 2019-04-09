//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyOslValidator(py::module& mod);

PYBIND11_MODULE(PyMaterialXRenderOsl, mod)
{
    mod.doc() = "Module containing Python bindings for the MaterialXRenderOsl library";

    bindPyOslValidator(mod);
}
