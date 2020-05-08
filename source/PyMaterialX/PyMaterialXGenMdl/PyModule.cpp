//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyMdlShaderGenerator(py::module& mod);

PYBIND11_MODULE(PyMaterialXGenMdl, mod)
{
    mod.doc() = "Module containing Python bindings for the MaterialXGenMdl library";

    bindPyMdlShaderGenerator(mod);
};
