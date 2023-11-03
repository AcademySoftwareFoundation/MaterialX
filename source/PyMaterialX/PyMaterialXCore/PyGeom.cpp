//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Geom.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_GEOMINFO_FUNC_INSTANCE(NAME, T) \
.def("_setGeomPropValue" #NAME, &mx::GeomInfo::setGeomPropValue<T>)

void bindPyGeom(py::module& mod)
{
    py::class_<mx::GeomElement, mx::GeomElementPtr, mx::Element>(mod, "GeomElement")

        .def("setGeom", &mx::GeomElement::setGeom,
             py::arg("geom"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the geometry string of this element.
)docstring"))

        .def("hasGeom", &mx::GeomElement::hasGeom,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a geometry string.
)docstring"))

        .def("getGeom", &mx::GeomElement::getGeom,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the geometry string of this element.
)docstring"))

        .def("setCollectionString", &mx::GeomElement::setCollectionString,
             py::arg("collection"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the collection string of this element.
)docstring"))

        .def("hasCollectionString", &mx::GeomElement::hasCollectionString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a collection string.
)docstring"))

        .def("getCollectionString", &mx::GeomElement::getCollectionString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the collection string of this element.
)docstring"))

        .def("setCollection", &mx::GeomElement::setCollection,
             py::arg("collection"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Assign a `Collection` to this element.
)docstring"))

        .def("getCollection", &mx::GeomElement::getCollection,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Collection` that is assigned to this element.
)docstring"))

        .def_readonly_static("GEOM_ATTRIBUTE", &mx::GeomElement::GEOM_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's geometry string is stored as an attribute.

    :see: `setGeom()`
    :see: `hasGeom()`
    :see: `getGeom()`
)docstring"))

        .def_readonly_static("COLLECTION_ATTRIBUTE", &mx::GeomElement::COLLECTION_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's collection string is stored as an
    attribute.

    :see: `setCollectionString()`
    :see: `hasCollectionString()`
    :see: `getCollectionString()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for geometric elements, which support bindings to geometries
    and geometric collections.

    Inherited by: `GeomInfo`, `MaterialAssign`, `PropertySetAssign`, and
    `Visibility`.

    :see: https://materialx.org/docs/api/class_geom_element.html
)docstring");

    py::class_<mx::GeomInfo, mx::GeomInfoPtr, mx::GeomElement>(mod, "GeomInfo")

        .def("addGeomProp", &mx::GeomInfo::addGeomProp,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `GeomProp` to this element.

    :param name: The name of the new `GeomProp`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `GeomProp`.
)docstring"))

        .def("getGeomProp", &mx::GeomInfo::getGeomProp,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `GeomProp`, if any, with the given name.
)docstring"))

        .def("getGeomProps", &mx::GeomInfo::getGeomProps,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `GeomProp` elements.
)docstring"))

        .def("removeGeomProp", &mx::GeomInfo::removeGeomProp,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `GeomProp`, if any, with the given name.
)docstring"))

        .def("addToken", &mx::GeomInfo::addToken,
             py::arg_v("name",
                       mx::DEFAULT_TYPE_STRING,
                       "mx.DEFAULT_TYPE_STRING"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `Token` to this element.

    :param name: The name of the new `Token`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `Token`.
)docstring"))

        .def("getToken", &mx::GeomInfo::getToken,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Token`, if any, with the given name.
)docstring"))

        .def("getTokens", &mx::GeomInfo::getTokens,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Token` elements.
)docstring"))

        .def("removeToken", &mx::GeomInfo::removeToken,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Token`, if any, with the given name.
)docstring"))

        .def("setTokenValue", &mx::GeomInfo::setTokenValue,
             py::arg("name"),
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the string `value` of a `Token` by its `name`, creating a child element
    to hold the `Token` if needed.
)docstring"))

        BIND_GEOMINFO_FUNC_INSTANCE(integer, int)
        BIND_GEOMINFO_FUNC_INSTANCE(boolean, bool)
        BIND_GEOMINFO_FUNC_INSTANCE(float, float)
        BIND_GEOMINFO_FUNC_INSTANCE(color3, mx::Color3)
        BIND_GEOMINFO_FUNC_INSTANCE(color4, mx::Color4)
        BIND_GEOMINFO_FUNC_INSTANCE(vector2, mx::Vector2)
        BIND_GEOMINFO_FUNC_INSTANCE(vector3, mx::Vector3)
        BIND_GEOMINFO_FUNC_INSTANCE(vector4, mx::Vector4)
        BIND_GEOMINFO_FUNC_INSTANCE(matrix33, mx::Matrix33)
        BIND_GEOMINFO_FUNC_INSTANCE(matrix44, mx::Matrix44)
        BIND_GEOMINFO_FUNC_INSTANCE(string, std::string)
        BIND_GEOMINFO_FUNC_INSTANCE(integerarray, mx::IntVec)
        BIND_GEOMINFO_FUNC_INSTANCE(booleanarray, mx::BoolVec)
        BIND_GEOMINFO_FUNC_INSTANCE(floatarray, mx::FloatVec)
        BIND_GEOMINFO_FUNC_INSTANCE(stringarray, mx::StringVec)

        .def_readonly_static("CATEGORY", &mx::GeomInfo::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `GeomInfo` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a geometry info element within a `Document`.

    :see: https://materialx.org/docs/api/class_geom_info.html
)docstring");

    py::class_<mx::GeomProp, mx::GeomPropPtr, mx::ValueElement>(mod, "GeomProp")

        .def_readonly_static("CATEGORY", &mx::GeomProp::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `GeomProp` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a geometric property element within a `GeomInfo`.

    :see: https://materialx.org/docs/api/class_geom_prop.html
)docstring");

    py::class_<mx::GeomPropDef, mx::GeomPropDefPtr, mx::TypedElement>(mod, "GeomPropDef")

        .def("setGeomProp", &mx::GeomPropDef::setGeomProp,
             py::arg("node"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the geometric property string of this element.
)docstring"))

        .def("hasGeomProp", &mx::GeomPropDef::hasGeomProp,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a geometric property string.
)docstring"))

        .def("getGeomProp", &mx::GeomPropDef::getGeomProp,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `GeomProp`, if any, with the given name.
)docstring"))

        .def("setSpace", &mx::GeomPropDef::setSpace,
             py::arg("space"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the geometric space string of this element.
)docstring"))

        .def("hasSpace", &mx::GeomPropDef::hasSpace,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a geometric space string.
)docstring"))

        .def("getSpace", &mx::GeomPropDef::getSpace,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the geometric space string of this element.
)docstring"))

        .def("setIndex", &mx::GeomPropDef::setIndex,
             py::arg("index"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the index string of this element.
)docstring"))

        .def("hasIndex", &mx::GeomPropDef::hasIndex,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has an index string.
)docstring"))

        .def("getIndex", &mx::GeomPropDef::getIndex,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the index string of this element.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::GeomPropDef::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `GeomPropDef` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("GEOM_PROP_ATTRIBUTE", &mx::GeomPropDef::GEOM_PROP_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's geometric property string is stored as an
    attribute. See `setGeomProp()`, `hasGeomProp()`, and `getGeomProp()`.
)docstring"))

        .def_readonly_static("SPACE_ATTRIBUTE", &mx::GeomPropDef::SPACE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's geometric space string is stored as an
    attribute. See `setSpace()`, `hasSpace()`, and `getSpace()`.
)docstring"))

        .def_readonly_static("INDEX_ATTRIBUTE", &mx::GeomPropDef::INDEX_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's index string is stored as an attribute.
    See `setIndex()`, `hasIndex()`, and `getIndex()`.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a declaration of geometric property data.

    A `GeomPropDef` element contains a reference to a geometric node and a set of
    modifiers for that node.  For example, a world-space normal can be declared
    as a reference to the `"normal"` geometric node with a space setting of
    `"world"`, or a specific set of texture coordinates can be declared as a
    reference to the `"texcoord"` geometric node with an index setting of `"1"`.

    :see: https://materialx.org/docs/api/class_geom_prop_def.html
)docstring");

    py::class_<mx::Collection, mx::CollectionPtr, mx::Element>(mod, "Collection")

        .def("setIncludeGeom", &mx::Collection::setIncludeGeom,
             py::arg("geom"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the include geometry string of this element.
)docstring"))

        .def("hasIncludeGeom", &mx::Collection::hasIncludeGeom,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has an include geometry string.
)docstring"))

        .def("getIncludeGeom", &mx::Collection::getIncludeGeom,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the include geometry string of this element.
)docstring"))

        .def("setExcludeGeom", &mx::Collection::setExcludeGeom,
             py::arg("geom"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the exclude geometry string of this element.
)docstring"))

        .def("hasExcludeGeom", &mx::Collection::hasExcludeGeom,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has an exclude geometry string.
)docstring"))

        .def("getExcludeGeom", &mx::Collection::getExcludeGeom,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the exclude geometry string of this element.
)docstring"))

        .def("setIncludeCollectionString", &mx::Collection::setIncludeCollectionString,
             py::arg("collection"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the include collection string of this element.
)docstring"))

        .def("hasIncludeCollectionString", &mx::Collection::hasIncludeCollectionString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has an include collection string.
)docstring"))

        .def("getIncludeCollectionString", &mx::Collection::getIncludeCollectionString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the include collection string of this element.
)docstring"))

        .def("setIncludeCollection", &mx::Collection::setIncludeCollection,
             py::arg("collection"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the collection that is directly included by this element.
)docstring"))

        .def("setIncludeCollections", &mx::Collection::setIncludeCollections,
             py::arg("collections"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the list of collections that are directly included by this element.
)docstring"))

        .def("getIncludeCollections", &mx::Collection::getIncludeCollections,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the list of collections that are directly included by this element.
)docstring"))

        .def("hasIncludeCycle", &mx::Collection::hasIncludeCycle,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the include chain for this element contains a cycle.
)docstring"))

        .def("matchesGeomString", &mx::Collection::matchesGeomString,
             py::arg("geom"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this collection and the given geometry string have any
    geometries in common.

    :raises ExceptionFoundCycle: If a cycle is encountered.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::Collection::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Collection` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("INCLUDE_GEOM_ATTRIBUTE", &mx::Collection::INCLUDE_GEOM_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's include geometry string is stored as an
    attribute.

    :see: `setIncludeGeom()`
    :see: `hasIncludeGeom()`
    :see: `getIncludeGeom()`
)docstring"))

        .def_readonly_static("EXCLUDE_GEOM_ATTRIBUTE", &mx::Collection::EXCLUDE_GEOM_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's exclude geometry string is stored as an
    attribute.

    :see: `setExcludeGeom()`
    :see: `hasExcludeGeom()`
    :see: `getExcludeGeom()`
)docstring"))

        .def_readonly_static("INCLUDE_COLLECTION_ATTRIBUTE", &mx::Collection::INCLUDE_COLLECTION_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's include collection string is stored as an
    attribute.

    :see: `setIncludeCollectionString()`
    :see: `hasIncludeCollectionString()`
    :see: `getIncludeCollectionString()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a collection element within a `Document`.

    :see: https://materialx.org/docs/api/class_geom_prop_def.html
)docstring");

    mod.def(
        "geomStringsMatch", &mx::geomStringsMatch,
        py::arg("geom1"),
        py::arg("geom2"),
        py::arg("contains"),
        PYMATERIALX_DOCSTRING(R"docstring(
    Given two geometry strings, each containing a list of geom names, return
    `True` if they have any geometries in common.

    An empty geometry string matches no geometries, while the universal geometry
    string `"/"` matches all non-empty geometries.

    If the `contains` argument is set to `True`, then we require that a geom path
    in the first string completely contains a geom path in the second string.

    :todo: Geometry name expressions are not yet supported.
)docstring"));

    mod.attr("GEOM_PATH_SEPARATOR") = mx::GEOM_PATH_SEPARATOR;
    mod.attr("UNIVERSAL_GEOM_NAME") = mx::UNIVERSAL_GEOM_NAME;
    mod.attr("UDIM_TOKEN") = mx::UDIM_TOKEN;
    mod.attr("UV_TILE_TOKEN") = mx::UV_TILE_TOKEN;
    mod.attr("UDIM_SET_PROPERTY") = mx::UDIM_SET_PROPERTY;
}
