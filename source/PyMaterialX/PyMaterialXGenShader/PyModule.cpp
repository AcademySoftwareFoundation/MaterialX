//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
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
    mod.doc() = R"docstring(
    Core shader generation support.

    Shader Generation Classes
    -------------------------

    .. autosummary::
        :toctree: shader-generation

        ShaderGenerator
        HwShaderGenerator
        HwResourceBindingContext
        GenContext
        GenOptions
        GenUserData
        ApplicationVariableHandler
        Shader
        ShaderPort
        ShaderPortPredicate
        ShaderStage
        ShaderTranslator
        TypeDesc
        VariableBlock

    Enumeration Classes
    -------------------

    .. autosummary::
        :toctree: enumeration

        ShaderInterfaceType
        HwSpecularEnvironmentMethod

    Color Management Classes
    ------------------------

    .. autosummary::
        :toctree: color-management

        ColorManagementSystem
        DefaultColorManagementSystem
        ColorSpaceTransform

    Unit System Classes
    -------------------

    .. autosummary::
        :toctree: unit-system

        UnitSystem
        UnitTransform

    Utility Functions
    -----------------

    .. autofunction:: connectsToWorldSpaceNode
    .. autofunction:: elementRequiresShading
    .. autofunction:: findRenderableElements
    .. autofunction:: findRenderableMaterialNodes
    .. autofunction:: getNodeDefInput
    .. autofunction:: getUdimCoordinates
    .. autofunction:: getUdimScaleAndOffset
    .. autofunction:: hasElementAttributes
    .. autofunction:: isTransparentSurface
    .. autofunction:: mapValueToColor
    .. autofunction:: requiresImplementation
    .. autofunction:: tokenSubstitution
)docstring";

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
