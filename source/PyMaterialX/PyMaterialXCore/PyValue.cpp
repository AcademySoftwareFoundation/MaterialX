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
    .def_readonly_static("TYPE", &mx::TypedValue<T>::TYPE)                                                  \
    .doc() = "A `TypedValue` of type `" #T "`.";

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyValue(py::module& mod)
{
    py::class_<mx::Value, mx::ValuePtr>(mod, "Value")
        .def("getValueString", &mx::Value::getValueString)
        .def("getTypeString", &mx::Value::getTypeString)
        .def_static("createValueFromStrings", &mx::Value::createValueFromStrings)
        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a generic, discriminated value, whose type may be
    queried dynamically.

    :see: https://materialx.org/docs/api/class_value.html
)docstring");

    BIND_TYPE_INSTANCE(integer, int)
    BIND_TYPE_INSTANCE(boolean, bool)
    BIND_TYPE_INSTANCE(float, float)
    // Use the full `MaterialX::` prefix here so that it appears in the docstrings
    BIND_TYPE_INSTANCE(color3, MaterialX::Color3)
    BIND_TYPE_INSTANCE(color4, MaterialX::Color4)
    BIND_TYPE_INSTANCE(vector2, MaterialX::Vector2)
    BIND_TYPE_INSTANCE(vector3, MaterialX::Vector3)
    BIND_TYPE_INSTANCE(vector4, MaterialX::Vector4)
    BIND_TYPE_INSTANCE(matrix33, MaterialX::Matrix33)
    BIND_TYPE_INSTANCE(matrix44, MaterialX::Matrix44)
    BIND_TYPE_INSTANCE(string, std::string)
    BIND_TYPE_INSTANCE(integerarray, MaterialX::IntVec)
    BIND_TYPE_INSTANCE(booleanarray, MaterialX::BoolVec)
    BIND_TYPE_INSTANCE(floatarray, MaterialX::FloatVec)
    BIND_TYPE_INSTANCE(stringarray, MaterialX::StringVec)
}
