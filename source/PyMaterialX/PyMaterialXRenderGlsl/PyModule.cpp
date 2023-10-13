//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyGlslProgram(py::module& mod);
void bindPyGlslRenderer(py::module& mod);
void bindPyGLTextureHandler(py::module& mod);
void bindPyTextureBaker(py::module& mod);

PYBIND11_MODULE(PyMaterialXRenderGlsl, mod)
{
    mod.doc() = "Rendering materials using OpenGL Shading Language";

    bindPyGlslProgram(mod);
    bindPyGlslRenderer(mod);
    bindPyGLTextureHandler(mod);
    bindPyTextureBaker(mod);
}
