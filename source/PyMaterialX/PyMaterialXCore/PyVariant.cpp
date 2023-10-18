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
        .def_readonly_static("CATEGORY", &mx::Variant::CATEGORY)
        .doc() = R"docstring(
    Class representing a variant element within a `VariantSet`.

    :see: https://materialx.org/docs/api/class_variant.html
)docstring";

    py::class_<mx::VariantSet, mx::VariantSetPtr, mx::Element>(mod, "VariantSet")
        .def("addVariant", &mx::VariantSet::addVariant,
            py::arg("name") = mx::EMPTY_STRING)
        .def("getVariant", &mx::VariantSet::getVariant)
        .def("getVariants", &mx::VariantSet::getVariants)
        .def("removeVariant", &mx::VariantSet::removeVariant)
        .def_readonly_static("CATEGORY", &mx::VariantSet::CATEGORY)
        .doc() = R"docstring(
    Class representing a variant set element within a `Document`.

    :see: https://materialx.org/docs/api/class_variant_set.html
)docstring";

    py::class_<mx::VariantAssign, mx::VariantAssignPtr, mx::Element>(mod, "VariantAssign")
        .def("setVariantSetString", &mx::VariantAssign::setVariantSetString)
        .def("hasVariantSetString", &mx::VariantAssign::hasVariantSetString)
        .def("getVariantSetString", &mx::VariantAssign::getVariantSetString)
        .def("setVariantString", &mx::VariantAssign::setVariantString)
        .def("hasVariantString", &mx::VariantAssign::hasVariantString)
        .def("getVariantString", &mx::VariantAssign::getVariantString)
        .def_readonly_static("CATEGORY", &mx::VariantAssign::CATEGORY)
        .doc() = R"docstring(
    Class representing a variant assignment element within a `Look`.

    :see: https://materialx.org/docs/api/class_variant_assign.html
)docstring";
}
