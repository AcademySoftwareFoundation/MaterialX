//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// Docstring for the PyMaterialXGenShader module

#define PyMaterialXGenShader_DOCSTRING PYMATERIALX_DOCSTRING(R"docstring(
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
)docstring");
