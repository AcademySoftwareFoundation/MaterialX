//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Look.h>

#include <PyBind11/stl.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_LOOK_FUNC_INSTANCE(NAME, T) \
.def("_setPropertyValue" #NAME, &mx::Look::setPropertyValue<T>, py::arg("name"), py::arg("value"), py::arg("type") = mx::EMPTY_STRING)

void bindPyLook(py::module& mod)
{
    py::class_<mx::Look, mx::LookPtr, mx::Element>(mod, "Look", py::metaclass())
        .def("addLookInherit", &mx::Look::addLookInherit,
            py::arg("name") = mx::EMPTY_STRING)
        .def("getLookInherit", &mx::Look::getLookInherit)
        .def("getLookInherits", &mx::Look::getLookInherits)
        .def("removeLookInherit", &mx::Look::removeLookInherit)
        .def("addMaterialAssign", &mx::Look::addMaterialAssign,
            py::arg("name") = mx::EMPTY_STRING, py::arg("material") = mx::EMPTY_STRING)
        .def("getMaterialAssign", &mx::Look::getMaterialAssign)
        .def("getMaterialAssigns", &mx::Look::getMaterialAssigns)
        .def("removeMaterialAssign", &mx::Look::removeMaterialAssign)
        .def("addPropertyAssign", &mx::Look::addPropertyAssign,
            py::arg("name") = mx::EMPTY_STRING)
        .def("getPropertyAssign", &mx::Look::getPropertyAssign)
        .def("getPropertyAssigns", &mx::Look::getPropertyAssigns)
        .def("removePropertyAssign", &mx::Look::removePropertyAssign)
        .def("addPropertySetAssign", &mx::Look::addPropertySetAssign,
            py::arg("name") = mx::EMPTY_STRING)
        .def("getPropertySetAssign", &mx::Look::getPropertySetAssign)
        .def("getPropertySetAssigns", &mx::Look::getPropertySetAssigns)
        .def("removePropertySetAssign", &mx::Look::removePropertySetAssign)
        .def("addVisibility", &mx::Look::addVisibility,
            py::arg("name") = mx::EMPTY_STRING)
        .def("getVisibility", &mx::Look::getVisibility)
        .def("getVisibilities", &mx::Look::getVisibilities)
        .def("removeVisibility", &mx::Look::removeVisibility)
        .def("setInheritsFrom", &mx::Look::setInheritsFrom)
        .def("getInheritsFrom", &mx::Look::getInheritsFrom)
        .def_readonly_static("CATEGORY", &mx::Look::CATEGORY);

    py::class_<mx::LookInherit, mx::LookInheritPtr, mx::Element>(mod, "LookInherit", py::metaclass())
        .def_readonly_static("CATEGORY", &mx::Look::CATEGORY);

    py::class_<mx::MaterialAssign, mx::MaterialAssignPtr, mx::GeomElement>(mod, "MaterialAssign", py::metaclass())
        .def("setExclusive", &mx::MaterialAssign::setExclusive)
        .def("getExclusive", &mx::MaterialAssign::getExclusive)
        .def("getReferencedMaterial", &mx::MaterialAssign::getReferencedMaterial)
        .def_readonly_static("CATEGORY", &mx::MaterialAssign::CATEGORY);

    py::class_<mx::Visibility, mx::VisibilityPtr, mx::GeomElement>(mod, "Visibility", py::metaclass())
        .def("setViewerGeom", &mx::Visibility::setViewerGeom)
        .def("hasViewerGeom", &mx::Visibility::hasViewerGeom)
        .def("getViewerGeom", &mx::Visibility::getViewerGeom)
        .def("setViewerCollection", &mx::Visibility::setViewerCollection)
        .def("hasViewerCollection", &mx::Visibility::hasViewerCollection)
        .def("getViewerCollection", &mx::Visibility::getViewerCollection)
        .def("setVisibilityType", &mx::Visibility::setVisibilityType)
        .def("hasVisibilityType", &mx::Visibility::hasVisibilityType)
        .def("getVisibilityType", &mx::Visibility::getVisibilityType)
        .def("setVisible", &mx::Visibility::setVisible)
        .def("getVisible", &mx::Visibility::getVisible)
        .def_readonly_static("CATEGORY", &mx::Visibility::CATEGORY);
}
