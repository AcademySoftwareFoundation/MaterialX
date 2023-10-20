//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
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
void bindPyUnitConverters(py::module& mod);
void bindPyUtil(py::module& mod);
void bindPyValue(py::module& mod);
void bindPyVariant(py::module& mod);

PYBIND11_MODULE(PyMaterialXCore, mod)
{
    mod.doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Core MaterialX elements and graph traversal.

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
)docstring");

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
    bindPyUnitConverters(mod);
    bindPyUtil(mod);
    bindPyException(mod);
}
