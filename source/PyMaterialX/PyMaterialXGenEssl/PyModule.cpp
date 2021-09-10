//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyEsslShaderGenerator(py::module& mod);
void bindPyEsslResourceBindingContext(py::module &mod);

PYBIND11_MODULE(PyMaterialXGenEssl, mod)
{
    mod.doc() = "Module containing Python bindings for the MaterialXGenEssl library";

    bindPyEsslShaderGenerator(mod);
}
