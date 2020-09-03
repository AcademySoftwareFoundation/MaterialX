//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyColorManagement(py::module& mod);
void bindPyShaderPort(py::module& mod);
void bindPyShader(py::module& mod);
void bindPyShaderGenerator(py::module& mod);
void bindPyGenContext(py::module& mod);
void bindPyHwShaderGenerator(py::module& mod);
void bindPyHwResourceBindingContext(py::module &mod);
void bindPyGenUserData(py::module& mod);
void bindPyGenOptions(py::module& mod);
void bindPyShaderStage(py::module& mod);
void bindPyShaderTranslator(py::module& mod);
void bindPyUtil(py::module& mod);
void bindPyTypeDesc(py::module& mod);
void bindPyUnitSystem(py::module& mod);

PYBIND11_MODULE(PyMaterialXGenShader, mod)
{
    mod.doc() = "Module containing Python bindings for the MaterialXGenShader library";

    bindPyColorManagement(mod);
    bindPyShaderPort(mod);
    bindPyShader(mod);
    bindPyShaderGenerator(mod);
    bindPyGenContext(mod);
    bindPyHwShaderGenerator(mod);
    bindPyGenOptions(mod);
    bindPyGenUserData(mod);
    bindPyShaderStage(mod);
    bindPyShaderTranslator(mod);
    bindPyUtil(mod);
    bindPyTypeDesc(mod);
    bindPyUnitSystem(mod);
    bindPyHwResourceBindingContext(mod);
}
