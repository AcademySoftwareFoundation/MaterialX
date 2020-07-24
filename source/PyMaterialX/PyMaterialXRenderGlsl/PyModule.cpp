//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyGlslProgram(py::module& mod);
void bindPyGlslRenderer(py::module& mod);
void bindPyGLTextureHandler(py::module& mod);
void bindPyTextureBaker(py::module& mod);

PYBIND11_MODULE(PyMaterialXRenderGlsl, mod)
{
    mod.doc() = "Module containing Python bindings for the MaterialXRenderGlsl library";

    bindPyGlslProgram(mod);
    bindPyGlslRenderer(mod);
    bindPyGLTextureHandler(mod);
    bindPyTextureBaker(mod);
}
