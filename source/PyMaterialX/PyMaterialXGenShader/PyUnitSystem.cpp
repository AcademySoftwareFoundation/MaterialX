//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/UnitSystem.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGraph.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyUnitSystem(py::module& mod)
{
    py::class_<mx::UnitTransform>(mod, "UnitTransform")

        .def(py::init<const std::string&, const std::string&, const mx::TypeDesc*, const std::string&>(),
             py::arg("sourceUnit"),
             py::arg("targetUnit"),
             py::arg("typeDesc"),
             py::arg("unitType"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class using the given source unit, target
    unit, type descriptor, and unit type name.
)docstring"))

        .def_readwrite("sourceUnit", &mx::UnitTransform::sourceUnit,
                       PYMATERIALX_DOCSTRING(R"docstring(
    The source unit of the unit transform.
)docstring"))

        .def_readwrite("targetUnit", &mx::UnitTransform::targetUnit,
                       PYMATERIALX_DOCSTRING(R"docstring(
    The target unit of the unit transform.
)docstring"))

        .def_readwrite("typeDesc", &mx::UnitTransform::type,
                       PYMATERIALX_DOCSTRING(R"docstring(
    The type descriptor of the unit transform.
)docstring"))

        .def_readwrite("unitType", &mx::UnitTransform::type,
                       PYMATERIALX_DOCSTRING(R"docstring(
    The unit type name of the unit transform.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing a structure that represents unit transform information.

    :see: https://materialx.org/docs/api/struct_unit_transform.html
)docstring");

    py::class_<mx::UnitSystem, mx::UnitSystemPtr>(mod, "UnitSystem")

        .def_static("create", &mx::UnitSystem::create,
                    py::arg("language"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class using the given language.
)docstring"))

        .def("getName", &mx::UnitSystem::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the UnitSystem name.
)docstring"))

        .def("loadLibrary", &mx::UnitSystem::loadLibrary,
             py::arg("document"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Assign document with unit implementations replacing any previously loaded
    content.
)docstring"))

        .def("supportsTransform", &mx::UnitSystem::supportsTransform,
             py::arg("transform"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Returns whether this unit system supports the given `transform`.
)docstring"))

        .def("setUnitConverterRegistry", &mx::UnitSystem::setUnitConverterRegistry,
             py::arg("registry"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Assign unit converter registry replacing any previous assignment.
)docstring"))

        .def("getUnitConverterRegistry", &mx::UnitSystem::getUnitConverterRegistry,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the currently assigned unit converter registry.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing base unit system support.

    :see: https://materialx.org/docs/api/class_unit_system.html
)docstring");
}
