//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// Docstring for the PyMaterialXCore module

#define PyMaterialXCore_DOCSTRING PYMATERIALX_DOCSTRING(R"docstring(
Core MaterialX elements and graph traversal.

All functions and classes that are defined in this module are available in
the top-level `MaterialX` Python package, and are typically used via an
`import` alias named `mx`:

.. code:: python

    import MaterialX as mx

Library Version
---------------

.. autofunction:: getVersionIntegers
.. autofunction:: getVersionString

Document Element Tree
---------------------

.. autofunction:: createDocument
.. autofunction:: getConnectedOutputs
.. autofunction:: prettyPrint

Element Classes
---------------

**Class Hierarchy**

* `Element`
    * `TypedElement`
        * `InterfaceElement`
            * `GraphElement`
                * `Document`
                * `NodeGraph`
            * `Implementation`
            * `Node`
            * `NodeDef`
            * `Variant`
        * `ValueElement`
            * `PortElement`
                * `Input`
                * `Output`
            * `GeomProp`
            * `Property`
            * `PropertyAssign`
            * `Token`
        * `AttributeDef`
        * `GeomPropDef`
        * `Member`
        * `TargetDef`
    * `GeomElement`
        * `GeomInfo`
        * `MaterialAssign`
        * `PropertySetAssign`
        * `Visibility`
    * `Backdrop`
    * `Collection`
    * `CommentElement`
    * `GenericElement`
    * `Look`
    * `LookGroup`
    * `NewlineElement`
    * `PropertySet`
    * `TypeDef`
    * `Unit`
    * `UnitDef`
    * `UnitTypeDef`
    * `VariantSet`
    * `VariantAssign`
* `Edge`
* `StringResolver`

**Alphabetical Index**

.. autosummary::
    :toctree: elements

    AttributeDef
    Backdrop
    Collection
    CommentElement
    Document
    Edge
    Element
    GenericElement
    GeomElement
    GeomInfo
    GeomProp
    GeomPropDef
    GraphElement
    Implementation
    Input
    InterfaceElement
    Look
    LookGroup
    MaterialAssign
    Member
    NewlineElement
    Node
    NodeDef
    NodeGraph
    Output
    PortElement
    Property
    PropertyAssign
    PropertySet
    PropertySetAssign
    StringResolver
    TargetDef
    Token
    TypeDef
    TypedElement
    Unit
    UnitDef
    UnitTypeDef
    ValueElement
    Variant
    VariantAssign
    VariantSet
    Visibility

Value Classes
-------------

**Class Hierarchy**

* `Value`
    * `TypedValued` -- The class template for typed subclasses of `Value`
        * `TypedValue_boolean`
        * `TypedValue_booleanarray`
        * `TypedValue_color3`
        * `TypedValue_color4`
        * `TypedValue_float`
        * `TypedValue_floatarray`
        * `TypedValue_integer`
        * `TypedValue_integerarray`
        * `TypedValue_matrix33`
        * `TypedValue_matrix44`
        * `TypedValue_string`
        * `TypedValue_stringarray`
        * `TypedValue_vector2`
        * `TypedValue_vector3`
        * `TypedValue_vector4`
* `MatrixBase`
    * `Matrix33`
    * `Matrix44`
* `VectorBase`
    * `Vector2`
    * `Vector3`
    * `Vector4`
* `Color3`
* `Color4`

**Alphabetical Index**

.. autosummary::
    :toctree: value-classes

    Color3
    Color4
    Matrix33
    Matrix44
    MatrixBase
    TypedValue_boolean
    TypedValue_booleanarray
    TypedValue_color3
    TypedValue_color4
    TypedValue_float
    TypedValue_floatarray
    TypedValue_integer
    TypedValue_integerarray
    TypedValue_matrix33
    TypedValue_matrix44
    TypedValue_string
    TypedValue_stringarray
    TypedValue_vector2
    TypedValue_vector3
    TypedValue_vector4
    Value
    Vector2
    Vector3
    Vector4
    VectorBase

Material Node Utilities
-----------------------

.. autofunction:: getGeometryBindings
.. autofunction:: getShaderNodes

Iterator Classes
----------------

.. autosummary::
    :toctree: iterators

    GraphIterator
    InheritanceIterator
    TreeIterator

Predicate Classes
-----------------

.. autosummary::
    :toctree: predicates

    ElementPredicate
    NodePredicate

Unit Converter Classes
----------------------

**Class Hierarchy**

* `UnitConverter`
    * `LinearUnitConverter`
* `UnitConverterRegistry`

**Alphabetical Index**

.. autosummary::
    :toctree: unit-converters

    LinearUnitConverter
    UnitConverter
    UnitConverterRegistry

Exception Classes
-----------------

**Class Hierarchy**

* `Exception`
    * `ExceptionFoundCycle`
    * `ExceptionOrphanedElement`
    * `PyMaterialXFormat.ExceptionFileMissing`
    * `PyMaterialXFormat.ExceptionParseError`
    * `PyMaterialXRender.ExceptionRenderError`

**Alphabetical Index**

.. autosummary::
    :toctree: exceptions

    Exception
    ExceptionFoundCycle
    ExceptionOrphanedElement

Name/Path Utilities
-------------------

.. autofunction:: createNamePath
.. autofunction:: createValidName
.. autofunction:: incrementName
.. autofunction:: isValidName
.. autofunction:: parentNamePath
.. autofunction:: splitNamePath

String Utilities
----------------

.. autofunction:: joinStrings
.. autofunction:: replaceSubstrings
.. autofunction:: splitString
.. autofunction:: stringEndsWith
.. autofunction:: stringStartsWith
.. autofunction:: geomStringsMatch
.. autofunction:: targetStringsMatch

Module Constants
----------------

.. py:data:: DEFAULT_TYPE_STRING
    :type: str
    :value: "color3"

    The name of the default type that is used when no other specific type
    is given/available.

    :see: `InterfaceElement.addInput()`
    :see: `InterfaceElement.addOutput()`

.. py:data:: FILENAME_TYPE_STRING
    :type: str
    :value: "filename"

    :see: `StringResolver.resolve()`

.. py:data:: GEOMNAME_TYPE_STRING
    :type: str
    :value: "geomname"

    :see: `StringResolver.resolve()`

.. py:data:: STRING_TYPE_STRING
    :type: str
    :value: "string"

.. py:data:: BSDF_TYPE_STRING
    :type: str
    :value: "BSDF"

.. py:data:: EDF_TYPE_STRING
    :type: str
    :value: "EDF"

.. py:data:: VDF_TYPE_STRING
    :type: str
    :value: "VDF"

.. py:data:: SURFACE_SHADER_TYPE_STRING
    :type: str
    :value: "surfaceshader"

    The node type name used for surface shader nodes.

    :see: `getShaderNodes()`

.. py:data:: DISPLACEMENT_SHADER_TYPE_STRING
    :type: str
    :value: "displacementshader"

    The node type name used for displacement shader nodes.

    :see: `getShaderNodes()`

.. py:data:: VOLUME_SHADER_TYPE_STRING
    :type: str
    :value: "volumeshader"

    The node type name used for volume shader nodes.

    :see: `getShaderNodes()`

.. py:data:: LIGHT_SHADER_TYPE_STRING
    :type: str
    :value: "lightshader"

    The node type name used for light shader nodes.

    :see: `getShaderNodes()`

.. py:data:: MATERIAL_TYPE_STRING
    :type: str
    :value: "material"

.. py:data:: SURFACE_MATERIAL_NODE_STRING
    :type: str
    :value: "surfacematerial"

.. py:data:: VOLUME_MATERIAL_NODE_STRING
    :type: str
    :value: "volumematerial"

.. py:data:: MULTI_OUTPUT_TYPE_STRING
    :type: str
    :value: "multioutput"

.. py:data:: NONE_TYPE_STRING
    :type: str
    :value: "none"

.. py:data:: VALUE_STRING_TRUE
    :type: str
    :value: "true"

.. py:data:: VALUE_STRING_FALSE
    :type: str
    :value: "false"

.. py:data:: NAME_PREFIX_SEPARATOR
    :type: str
    :value: ":"

.. py:data:: NAME_PATH_SEPARATOR
    :type: str
    :value: "/"

.. py:data:: ARRAY_VALID_SEPARATORS
    :type: str
    :value: ", "

.. py:data:: ARRAY_PREFERRED_SEPARATOR
    :type: str
    :value: ", "

.. py:data:: GEOM_PATH_SEPARATOR
    :type: str
    :value: "/"

.. py:data:: UNIVERSAL_GEOM_NAME
    :type: str
    :value: GEOM_PATH_SEPARATOR

    :see: `GEOM_PATH_SEPARATOR`
    :see: `getGeometryBindings()`

.. py:data:: UDIM_TOKEN
    :type: str
    :value: "<UDIM>"

.. py:data:: UV_TILE_TOKEN
    :type: str
    :value: "<UVTILE>"

.. py:data:: UDIM_SET_PROPERTY
    :type: str
    :value: "udimset"

)docstring");
