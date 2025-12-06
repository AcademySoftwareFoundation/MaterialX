//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Property.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_PROPERTYSET_TYPE_INSTANCE(NAME, T) \
.def("_setPropertyValue" #NAME, &mx::PropertySet::setPropertyValue<T>, py::arg("name"), py::arg("value"), py::arg("type") = mx::EMPTY_STRING)

void bindPyProperty(py::module& mod)
{
    py::class_<mx::Property, mx::PropertyPtr, mx::ValueElement>(mod, "Property", "A property element within a PropertySet.")
        .def_readonly_static("CATEGORY", &mx::Property::CATEGORY);

    py::class_<mx::PropertyAssign, mx::PropertyAssignPtr, mx::ValueElement>(mod, "PropertyAssign", "A property assignment element within a Look.")
        .def("setProperty", &mx::PropertyAssign::setProperty, "Set the property string of this element.")
        .def("hasProperty", &mx::PropertyAssign::hasProperty, "Return true if this element has a property string.")
        .def("getProperty", &mx::PropertyAssign::getProperty, "Return the property string of this element.")
        .def("setGeom", &mx::PropertyAssign::setGeom, "Set the geometry string of this element.")
        .def("hasGeom", &mx::PropertyAssign::hasGeom, "Return true if this element has a geometry string.")
        .def("getGeom", &mx::PropertyAssign::getGeom, "Return the geometry string of this element.")
        .def("setCollectionString", &mx::PropertyAssign::setCollectionString, "Set the collection string of this element.")
        .def("hasCollectionString", &mx::PropertyAssign::hasCollectionString, "Return true if this element has a collection string.")
        .def("getCollectionString", &mx::PropertyAssign::getCollectionString, "Return the collection string of this element.")
        .def("setCollection", &mx::PropertyAssign::setCollection, "Assign a Collection to this element.")
        .def("getCollection", &mx::PropertyAssign::getCollection, "Return the Collection that is assigned to this element.")
        .def_readonly_static("CATEGORY", &mx::PropertyAssign::CATEGORY);

    py::class_<mx::PropertySet, mx::PropertySetPtr, mx::Element>(mod, "PropertySet", "A property set element within a Document.")
        .def("addProperty", &mx::PropertySet::addProperty, "Add a Property to the set.\n\nArgs:\n    name: The name of the new Property. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new Property.")
        .def("getProperties", &mx::PropertySet::getProperties, "Return a vector of all Property elements in the set.")
        .def("removeProperty", &mx::PropertySet::removeProperty, "Remove the Property with the given name, if present.")
        .def("_getPropertyValue", &mx::PropertySet::getPropertyValue, "Return the typed value of a property by its name.\n\nArgs:\n    name: The name of the property to be evaluated.\n\nReturns:\n    If the given property is found, then a shared pointer to its value is returned; otherwise, an empty shared pointer is returned.")
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
        .def_readonly_static("CATEGORY", &mx::Property::CATEGORY);

    py::class_<mx::PropertySetAssign, mx::PropertySetAssignPtr, mx::GeomElement>(mod, "PropertySetAssign", "A property set assignment element within a Look.")
        .def("setPropertySetString", &mx::PropertySetAssign::setPropertySetString, "Set the property set string of this element.")
        .def("hasPropertySetString", &mx::PropertySetAssign::hasPropertySetString, "Return true if this element has a property set string.")
        .def("getPropertySetString", &mx::PropertySetAssign::getPropertySetString, "Return the property set string of this element.")
        .def("setPropertySet", &mx::PropertySetAssign::setPropertySet, "Assign a property set to this element.")
        .def("getPropertySet", &mx::PropertySetAssign::getPropertySet, "Return the property set that is assigned to this element.")
        .def_readonly_static("CATEGORY", &mx::PropertySetAssign::CATEGORY);
}
