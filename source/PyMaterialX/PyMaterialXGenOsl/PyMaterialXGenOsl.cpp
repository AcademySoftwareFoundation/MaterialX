//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyOslShaderGenerator(py::module& mod);
void bindPyArnoldShaderGenerator(py::module& mod);

PYBIND11_MODULE(PyMaterialXGenOsl, mod)
{
    mod.doc() = "Module containing Python bindings for the MaterialXGenOsl library (C++)";

    bindPyOslShaderGenerator(mod);
    bindPyArnoldShaderGenerator(mod);
}
