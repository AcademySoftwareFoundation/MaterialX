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
    py::class_<mx::UnitTransform>(mod, "UnitTransform", "Structure that represents unit transform information.")
        .def(py::init<const std::string&, const std::string&, mx::TypeDesc, const std::string&>())
        .def_readwrite("sourceUnit", &mx::UnitTransform::sourceUnit)
        .def_readwrite("targetUnit", &mx::UnitTransform::targetUnit)
        .def_readwrite("type", &mx::UnitTransform::type)
        .def_readwrite("unitType", &mx::UnitTransform::type);

    py::class_<mx::UnitSystem, mx::UnitSystemPtr>(mod, "UnitSystem", "Base unit system support.")
        .def_static("create", &mx::UnitSystem::create)
        .def("getName", &mx::UnitSystem::getName, "Return the ColorManagementSystem name.")
        .def("loadLibrary", &mx::UnitSystem::loadLibrary, "Load a library of implementations from the provided document, replacing any previously loaded content.")
        .def("supportsTransform", &mx::UnitSystem::supportsTransform, "Returns whether this color management system supports a provided transform.")
        .def("setUnitConverterRegistry", &mx::UnitSystem::setUnitConverterRegistry, "Assign unit converter registry replacing any previous assignment.")
        .def("getUnitConverterRegistry", &mx::UnitSystem::getUnitConverterRegistry, "Returns the currently assigned unit converter registry.");
}
