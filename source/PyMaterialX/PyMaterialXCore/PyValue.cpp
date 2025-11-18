//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Value.h>
#include <MaterialXCore/Definition.h>

#define BIND_TYPE_INSTANCE(NAME, T)                                                                         \
py::class_<mx::TypedValue<T>, std::shared_ptr< mx::TypedValue<T> >, mx::Value>(mod, "TypedValue_" #NAME)    \
    .def("getData", &mx::TypedValue<T>::getData, "Return the raw float vector.")                                                            \
    .def("getValueString", &mx::TypedValue<T>::getValueString, "Return the value set on this port as a string, or an empty string if there is no value.")                                              \
    .def_static("createValue", &mx::Value::createValue<T>)                                                  \
    .def_readonly_static("TYPE", &mx::TypedValue<T>::TYPE);

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyValue(py::module& mod)
{
    py::class_<mx::Value, mx::ValuePtr>(mod, "Value", "A generic, discriminated value, whose type may be queried dynamically.")
        .def("getValueString", &mx::Value::getValueString, "Return the value string for this value.")
        .def("getTypeString", &mx::Value::getTypeString, "Return the type string for this value.")
        .def_static("createValueFromStrings", &mx::Value::createValueFromStrings, py::arg("value"), py::arg("type"), py::arg("typeDefPtr") = nullptr, "Create a new value instance from value and type strings.\n\nReturns:\n    A shared pointer to a typed value, or an empty shared pointer if the conversion to the given data type cannot be performed.");

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
}
