//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Property.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_PROPERTYSET_TYPE_INSTANCE(NAME, T)                               \
.def("_setPropertyValue" #NAME, &mx::PropertySet::setPropertyValue<T>,        \
     py::arg("name"),                                                         \
     py::arg("value"),                                                        \
     py::arg("propertyType") = mx::EMPTY_STRING)

void bindPyProperty(py::module& mod)
{
    py::class_<mx::Property, mx::PropertyPtr, mx::ValueElement>(mod, "Property")

        .def_readonly_static("CATEGORY", &mx::Property::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Property` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a property element within a `PropertySet`.

    :see: https://materialx.org/docs/api/class_property.html
)docstring");

    py::class_<mx::PropertyAssign, mx::PropertyAssignPtr, mx::ValueElement>(mod, "PropertyAssign")

        .def("setProperty", &mx::PropertyAssign::setProperty,
             py::arg("property"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the property string of this element.
)docstring"))

        .def("hasProperty", &mx::PropertyAssign::hasProperty,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a property string.
)docstring"))

        .def("getProperty", &mx::PropertyAssign::getProperty,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the property string of this element.
)docstring"))

        .def("setGeom", &mx::PropertyAssign::setGeom,
             py::arg("geom"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the geometry string of this element.
)docstring"))

        .def("hasGeom", &mx::PropertyAssign::hasGeom,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a geometry string.
)docstring"))

        .def("getGeom", &mx::PropertyAssign::getGeom,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the geometry string of this element.
)docstring"))

        .def("setCollectionString", &mx::PropertyAssign::setCollectionString,
             py::arg("collection"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the collection string of this element.
)docstring"))

        .def("hasCollectionString", &mx::PropertyAssign::hasCollectionString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a collection string.
)docstring"))

        .def("getCollectionString", &mx::PropertyAssign::getCollectionString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the collection string of this element.
)docstring"))

        .def("setCollection", &mx::PropertyAssign::setCollection,
             py::arg("collection"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Assign a `Collection` to this element.
)docstring"))

        .def("getCollection", &mx::PropertyAssign::getCollection,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Collection` that is assigned to this element.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::PropertyAssign::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `PropertyAssign` elements within a
    MaterialX document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("PROPERTY_ATTRIBUTE",
                             &mx::PropertyAssign::PROPERTY_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which a `PropertyAssign` element's property string is stored
    as an attribute.

    :see: `setProperty()`
    :see: `hasProperty()`
    :see: `getProperty()`
)docstring"))

        .def_readonly_static("GEOM_ATTRIBUTE",
                             &mx::PropertyAssign::GEOM_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which a `PropertyAssign` element's geometry string is stored
    as an attribute.

    :see: `setGeom()`
    :see: `hasGeom()`
    :see: `getGeom()`
)docstring"))

        .def_readonly_static("COLLECTION_ATTRIBUTE",
                             &mx::PropertyAssign::COLLECTION_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which a `PropertyAssign` element's collection string is
    stored as an attribute.

    :see: `setCollectionString()`
    :see: `hasCollectionString()`
    :see: `getCollectionString()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a property assignment element within a `Look`.

    :see: https://materialx.org/docs/api/class_property_assign.html
)docstring");

    py::class_<mx::PropertySet, mx::PropertySetPtr, mx::Element>(mod, "PropertySet")

        .def("addProperty", &mx::PropertySet::addProperty,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `Property` to the set.

    :type name: str
    :param name: The name of the new `Property`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new Property.
)docstring"))

        .def("getProperties", &mx::PropertySet::getProperties,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Property` elements in the set.
)docstring"))

        .def("removeProperty", &mx::PropertySet::removeProperty,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Property` with the given `name`, if present.
)docstring"))

        .def("_getPropertyValue", &mx::PropertySet::getPropertyValue,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the typed value of a property by its name.

    :type name: str
    :param name: The name of the property to be evaluated.
    :returns: If the given property is found, then its value is returned;
        otherwise, `None` is returned.
)docstring"))

        BIND_PROPERTYSET_TYPE_INSTANCE(integer, int)
        BIND_PROPERTYSET_TYPE_INSTANCE(boolean, bool)
        BIND_PROPERTYSET_TYPE_INSTANCE(float, float)
        BIND_PROPERTYSET_TYPE_INSTANCE(color3, mx::Color3)
        BIND_PROPERTYSET_TYPE_INSTANCE(color4, mx::Color4)
        BIND_PROPERTYSET_TYPE_INSTANCE(vector2, mx::Vector2)
        BIND_PROPERTYSET_TYPE_INSTANCE(vector3, mx::Vector3)
        BIND_PROPERTYSET_TYPE_INSTANCE(vector4, mx::Vector4)
        BIND_PROPERTYSET_TYPE_INSTANCE(matrix33, mx::Matrix33)
        BIND_PROPERTYSET_TYPE_INSTANCE(matrix44, mx::Matrix44)
        BIND_PROPERTYSET_TYPE_INSTANCE(string, std::string)
        BIND_PROPERTYSET_TYPE_INSTANCE(integerarray, mx::IntVec)
        BIND_PROPERTYSET_TYPE_INSTANCE(booleanarray, mx::BoolVec)
        BIND_PROPERTYSET_TYPE_INSTANCE(floatarray, mx::FloatVec)
        BIND_PROPERTYSET_TYPE_INSTANCE(stringarray, mx::StringVec)

        .def_readonly_static("CATEGORY", &mx::Property::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Property` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a property set element within a `Document`.

    :see: https://materialx.org/docs/api/class_property_set.html
)docstring");

    py::class_<mx::PropertySetAssign, mx::PropertySetAssignPtr, mx::GeomElement>(mod, "PropertySetAssign")

        .def("setPropertySetString", &mx::PropertySetAssign::setPropertySetString,
             py::arg("propertySet"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the property set string of this element.
)docstring"))

        .def("hasPropertySetString", &mx::PropertySetAssign::hasPropertySetString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a property set string.
)docstring"))

        .def("getPropertySetString", &mx::PropertySetAssign::getPropertySetString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the property set string of this element.
)docstring"))

        .def("setPropertySet", &mx::PropertySetAssign::setPropertySet,
             py::arg("propertySet"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Assign a property set to this element.
)docstring"))

        .def("getPropertySet", &mx::PropertySetAssign::getPropertySet,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the property set that is assigned to this element.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::PropertySetAssign::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `PropertySetAssign` elements within a
    MaterialX document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("PROPERTY_SET_ATTRIBUTE",
                             &mx::PropertySetAssign::PROPERTY_SET_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which a `PropertySetAssign` element's property set string is
    stored as an attribute.

    :see: `setPropertySetString()`
    :see: `hasPropertySetString()`
    :see: `getPropertySetString()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a property set assignment element within a `Look`.

    :see: https://materialx.org/docs/api/class_property_set_assign.html
)docstring");
}
