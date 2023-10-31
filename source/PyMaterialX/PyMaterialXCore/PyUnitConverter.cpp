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
    py::class_<mx::UnitConverter, PyUnitConverter, mx::UnitConverterPtr>(mod, "UnitConverter")

        .def("convert", (float       (mx::UnitConverter::*)(float, const std::string&, const std::string&)const) &mx::UnitConverter::convert,
             py::arg("input"),
             py::arg("inputUnit"),
             py::arg("outputUnit"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Convert the given `input` of type `float` from the given `inputUnit` to the
    desired `outputUnit`.

    :type input: float
    :param input: The input value to convert.
    :type inputUnit: str
    :param inputUnit: Unit of input value.
    :type outputUnit: str
    :param outputUnit: Unit for output value.
)docstring"))

        .def("convert", (mx::Vector2 (mx::UnitConverter::*)(const mx::Vector2&, const std::string&, const std::string&)const) &mx::UnitConverter::convert,
             py::arg("input"),
             py::arg("inputUnit"),
             py::arg("outputUnit"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Convert the given `input` of type `Vector2` from the given `inputUnit` to
    the desired `outputUnit`.

    :type input: `Vector2`
    :param input: The input value to convert.
    :type inputUnit: str
    :param inputUnit: Unit of input value.
    :type outputUnit: str
    :param outputUnit: Unit for output value.
)docstring"))

        .def("convert", (mx::Vector3 (mx::UnitConverter::*)(const mx::Vector3&, const std::string&, const std::string&)const) &mx::UnitConverter::convert,
             py::arg("input"),
             py::arg("inputUnit"),
             py::arg("outputUnit"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Convert the given `input` of type `Vector3` from the given `inputUnit` to
    the desired `outputUnit`.

    :type input: `Vector3`
    :param input: The input value to convert.
    :type inputUnit: str
    :param inputUnit: Unit of input value.
    :type outputUnit: str
    :param outputUnit: Unit for output value.
)docstring"))

        .def("convert", (mx::Vector4 (mx::UnitConverter::*)(const mx::Vector4&, const std::string&, const std::string&)const) &mx::UnitConverter::convert,
             py::arg("input"),
             py::arg("inputUnit"),
             py::arg("outputUnit"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Convert the given `input` of type `Vector4` from the given `inputUnit` to
    the desired `outputUnit`.

    :type input: `Vector4`
    :param input: The input value to convert.
    :type inputUnit: str
    :param inputUnit: Unit of input value.
    :type outputUnit: str
    :param outputUnit: Unit for output value.
)docstring"))

        .def("getUnitAsInteger", &mx::UnitConverter::getUnitAsInteger,
             py::arg("unitName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Given a unit name, return a value that it can map to as an integer.

    Returns `-1` if no unit of the given name was found.
)docstring"))

        .def("getUnitFromInteger", &mx::UnitConverter::getUnitFromInteger,
             py::arg("index"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Given an integer index, return the unit name in the map used by the
    converter.

    Returns an empty string if no unit of the given index was found.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Abstract base class for unit converters.

    Each unit converter instance is responsible for a single unit type.

    :see: https://materialx.org/docs/api/class_unit_converter.html
)docstring");

    py::class_<mx::LinearUnitConverter, mx::UnitConverter, mx::LinearUnitConverterPtr>(mod, "LinearUnitConverter")

        .def_static("create", &mx::LinearUnitConverter::create,
                    py::arg("unitTypeDef"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a `LinearUnitConverter` based on a given `UnitTypeDef`.
)docstring"))

        .def("getUnitScale", &mx::LinearUnitConverter::getUnitScale,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the mappings from unit names to the scale value defined by a linear
    converter.
)docstring"))

        .def("convert", (float       (mx::LinearUnitConverter::*)(float, const std::string&, const std::string&)const) &mx::LinearUnitConverter::convert,
             py::arg("input"),
             py::arg("inputUnit"),
             py::arg("outputUnit"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Convert the given `input` of type `float` from the given `inputUnit` to the
    desired `outputUnit`.

    :type input: float
    :param input: The input value to convert.
    :type inputUnit: str
    :param inputUnit: Unit of input value.
    :type outputUnit: str
    :param outputUnit: Unit for output value.
)docstring"))

        .def("convert", (mx::Vector2 (mx::LinearUnitConverter::*)(const mx::Vector2&, const std::string&, const std::string&)const) &mx::LinearUnitConverter::convert,
             py::arg("input"),
             py::arg("inputUnit"),
             py::arg("outputUnit"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Convert the given `input` of type `Vector2` from the given `inputUnit` to
    the desired `outputUnit`.

    :type input: `Vector2`
    :param input: The input value to convert.
    :type inputUnit: str
    :param inputUnit: Unit of input value.
    :type outputUnit: str
    :param outputUnit: Unit for output value.
)docstring"))

        .def("convert", (mx::Vector3 (mx::LinearUnitConverter::*)(const mx::Vector3&, const std::string&, const std::string&)const) &mx::LinearUnitConverter::convert,
             py::arg("input"),
             py::arg("inputUnit"),
             py::arg("outputUnit"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Convert the given `input` of type `Vector3` from the given `inputUnit` to
    the desired `outputUnit`.

    :type input: `Vector3`
    :param input: The input value to convert.
    :type inputUnit: str
    :param inputUnit: Unit of input value.
    :type outputUnit: str
    :param outputUnit: Unit for output value.
)docstring"))

        .def("convert", (mx::Vector4 (mx::LinearUnitConverter::*)(const mx::Vector4&, const std::string&, const std::string&)const) &mx::LinearUnitConverter::convert,
             py::arg("input"),
             py::arg("inputUnit"),
             py::arg("outputUnit"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Convert the given `input` of type `Vector4` from the given `inputUnit` to
    the desired `outputUnit`.

    :type input: `Vector4`
    :param input: The input value to convert.
    :type inputUnit: str
    :param inputUnit: Unit of input value.
    :type outputUnit: str
    :param outputUnit: Unit for output value.
)docstring"))

        .def("getUnitAsInteger", &mx::LinearUnitConverter::getUnitAsInteger,
             py::arg("unitName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Given a unit name, return a value that it can map to as an integer.

    Returns `-1` if no unit of the given name was found.
)docstring"))

        .def("getUnitFromInteger", &mx::LinearUnitConverter::getUnitFromInteger,
             py::arg("index"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Given an integer index, return the unit name in the map used by the
    converter.

    Returns an empty string if no unit of the given index was found.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A `UnitConverter` class for linear units that require only a scalar
    multiplication.

    :see: https://materialx.org/docs/api/class_linear_unit_converter.html
)docstring");

    py::class_<mx::UnitConverterRegistry, mx::UnitConverterRegistryPtr>(mod, "UnitConverterRegistry")

        .def_static("create", &mx::UnitConverterRegistry::create,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a `UnitConverterRegistry`.
)docstring"))

        .def("addUnitConverter", &mx::UnitConverterRegistry::addUnitConverter,
             py::arg("def"),
             py::arg("converter"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a unit converter for a given `UnitTypeDef`.

    :type def: `UnitTypeDef`
    :param def: An object describing the unit type for which to add the given
        converter.
    :type converter: `UnitConverter`
    :param converter: The unit converter to add.
    :returns: `False` if a converter has already been registered for the given
        `UnitTypeDef`.
)docstring"))

        .def("removeUnitConverter", &mx::UnitConverterRegistry::removeUnitConverter,
             py::arg("def"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove a unit converter for a given `UnitTypeDef`.

    :returns: `False` if a converter does not exist for the given `UnitTypeDef`.
)docstring"))

        .def("getUnitConverter", &mx::UnitConverterRegistry::getUnitConverter,
             py::arg("def"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Get a unit converter for a given `UnitTypeDef`.

    Returns `None` if a converter does not exist for the given `UnitTypeDef`.
)docstring"))

        .def("clearUnitConverters", &mx::UnitConverterRegistry::clearUnitConverters,
             PYMATERIALX_DOCSTRING(R"docstring(
    Clear all unit converters from the registry.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing a registry for `UnitConverter` objects.

    :see: https://materialx.org/docs/api/class_unit_converter_registry.html
)docstring");
}
