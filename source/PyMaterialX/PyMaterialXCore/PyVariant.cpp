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
    py::class_<mx::Variant, mx::VariantPtr, mx::InterfaceElement>(mod, "Variant", "A variant element within a VariantSet.")
        .def_readonly_static("CATEGORY", &mx::Variant::CATEGORY);

    py::class_<mx::VariantSet, mx::VariantSetPtr, mx::Element>(mod, "VariantSet", "A variant set element within a Document.")
        .def("addVariant", &mx::VariantSet::addVariant,
            py::arg("name") = mx::EMPTY_STRING, "Add a Variant to the variant set.\n\nArgs:\n    name: The name of the new Variant. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new Variant.")
        .def("getVariant", &mx::VariantSet::getVariant, "Return the Variant, if any, with the given name.")
        .def("getVariants", &mx::VariantSet::getVariants, "Return a vector of all Variant elements in the look.")
        .def("removeVariant", &mx::VariantSet::removeVariant, "Remove the Variant, if any, with the given name.")
        .def_readonly_static("CATEGORY", &mx::VariantSet::CATEGORY);

    py::class_<mx::VariantAssign, mx::VariantAssignPtr, mx::Element>(mod, "VariantAssign", "A variant assignment element within a Look.")
        .def("setVariantSetString", &mx::VariantAssign::setVariantSetString, "Set the element's variant set string.")
        .def("hasVariantSetString", &mx::VariantAssign::hasVariantSetString, "Return true if the given element has a variant set string.")
        .def("getVariantSetString", &mx::VariantAssign::getVariantSetString, "Return the element's variant set string.")
        .def("setVariantString", &mx::VariantAssign::setVariantString, "Set the element's variant string.")
        .def("hasVariantString", &mx::VariantAssign::hasVariantString, "Return true if the given element has a variant string.")
        .def("getVariantString", &mx::VariantAssign::getVariantString, "Return the element's variant string.")
        .def_readonly_static("CATEGORY", &mx::VariantAssign::CATEGORY);
}
