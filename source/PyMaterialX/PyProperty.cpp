//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Property.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_PROPERTYSET_TYPE_INSTANCE(NAME, T) \
.def("_setPropertyValue" #NAME, &mx::PropertySet::setPropertyValue<T>, py::arg("name"), py::arg("value"), py::arg("type") = mx::EMPTY_STRING)

void bindPyProperty(py::module& mod)
{
    py::class_<mx::Property, mx::PropertyPtr, mx::ValueElement>(mod, "Property")
        .def_readonly_static("CATEGORY", &mx::Property::CATEGORY);

    py::class_<mx::PropertyAssign, mx::PropertyAssignPtr, mx::ValueElement>(mod, "PropertyAssign")
        .def("setGeom", &mx::PropertyAssign::setGeom)
        .def("hasGeom", &mx::PropertyAssign::hasGeom)
        .def("getGeom", &mx::PropertyAssign::getGeom)
        .def("setCollectionString", &mx::PropertyAssign::setCollectionString)
        .def("hasCollectionString", &mx::PropertyAssign::hasCollectionString)
        .def("getCollectionString", &mx::PropertyAssign::getCollectionString)
        .def("setCollection", &mx::PropertyAssign::setCollection)
        .def("getCollection", &mx::PropertyAssign::getCollection)
        .def_readonly_static("CATEGORY", &mx::PropertyAssign::CATEGORY);

    py::class_<mx::PropertySet, mx::PropertySetPtr, mx::Element>(mod, "PropertySet")
        .def("addProperty", &mx::PropertySet::addProperty)
        .def("getProperties", &mx::PropertySet::getProperties)
        .def("removeProperty", &mx::PropertySet::removeProperty)
        .def("_getPropertyValue", &mx::PropertySet::getPropertyValue)
        BIND_PROPERTYSET_TYPE_INSTANCE(integer, int)
        BIND_PROPERTYSET_TYPE_INSTANCE(boolean, bool)
        BIND_PROPERTYSET_TYPE_INSTANCE(float, float)
        BIND_PROPERTYSET_TYPE_INSTANCE(color2, mx::Color2)
        BIND_PROPERTYSET_TYPE_INSTANCE(color3, mx::Color3)
        BIND_PROPERTYSET_TYPE_INSTANCE(color4, mx::Color4)
        BIND_PROPERTYSET_TYPE_INSTANCE(vector2, mx::Vector2)
        BIND_PROPERTYSET_TYPE_INSTANCE(vector3, mx::Vector3)
        BIND_PROPERTYSET_TYPE_INSTANCE(vector4, mx::Vector4)
        BIND_PROPERTYSET_TYPE_INSTANCE(matrix33, mx::Matrix3x3)
        BIND_PROPERTYSET_TYPE_INSTANCE(matrix44, mx::Matrix4x4)
        BIND_PROPERTYSET_TYPE_INSTANCE(string, std::string)
        .def_readonly_static("CATEGORY", &mx::Property::CATEGORY);

    py::class_<mx::PropertySetAssign, mx::PropertySetAssignPtr, mx::GeomElement>(mod, "PropertySetAssign")
        .def_readonly_static("CATEGORY", &mx::PropertySetAssign::CATEGORY);
}
