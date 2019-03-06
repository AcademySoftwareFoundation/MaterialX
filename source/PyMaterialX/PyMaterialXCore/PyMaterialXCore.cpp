//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyDefinition(py::module& mod);
void bindPyDocument(py::module& mod);
void bindPyElement(py::module& mod);
void bindPyException(py::module& mod);
void bindPyGeom(py::module& mod);
void bindPyInterface(py::module& mod);
void bindPyLook(py::module& mod);
void bindPyMaterial(py::module& mod);
void bindPyNode(py::module& mod);
void bindPyProperty(py::module& mod);
void bindPyTraversal(py::module& mod);
void bindPyTypes(py::module& mod);
void bindPyUtil(py::module& mod);
void bindPyValue(py::module& mod);
void bindPyVariant(py::module& mod);

PYBIND11_MODULE(PyMaterialXCore, mod)
{
    mod.doc() = "Module containing Python bindings for MaterialX C++";

    bindPyElement(mod);
    bindPyTraversal(mod);
    bindPyInterface(mod);
    bindPyValue(mod);
    bindPyGeom(mod);
    bindPyProperty(mod);
    bindPyLook(mod);
    bindPyDefinition(mod);
    bindPyNode(mod);
    bindPyMaterial(mod);
    bindPyVariant(mod);
    bindPyDocument(mod);
    bindPyTypes(mod);
    bindPyUtil(mod);
    bindPyException(mod);
}
