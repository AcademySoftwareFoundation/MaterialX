//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyHlslContext(py::module& mod);
void bindPyHlslD3D12(py::module& mod);
void bindPyHlslFramebuffer(py::module& mod);
void bindPyHlslMaterial(py::module& mod);
void bindPyHlslProgram(py::module& mod);
void bindPyHlslRenderer(py::module& mod);
void bindPyHlslTextureHandler(py::module& mod);

PYBIND11_MODULE(PyMaterialXRenderHlsl, mod)
{
    mod.doc() = "Rendering support for the HLSL shading language (D3D11 and D3D12).";

    // PyMaterialXRenderHlsl depends on types defined in PyMaterialXRender
    // and the HLSL shader generator types in PyMaterialXGenHlsl.
    PYMATERIALX_IMPORT_MODULE(PyMaterialXRender);

    bindPyHlslContext(mod);
    bindPyHlslFramebuffer(mod);
    bindPyHlslProgram(mod);
    bindPyHlslMaterial(mod);
    bindPyHlslTextureHandler(mod);
    bindPyHlslRenderer(mod);
    bindPyHlslD3D12(mod);
}
