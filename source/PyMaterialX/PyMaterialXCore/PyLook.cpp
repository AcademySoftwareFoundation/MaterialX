//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Look.h>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_LOOK_FUNC_INSTANCE(NAME, T) \
.def("_setPropertyValue" #NAME, &mx::Look::setPropertyValue<T>, py::arg("name"), py::arg("value"), py::arg("type") = mx::EMPTY_STRING)

void bindPyLook(py::module& mod)
{
    py::class_<mx::Look, mx::LookPtr, mx::Element>(mod, "Look", "A look element within a Document.")
        .def("addMaterialAssign", &mx::Look::addMaterialAssign, py::arg("name") = mx::EMPTY_STRING, py::arg("material") = mx::EMPTY_STRING, "Add a MaterialAssign to the look.\n\nArgs:\n    name: The name of the new MaterialAssign. If no name is specified, then a unique name will automatically be generated.\n    material: An optional material string, which should match the name of the material node to be assigned.\n\nReturns:\n    A shared pointer to the new MaterialAssign.")
        .def("getMaterialAssign", &mx::Look::getMaterialAssign, "Return the MaterialAssign, if any, with the given name.")
        .def("getMaterialAssigns", &mx::Look::getMaterialAssigns, "Return a vector of all MaterialAssign elements in the look.")
        .def("getActiveMaterialAssigns", &mx::Look::getActiveMaterialAssigns, "Return a vector of all MaterialAssign elements that belong to this look, taking look inheritance into account.")
        .def("removeMaterialAssign", &mx::Look::removeMaterialAssign, "Remove the MaterialAssign, if any, with the given name.")
        .def("addPropertyAssign", &mx::Look::addPropertyAssign, py::arg("name") = mx::EMPTY_STRING, "Add a PropertyAssign to the look.\n\nArgs:\n    name: The name of the new PropertyAssign. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new PropertyAssign.")
        .def("getPropertyAssign", &mx::Look::getPropertyAssign, "Return the PropertyAssign, if any, with the given name.")
        .def("getPropertyAssigns", &mx::Look::getPropertyAssigns, "Return a vector of all PropertyAssign elements in the look.")
        .def("getActivePropertyAssigns", &mx::Look::getActivePropertyAssigns, "Return a vector of all PropertyAssign elements that belong to this look, taking look inheritance into account.")
        .def("removePropertyAssign", &mx::Look::removePropertyAssign, "Remove the PropertyAssign, if any, with the given name.")
        .def("addPropertySetAssign", &mx::Look::addPropertySetAssign, py::arg("name") = mx::EMPTY_STRING, "Add a PropertySetAssign to the look.\n\nArgs:\n    name: The name of the new PropertySetAssign. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new PropertySetAssign.")
        .def("getPropertySetAssign", &mx::Look::getPropertySetAssign, "Return the PropertySetAssign, if any, with the given name.")
        .def("getPropertySetAssigns", &mx::Look::getPropertySetAssigns, "Return a vector of all PropertySetAssign elements in the look.")
        .def("getActivePropertySetAssigns", &mx::Look::getActivePropertySetAssigns, "Return a vector of all PropertySetAssign elements that belong to this look, taking look inheritance into account.")
        .def("removePropertySetAssign", &mx::Look::removePropertySetAssign, "Remove the PropertySetAssign, if any, with the given name.")
        .def("addVariantAssign", &mx::Look::addVariantAssign, py::arg("name") = mx::EMPTY_STRING, "Add a VariantAssign to the look.\n\nArgs:\n    name: The name of the new VariantAssign. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new VariantAssign.")
        .def("getVariantAssign", &mx::Look::getVariantAssign, "Return the VariantAssign, if any, with the given name.")
        .def("getVariantAssigns", &mx::Look::getVariantAssigns, "Return a vector of all VariantAssign elements in the look.")
        .def("getActiveVariantAssigns", &mx::Look::getActiveVariantAssigns, "Return a vector of all VariantAssign elements that belong to this look, taking look inheritance into account.")
        .def("removeVariantAssign", &mx::Look::removeVariantAssign, "Remove the VariantAssign, if any, with the given name.")
        .def("addVisibility", &mx::Look::addVisibility, py::arg("name") = mx::EMPTY_STRING, "Add a Visibility to the look.\n\nArgs:\n    name: The name of the new Visibility. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new Visibility.")
        .def("getVisibility", &mx::Look::getVisibility, "Return the Visibility, if any, with the given name.")
        .def("getVisibilities", &mx::Look::getVisibilities, "Return a vector of all Visibility elements in the look.")
        .def("getActiveVisibilities", &mx::Look::getActiveVisibilities, "Return a vector of all Visibility elements that belong to this look, taking look inheritance into account.")
        .def("removeVisibility", &mx::Look::removeVisibility, "Remove the Visibility, if any, with the given name.")
        .def_readonly_static("CATEGORY", &mx::Look::CATEGORY);

    py::class_<mx::LookGroup, mx::LookGroupPtr, mx::Element>(mod, "LookGroup", "A look group element within a Document.")
        .def("getLooks", &mx::LookGroup::getLooks, "Get comma-separated list of looks.")
        .def("setLooks", &mx::LookGroup::setLooks, "Set comma-separated list of looks.")
        .def("getActiveLook", &mx::LookGroup::getActiveLook, "Return the active look, if any.")
        .def("setActiveLook", &mx::LookGroup::setActiveLook, "Set the active look.")
        .def_readonly_static("CATEGORY", &mx::LookGroup::CATEGORY)
        .def_readonly_static("LOOKS_ATTRIBUTE", &mx::LookGroup::LOOKS_ATTRIBUTE)
        .def_readonly_static("ACTIVE_ATTRIBUTE", &mx::LookGroup::ACTIVE_ATTRIBUTE);

    py::class_<mx::MaterialAssign, mx::MaterialAssignPtr, mx::GeomElement>(mod, "MaterialAssign", "A material assignment element within a Look.")
        .def("setMaterial", &mx::MaterialAssign::setMaterial, "Set the material string for the MaterialAssign.")
        .def("hasMaterial", &mx::MaterialAssign::hasMaterial, "Return true if the given MaterialAssign has a material string.")
        .def("getMaterial", &mx::MaterialAssign::getMaterial, "Return the material string for the MaterialAssign.")
        .def("getMaterialOutputs", &mx::MaterialAssign::getMaterialOutputs, "Return the outputs on any referenced material.")        
        .def("setExclusive", &mx::MaterialAssign::setExclusive, "Set the exclusive boolean for the MaterialAssign.")
        .def("getExclusive", &mx::MaterialAssign::getExclusive, "Return the exclusive boolean for the MaterialAssign.")
        .def("getReferencedMaterial", &mx::MaterialAssign::getReferencedMaterial, "Return the material node, if any, referenced by the MaterialAssign.")
        .def_readonly_static("CATEGORY", &mx::MaterialAssign::CATEGORY);

    py::class_<mx::Visibility, mx::VisibilityPtr, mx::GeomElement>(mod, "Visibility", "A visibility element within a Look.\n\nA Visibility describes the visibility relationship between two geometries or geometric collections.")
        .def("setViewerGeom", &mx::Visibility::setViewerGeom, "Set the viewer geom string of the element.")
        .def("hasViewerGeom", &mx::Visibility::hasViewerGeom, "Return true if the given element has a viewer geom string.")
        .def("getViewerGeom", &mx::Visibility::getViewerGeom, "Return the viewer geom string of the element.")
        .def("setViewerCollection", &mx::Visibility::setViewerCollection, "Set the viewer geom string of the element.")
        .def("hasViewerCollection", &mx::Visibility::hasViewerCollection, "Return true if the given element has a viewer collection string.")
        .def("getViewerCollection", &mx::Visibility::getViewerCollection, "Return the viewer collection string of the element.")
        .def("setVisibilityType", &mx::Visibility::setVisibilityType, "Set the visibility type string of the element.")
        .def("hasVisibilityType", &mx::Visibility::hasVisibilityType, "Return true if the given element has a visibility type string.")
        .def("getVisibilityType", &mx::Visibility::getVisibilityType, "Return the visibility type string of the element.")
        .def("setVisible", &mx::Visibility::setVisible, "Set the visible boolean of the element.")
        .def("getVisible", &mx::Visibility::getVisible, "Return the visible boolean of the element.")
        .def_readonly_static("CATEGORY", &mx::Visibility::CATEGORY);

    mod.def("getGeometryBindings", &mx::getGeometryBindings, py::arg("materialNode"), py::arg("geom") = mx::UNIVERSAL_GEOM_NAME, "Return a vector of all MaterialAssign elements that bind this material node to the given geometry string.\n\nArgs:\n    materialNode: Node to examine\n    geom: The geometry for which material bindings should be returned. By default, this argument is the universal geometry string \"/\", and all material bindings are returned.\n\nReturns:\n    Vector of MaterialAssign elements");
}
