//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyMesh(py::module& mod);
void bindPyGeometryHandler(py::module& mod);
void bindPyLightHandler(py::module& mod);
void bindPyImage(py::module& mod);
void bindPyImageHandler(py::module& mod);
void bindPyStbImageLoader(py::module& mod);
#ifdef MATERIALX_BUILD_OIIO
void bindPyOiioImageLoader(py::module& mod);
#endif
void bindPyTinyObjLoader(py::module& mod);
void bindPyCamera(py::module& mod);
void bindPyShaderRenderer(py::module& mod);

PYBIND11_MODULE(PyMaterialXRender, mod)
{
    mod.doc() = "Module containing Python bindings for the MaterialXRender library";

    bindPyMesh(mod);
    bindPyGeometryHandler(mod);
    bindPyLightHandler(mod);
    bindPyImage(mod);
    bindPyImageHandler(mod);
    bindPyStbImageLoader(mod);
#ifdef MATERIALX_BUILD_OIIO
    bindPyOiioImageLoader(mod);
#endif
    bindPyTinyObjLoader(mod);
    bindPyCamera(mod);
    bindPyShaderRenderer(mod);
}
