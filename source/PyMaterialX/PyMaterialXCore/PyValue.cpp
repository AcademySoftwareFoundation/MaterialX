//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Value.h>

#define BIND_TYPE_INSTANCE(NAME, T)                                                                         \
py::class_<mx::TypedValue<T>, std::shared_ptr< mx::TypedValue<T> >, mx::Value>(mod, "TypedValue_" #NAME)    \
    .def("getData", &mx::TypedValue<T>::getData)                                                            \
    .def("getValueString", &mx::TypedValue<T>::getValueString)                                              \
    .def_static("createValue", &mx::Value::createValue<T>)                                                  \
    .def_readonly_static("TYPE", &mx::TypedValue<T>::TYPE);

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyValue(py::module& mod)
{
    py::class_<mx::Value, mx::ValuePtr>(mod, "Value")
        .def("getValueString", &mx::Value::getValueString)
        .def("getTypeString", &mx::Value::getTypeString)
        .def_static("createValueFromStrings", &mx::Value::createValueFromStrings);
    mod.attr("Value").doc() = R"docstring(
    A generic, discriminated value, whose type may be queried dynamically.

    :see: https://materialx.org/docs/api/class_value.html)docstring";

    BIND_TYPE_INSTANCE(integer, int)
    BIND_TYPE_INSTANCE(boolean, bool)
    BIND_TYPE_INSTANCE(float, float)
    BIND_TYPE_INSTANCE(color3, mx::Color3)
    BIND_TYPE_INSTANCE(color4, mx::Color4)
    BIND_TYPE_INSTANCE(vector2, mx::Vector2)
    BIND_TYPE_INSTANCE(vector3, mx::Vector3)
    BIND_TYPE_INSTANCE(vector4, mx::Vector4)
    BIND_TYPE_INSTANCE(matrix33, mx::Matrix33)
    BIND_TYPE_INSTANCE(matrix44, mx::Matrix44)
    BIND_TYPE_INSTANCE(string, std::string)
    BIND_TYPE_INSTANCE(integerarray, mx::IntVec)
    BIND_TYPE_INSTANCE(booleanarray, mx::BoolVec)
    BIND_TYPE_INSTANCE(floatarray, mx::FloatVec)
    BIND_TYPE_INSTANCE(stringarray, mx::StringVec)

    mod.attr("TypedValue_boolean").doc() = "A `Value` storing a `bool` value.";
    mod.attr("TypedValue_booleanarray").doc() = "A `Value` storing a `list` of `bool` values.";
    mod.attr("TypedValue_color3").doc() = "A `Value` storing a `Color3` value.";
    mod.attr("TypedValue_color4").doc() = "A `Value` storing a `Color4` value.";
    mod.attr("TypedValue_float").doc() = "A `Value` storing a `float` value.";
    mod.attr("TypedValue_floatarray").doc() = "A `Value` storing a `list` of `float` values.";
    mod.attr("TypedValue_integer").doc() = "A `Value` storing an `int` value.";
    mod.attr("TypedValue_integerarray").doc() = "A `Value` storing a `list` of `int` values.";
    mod.attr("TypedValue_matrix33").doc() = "A `Value` storing a `Matrix33` value.";
    mod.attr("TypedValue_matrix44").doc() = "A `Value` storing a `Matrix44` value.";
    mod.attr("TypedValue_string").doc() = "A `Value` storing a `str` value.";
    mod.attr("TypedValue_stringarray").doc() = "A `Value` storing a `list` of `str` values.";
    mod.attr("TypedValue_vector2").doc() = "A `Value` storing a `Vector2` value.";
    mod.attr("TypedValue_vector3").doc() = "A `Value` storing a `Vector3` value.";
    mod.attr("TypedValue_vector4").doc() = "A `Value` storing a `Vector4` value.";
}
