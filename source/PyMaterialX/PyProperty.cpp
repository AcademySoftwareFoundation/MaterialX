//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Property.h>

#include <PyBind11/stl.h>

namespace py = pybind11;
namespace mx = MaterialX;

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
        .def_readonly_static("CATEGORY", &mx::Property::CATEGORY);

    py::class_<mx::PropertySetAssign, mx::PropertySetAssignPtr, mx::GeomElement>(mod, "PropertySetAssign")
        .def_readonly_static("CATEGORY", &mx::PropertySetAssign::CATEGORY);
}
