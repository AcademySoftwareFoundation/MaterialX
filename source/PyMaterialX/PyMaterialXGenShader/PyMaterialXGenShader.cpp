//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyColorManagement(py::module& mod);
void bindPyGenOptions(py::module& mod);
void bindPyShader(py::module& mod);
void bindPyHwShader(py::module& mod);
void bindPyShaderGenerator(py::module& mod);
void bindPyUtil(py::module& mod);

PYBIND11_MODULE(PyMaterialXGenShader, mod)
{
    mod.doc() = "Module containing Python bindings for the MaterialXGenShader library (C++)";

    bindPyColorManagement(mod);
    bindPyGenOptions(mod);
    bindPyShader(mod);
    bindPyHwShader(mod);
    bindPyShaderGenerator(mod);
    bindPyUtil(mod);
}
