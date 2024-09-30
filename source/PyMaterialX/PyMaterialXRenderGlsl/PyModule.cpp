//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>
#include "__doc__.md.h"

namespace py = pybind11;

void bindPyGlslProgram(py::module& mod);
void bindPyGlslRenderer(py::module& mod);
void bindPyGLTextureHandler(py::module& mod);
void bindPyTextureBaker(py::module& mod);

PYBIND11_MODULE(PyMaterialXRenderGlsl, mod)
{
    mod.doc() = PyMaterialXRenderGlsl_DOCSTRING;

    // PyMaterialXRenderGlsl depends on types defined in PyMaterialXRender
    PYMATERIALX_IMPORT_MODULE(PyMaterialXRender);

    bindPyGlslProgram(mod);
    bindPyGlslRenderer(mod);
    bindPyGLTextureHandler(mod);
    bindPyTextureBaker(mod);
}
