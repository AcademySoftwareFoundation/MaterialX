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
    py::class_<mx::Look, mx::LookPtr, mx::Element>(mod, "Look")

        .def("addMaterialAssign", &mx::Look::addMaterialAssign,
             py::arg("name") = mx::EMPTY_STRING,
             py::arg("material") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `MaterialAssign` to the look.

    :type name: str
    :param name: The name of the new `MaterialAssign`.
        If no name is specified, then a unique name will automatically be
        generated.
    :type material: str
    :param material: An optional material string, which should match the
        name of the material node to be assigned.
    :returns: The new `MaterialAssign`.
)docstring"))

        .def("getMaterialAssign", &mx::Look::getMaterialAssign,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `MaterialAssign`, if any, with the given `name`.
)docstring"))

        .def("getMaterialAssigns", &mx::Look::getMaterialAssigns,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `MaterialAssign` elements in the look.
)docstring"))

        .def("getActiveMaterialAssigns", &mx::Look::getActiveMaterialAssigns,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `MaterialAssign` elements that belong to this look,
    taking look inheritance into account.
)docstring"))

        .def("removeMaterialAssign", &mx::Look::removeMaterialAssign,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `MaterialAssign`, if any, with the given name.
)docstring"))

        .def("addPropertyAssign", &mx::Look::addPropertyAssign,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `PropertyAssign` to the look.

    :type name: str
    :param name: The name of the new `PropertyAssign`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `PropertyAssign`.
)docstring"))

        .def("getPropertyAssign", &mx::Look::getPropertyAssign,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `PropertyAssign`, if any, with the given `name`.
)docstring"))

        .def("getPropertyAssigns", &mx::Look::getPropertyAssigns,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `PropertyAssign` elements in the look.
)docstring"))

        .def("getActivePropertyAssigns", &mx::Look::getActivePropertyAssigns,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `PropertyAssign` elements that belong to this look,
    taking look inheritance into account.
)docstring"))

        .def("removePropertyAssign", &mx::Look::removePropertyAssign,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `PropertyAssign`, if any, with the given `name`.
)docstring"))

        .def("addPropertySetAssign", &mx::Look::addPropertySetAssign,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `PropertySetAssign` to the look.

    :type name: str
    :param name: The name of the new `PropertySetAssign`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `PropertySetAssign`.
)docstring"))

        .def("getPropertySetAssign", &mx::Look::getPropertySetAssign,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `PropertySetAssign`, if any, with the given `name`.
)docstring"))

        .def("getPropertySetAssigns", &mx::Look::getPropertySetAssigns,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `PropertySetAssign` elements in the look.
)docstring"))

        .def("getActivePropertySetAssigns", &mx::Look::getActivePropertySetAssigns,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `PropertySetAssign` elements that belong to this look,
    taking look inheritance into account.
)docstring"))

        .def("removePropertySetAssign", &mx::Look::removePropertySetAssign,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `PropertySetAssign`, if any, with the given `name`.
)docstring"))

        .def("addVariantAssign", &mx::Look::addVariantAssign,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `VariantAssign` to the look.

    :type name: str
    :param name: The name of the new `VariantAssign`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `VariantAssign`.
)docstring"))

        .def("getVariantAssign", &mx::Look::getVariantAssign,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `VariantAssign`, if any, with the given `name`.
)docstring"))

        .def("getVariantAssigns", &mx::Look::getVariantAssigns,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `VariantAssign` elements in the look.
)docstring"))

        .def("getActiveVariantAssigns", &mx::Look::getActiveVariantAssigns,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `VariantAssign` elements that belong to this look,
    taking look inheritance into account.
)docstring"))

        .def("removeVariantAssign", &mx::Look::removeVariantAssign,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `VariantAssign`, if any, with the given `name`.
)docstring"))

        .def("addVisibility", &mx::Look::addVisibility,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `Visibility` to the look.

    :type name: str
    :param name: The name of the new `Visibility`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `Visibility`.
)docstring"))

        .def("getVisibility", &mx::Look::getVisibility,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Visibility`, if any, with the given `name`.
)docstring"))

        .def("getVisibilities", &mx::Look::getVisibilities,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Visibility` elements in the look.
)docstring"))

        .def("getActiveVisibilities", &mx::Look::getActiveVisibilities,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Visibility` elements that belong to this look,
    taking look inheritance into account.
)docstring"))

        .def("removeVisibility", &mx::Look::removeVisibility,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Visibility`, if any, with the given `name`.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::Look::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Look` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a look element within a `Document`.

    :see: https://materialx.org/docs/api/class_look.html
)docstring");

    py::class_<mx::LookGroup, mx::LookGroupPtr, mx::Element>(mod, "LookGroup")

        .def("getLooks", &mx::LookGroup::getLooks,
             PYMATERIALX_DOCSTRING(R"docstring(
    Get comma-separated list of looks.
)docstring"))

        .def("setLooks", &mx::LookGroup::setLooks,
             py::arg("looks"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set comma-separated list of looks.
)docstring"))

        .def("getActiveLook", &mx::LookGroup::getActiveLook,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the active look, if any.
)docstring"))

        .def("setActiveLook", &mx::LookGroup::setActiveLook,
             py::arg("look"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the active look.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::LookGroup::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `LookGroup` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("LOOKS_ATTRIBUTE", &mx::LookGroup::LOOKS_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's looks are stored as an attribute.
    Is expected to be a comma-separated list of names of looks.

    :see: `setLooks()`
    :see: `getLooks()`
)docstring"))

        .def_readonly_static("ACTIVE_ATTRIBUTE", &mx::LookGroup::ACTIVE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's active look is stored as an attribute.

    :see: `setActiveLook()`
    :see: `getActiveLook()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a look group element within a `Document`.

    :see: https://materialx.org/docs/api/class_look_group.html
)docstring");

    py::class_<mx::MaterialAssign, mx::MaterialAssignPtr, mx::GeomElement>(mod, "MaterialAssign")

        .def("setMaterial", &mx::MaterialAssign::setMaterial,
             py::arg("material"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the material string for the MaterialAssign.
)docstring"))

        .def("hasMaterial", &mx::MaterialAssign::hasMaterial,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this MaterialAssign has a material string.
)docstring"))

        .def("getMaterial", &mx::MaterialAssign::getMaterial,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the material string for the MaterialAssign.
)docstring"))

        .def("getMaterialOutputs", &mx::MaterialAssign::getMaterialOutputs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the outputs on any referenced material.
)docstring"))

        .def("setExclusive", &mx::MaterialAssign::setExclusive,
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the exclusive flag for the MaterialAssign.
)docstring"))

        .def("getExclusive", &mx::MaterialAssign::getExclusive,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the exclusive flag for the MaterialAssign.
)docstring"))

        .def("getReferencedMaterial", &mx::MaterialAssign::getReferencedMaterial,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the material node, if any, referenced by the MaterialAssign.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::MaterialAssign::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `MaterialAssign` elements within a
    MaterialX document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("MATERIAL_ATTRIBUTE",
                             &mx::MaterialAssign::MATERIAL_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's material string is stored as an attribute.

    :see: `setMaterial()`
    :see: `hasMaterial()`
    :see: `getMaterial()`
)docstring"))

        .def_readonly_static("EXCLUSIVE_ATTRIBUTE",
                             &mx::MaterialAssign::EXCLUSIVE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's exclusive flag is stored as an attribute.

    :see: `setExclusive()`
    :see: `getExclusive()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a material assignment element within a `Look`.

    :see: https://materialx.org/docs/api/class_material_assign.html
)docstring");

    py::class_<mx::Visibility, mx::VisibilityPtr, mx::GeomElement>(mod, "Visibility")

        .def("setViewerGeom", &mx::Visibility::setViewerGeom,
             py::arg("geom"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the viewer geom string of the element.
)docstring"))

        .def("hasViewerGeom", &mx::Visibility::hasViewerGeom,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a viewer geom string.
)docstring"))

        .def("getViewerGeom", &mx::Visibility::getViewerGeom,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the viewer geom string of the element.
)docstring"))

        .def("setViewerCollection", &mx::Visibility::setViewerCollection,
             py::arg("collection"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the viewer collection string of the element.
)docstring"))

        .def("hasViewerCollection", &mx::Visibility::hasViewerCollection,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a viewer collection string.
)docstring"))

        .def("getViewerCollection", &mx::Visibility::getViewerCollection,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the viewer collection string of the element.
)docstring"))

        .def("setVisibilityType", &mx::Visibility::setVisibilityType,
             py::arg("visibilityType"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the visibility type string of the element.
)docstring"))

        .def("hasVisibilityType", &mx::Visibility::hasVisibilityType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a visibility type string.
)docstring"))

        .def("getVisibilityType", &mx::Visibility::getVisibilityType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the visibility type string of the element.
)docstring"))

        .def("setVisible", &mx::Visibility::setVisible,
             py::arg("visible"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the visible flag of the element.
)docstring"))

        .def("getVisible", &mx::Visibility::getVisible,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the visible flag of the element.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::Visibility::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Visibility` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("VIEWER_GEOM_ATTRIBUTE",
                             &mx::Visibility::VIEWER_GEOM_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's viewer geom string is stored as an
    attribute.

    :see: `setViewerGeom()`
    :see: `hasViewerGeom()`
    :see: `getViewerGeom()`
)docstring"))

        .def_readonly_static("VIEWER_COLLECTION_ATTRIBUTE",
                             &mx::Visibility::VIEWER_COLLECTION_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's viewer collection string is stored as an
    attribute.

    :see: `setViewerCollection()`
    :see: `hasViewerCollection()`
    :see: `getViewerCollection()`
)docstring"))

        .def_readonly_static("VISIBILITY_TYPE_ATTRIBUTE",
                             &mx::Visibility::VISIBILITY_TYPE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's visibility type string is stored as an
    attribute.

    :see: `setVisibilityType()`
    :see: `hasVisibilityType()`
    :see: `getVisibilityType()`
)docstring"))

        .def_readonly_static("VISIBLE_ATTRIBUTE",
                             &mx::Visibility::VISIBLE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's visible flag is stored as an attribute.

    :see: `setVisible()`
    :see: `getVisible()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a visibility element within a `Look`.

    A `Visibility` describes the visibility relationship between two geometries
    or geometric collections.

    :see: https://materialx.org/docs/api/class_visibility.html
    :todo: Add a `Look.geomIsVisible()` method that computes the visibility between
        two geometries in the context of a specific `Look`.
)docstring");

    mod.def("getGeometryBindings", &mx::getGeometryBindings,
            py::arg("materialNode"),
            py::arg_v("geom",
                      mx::UNIVERSAL_GEOM_NAME,
                      "mx.UNIVERSAL_GEOM_NAME"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `MaterialAssign` elements that bind the given
    `materialNode` to the given geometry string.

    :type materialNode: `Node`
    :param materialNode: The node to examine.
    :type geom: str
    :param geom: The geometry for which material bindings should be returned.
        By default, this argument is the universal geometry string `"/"`,
        and all material bindings are returned.
    :return: List of `MaterialAssign` elements.
    :see: `UNIVERSAL_GEOM_NAME`
)docstring"));
}
