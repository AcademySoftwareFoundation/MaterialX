//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Types.h>

#include <MaterialXCore/Value.h>

#include <sstream>

namespace py = pybind11;
namespace mx = MaterialX;

#define BIND_VECTOR_SUBCLASS(T, N)                          \
.def(py::init<>())                                          \
.def(py::init<float>())                                     \
.def(py::init<const std::array<float, N>&>())               \
.def(py::init<const std::vector<float>&>())                 \
.def(py::self == py::self)                                  \
.def(py::self != py::self)                                  \
.def("__add__", [](const T& v1, const T& v2)                \
    { return T(v1 + v2); } )                                \
.def("__sub__", [](const T& v1, const T& v2)                \
    { return T(v1 - v2); } )                                \
.def("__mul__", [](const T& v1, const T& v2)                \
    { return T(v1 * v2); } )                                \
.def("__div__", [](const T& v1, const T& v2)                \
    { return T(v1 / v2); } )                                \
.def("__getitem__", [](T& vec, size_t i)                    \
    { return vec[i]; } )                                    \
.def("__setitem__", [](T& vec, size_t i, float value)       \
    { vec[i] = value; } )                                   \
.def("__str__", [](const T& vec)                            \
    { return mx::toValueString(vec); })                     \
.def("copy", [](const T& vec) { return T(vec); })           \
.def("asTuple", [](const mx::Vector2& vec)                  \
    { return std::make_tuple(vec[0], vec[1]); })            \
.def_static("__len__", &T::length)

void bindPyTypes(py::module& mod)
{
    py::class_<mx::VectorBase>(mod, "VectorBase");

    py::class_<mx::Vector2, mx::VectorBase>(mod, "Vector2")
        BIND_VECTOR_SUBCLASS(mx::Vector2, 2)
        .def(py::init<float, float>());

    py::class_<mx::Vector3, mx::VectorBase>(mod, "Vector3")
        BIND_VECTOR_SUBCLASS(mx::Vector3, 3)
        .def(py::init<float, float, float>());

    py::class_<mx::Vector4, mx::VectorBase>(mod, "Vector4")
        BIND_VECTOR_SUBCLASS(mx::Vector4, 4)
        .def(py::init<float, float, float, float>());

    py::class_<mx::Color2, mx::Vector2>(mod, "Color2")
        BIND_VECTOR_SUBCLASS(mx::Color2, 2)
        .def(py::init<float, float>());

    py::class_<mx::Color3, mx::Vector3>(mod, "Color3")
        BIND_VECTOR_SUBCLASS(mx::Color3, 3)
        .def(py::init<float, float, float>());

    py::class_<mx::Color4, mx::Vector4>(mod, "Color4")
        BIND_VECTOR_SUBCLASS(mx::Color4, 4)
        .def(py::init<float, float, float, float>());

    py::class_<mx::Matrix3x3, mx::VectorBase>(mod, "Matrix3x3")
        BIND_VECTOR_SUBCLASS(mx::Matrix3x3, 9)
        .def(py::init<float, float, float,
                      float, float, float,
                      float, float, float>())
        .def_readonly_static("IDENTITY", &mx::Matrix3x3::IDENTITY);

    py::class_<mx::Matrix4x4, mx::VectorBase>(mod, "Matrix4x4")
        BIND_VECTOR_SUBCLASS(mx::Matrix4x4, 16)
        .def(py::init<float, float, float, float,
                      float, float, float, float,
                      float, float, float, float,
                      float, float, float, float>())
        .def_readonly_static("IDENTITY", &mx::Matrix4x4::IDENTITY);
}
