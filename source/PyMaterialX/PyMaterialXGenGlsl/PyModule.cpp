//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyGlslShaderGenerator(py::module& mod);
void bindPyGlslResourceBindingContext(py::module &mod);
void bindPyEsslShaderGenerator(py::module& mod);
void bindPyVkShaderGenerator(py::module& mod);

PYBIND11_MODULE(PyMaterialXGenGlsl, mod)
{
    mod.doc() = "Module containing Python bindings for the MaterialXGenGlsl library";

    bindPyGlslShaderGenerator(mod);
    bindPyGlslResourceBindingContext(mod);

    bindPyEsslShaderGenerator(mod);
    bindPyVkShaderGenerator(mod);
}
