//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Unit.h>

namespace py = pybind11;
namespace mx = MaterialX;

class PyUnitConverter : public mx::UnitConverter
{
  public:
    float convert(float input, const std::string& inputUnit, const std::string& outputUnit) const override
    {
        PYBIND11_OVERLOAD_PURE(
            float,
            mx::UnitConverter,
            convert,
            input,
            inputUnit,
            outputUnit
        );
    }

    mx::Vector2 convert(const mx::Vector2& input, const std::string& inputUnit, const std::string& outputUnit) const override
    {
        PYBIND11_OVERLOAD_PURE(
            mx::Vector2,
            mx::UnitConverter,
            convert,
            input,
            inputUnit,
            outputUnit
        );
    }

    mx::Vector3 convert(const mx::Vector3& input, const std::string& inputUnit, const std::string& outputUnit) const override
    {
        PYBIND11_OVERLOAD_PURE(
            mx::Vector3,
            mx::UnitConverter,
            convert,
            input,
            inputUnit,
            outputUnit
        );
    }

    mx::Vector4 convert(const mx::Vector4& input, const std::string& inputUnit, const std::string& outputUnit) const override
    {
        PYBIND11_OVERLOAD_PURE(
            mx::Vector4,
            mx::UnitConverter,
            convert,
            input,
            inputUnit,
            outputUnit
        );
    }
};

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

void bindPyUnitConverters(py::module& mod)
{
    py::class_<mx::UnitConverter, PyUnitConverter, mx::UnitConverterPtr>(mod, "UnitConverter", "An abstract base class for unit converters.\n\nEach unit converter instance is responsible for a single unit type.")
        .def("convert", (float       (mx::UnitConverter::*)(float, const std::string&, const std::string&)const) &mx::UnitConverter::convert, "Convert a given value in a given unit to a desired unit.\n\nArgs:\n    input: Input value to convert\n    inputUnit: Unit of input value\n    outputUnit: Unit for output value")
        .def("convert", (mx::Vector2 (mx::UnitConverter::*)(const mx::Vector2&, const std::string&, const std::string&)const) &mx::UnitConverter::convert, "Convert a given value in a given unit to a desired unit.\n\nArgs:\n    input: Input value to convert\n    inputUnit: Unit of input value\n    outputUnit: Unit for output value")
        .def("convert", (mx::Vector3 (mx::UnitConverter::*)(const mx::Vector3&, const std::string&, const std::string&)const) &mx::UnitConverter::convert, "Convert a given value in a given unit to a desired unit.\n\nArgs:\n    input: Input value to convert\n    inputUnit: Unit of input value\n    outputUnit: Unit for output value")
        .def("convert", (mx::Vector4 (mx::UnitConverter::*)(const mx::Vector4&, const std::string&, const std::string&)const) &mx::UnitConverter::convert, "Convert a given value in a given unit to a desired unit.\n\nArgs:\n    input: Input value to convert\n    inputUnit: Unit of input value\n    outputUnit: Unit for output value")
        .def("getUnitAsInteger", &mx::UnitConverter::getUnitAsInteger, "Given a unit name return a value that it can map to as an integer Returns -1 value if not found.")
        .def("getUnitFromInteger", &mx::UnitConverter::getUnitFromInteger, "Given an integer index return the unit name in the map used by the converter Returns Empty string if not found.");

    py::class_<mx::LinearUnitConverter, mx::UnitConverter, mx::LinearUnitConverterPtr>(mod, "LinearUnitConverter", "A converter class for linear units that require only a scalar multiplication.")
        .def_static("create", &mx::LinearUnitConverter::create, "Creator.")
        .def("getUnitScale", &mx::LinearUnitConverter::getUnitScale, "Return the mappings from unit names to the scale value defined by a linear converter.")
        .def("convert", (float       (mx::LinearUnitConverter::*)(float, const std::string&, const std::string&)const) &mx::LinearUnitConverter::convert, "Convert a given value in a given unit to a desired unit.\n\nArgs:\n    input: Input value to convert\n    inputUnit: Unit of input value\n    outputUnit: Unit for output value")
        .def("convert", (mx::Vector2 (mx::LinearUnitConverter::*)(const mx::Vector2&, const std::string&, const std::string&)const) &mx::LinearUnitConverter::convert, "Convert a given value in a given unit to a desired unit.\n\nArgs:\n    input: Input value to convert\n    inputUnit: Unit of input value\n    outputUnit: Unit for output value")
        .def("convert", (mx::Vector3 (mx::LinearUnitConverter::*)(const mx::Vector3&, const std::string&, const std::string&)const) &mx::LinearUnitConverter::convert, "Convert a given value in a given unit to a desired unit.\n\nArgs:\n    input: Input value to convert\n    inputUnit: Unit of input value\n    outputUnit: Unit for output value")
        .def("convert", (mx::Vector4 (mx::LinearUnitConverter::*)(const mx::Vector4&, const std::string&, const std::string&)const) &mx::LinearUnitConverter::convert, "Convert a given value in a given unit to a desired unit.\n\nArgs:\n    input: Input value to convert\n    inputUnit: Unit of input value\n    outputUnit: Unit for output value")
        .def("getUnitAsInteger", &mx::LinearUnitConverter::getUnitAsInteger, "Given a unit name return a value that it can map to as an integer.\n\nReturns -1 value if not found")
        .def("getUnitFromInteger", &mx::LinearUnitConverter::getUnitFromInteger, "Given an integer index return the unit name in the map used by the converter.\n\nReturns Empty string if not found");

    py::class_<mx::UnitConverterRegistry, mx::UnitConverterRegistryPtr>(mod, "UnitConverterRegistry", "A registry for unit converters.")
        .def_static("create", &mx::UnitConverterRegistry::create, "Creator.")
        .def("addUnitConverter", &mx::UnitConverterRegistry::addUnitConverter, "Add a unit converter for a given UnitDef.\n\nReturns false if a converter has already been registered for the given UnitDef")
        .def("removeUnitConverter", &mx::UnitConverterRegistry::removeUnitConverter, "Remove a unit converter for a given UnitDef.\n\nReturns false if a converter does not exist for the given UnitDef")
        .def("getUnitConverter", &mx::UnitConverterRegistry::getUnitConverter, "Get a unit converter for a given UnitDef Returns any empty pointer if a converter does not exist for the given UnitDef.")
        .def("clearUnitConverters", &mx::UnitConverterRegistry::clearUnitConverters, "Clear all unit converters from the registry.");
}
