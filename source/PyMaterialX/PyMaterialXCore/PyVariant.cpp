//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Variant.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyVariant(py::module& mod)
{
    py::class_<mx::Variant, mx::VariantPtr, mx::InterfaceElement>(mod, "Variant")

        .def_readonly_static("CATEGORY", &mx::Variant::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Variant` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a variant element within a `VariantSet`.

    :see: https://materialx.org/docs/api/class_variant.html
)docstring");

    py::class_<mx::VariantSet, mx::VariantSetPtr, mx::Element>(mod, "VariantSet")

        .def("addVariant", &mx::VariantSet::addVariant,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `Variant` to the variant set.

    :type name: str
    :param name: The name of the new `Variant`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `Variant`.
)docstring"))

        .def("getVariant", &mx::VariantSet::getVariant,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Variant`, if any, with the given name.
)docstring"))

        .def("getVariants", &mx::VariantSet::getVariants,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Variant` elements in the look.
)docstring"))

        .def("removeVariant", &mx::VariantSet::removeVariant,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Variant`, if any, with the given name.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::VariantSet::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `VariantSet` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a variant set element within a `Document`.

    :see: https://materialx.org/docs/api/class_variant_set.html
)docstring");

    py::class_<mx::VariantAssign, mx::VariantAssignPtr, mx::Element>(mod, "VariantAssign")

        .def("setVariantSetString", &mx::VariantAssign::setVariantSetString,
             py::arg("variantSet"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the element's variant set string.
)docstring"))

        .def("hasVariantSetString", &mx::VariantAssign::hasVariantSetString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a variant set string.
)docstring"))

        .def("getVariantSetString", &mx::VariantAssign::getVariantSetString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element's variant set string.
)docstring"))

        .def("setVariantString", &mx::VariantAssign::setVariantString,
             py::arg("variant"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the element's variant string.
)docstring"))

        .def("hasVariantString", &mx::VariantAssign::hasVariantString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a variant string.
)docstring"))

        .def("getVariantString", &mx::VariantAssign::getVariantString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element's variant string.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::VariantAssign::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `VariantAssign` elements within a
    MaterialX document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("VARIANT_SET_ATTRIBUTE",
                             &mx::VariantAssign::VARIANT_SET_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's variant set string is stored as an
    attribute.

    :see: `setVariantSetString()`
    :see: `hasVariantSetString()`
    :see: `getVariantSetString()`
)docstring"))

        .def_readonly_static("VARIANT_ATTRIBUTE",
                             &mx::VariantAssign::VARIANT_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's variant string is stored as an attribute.

    :see: `setVariantString()`
    :see: `hasVariantString()`
    :see: `getVariantString()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a variant assignment element within a `Look`.

    :see: https://materialx.org/docs/api/class_variant_assign.html
)docstring");
}
