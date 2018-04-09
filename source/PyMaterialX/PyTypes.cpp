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

using IndexPair = std::pair<size_t, size_t>;

#define BIND_VECTOR_SUBCLASS(T, N)                      \
.def(py::init<>())                                      \
.def(py::init<float>())                                 \
.def(py::init<const std::array<float, N>&>())           \
.def(py::init<const std::vector<float>&>())             \
.def(py::self == py::self)                              \
.def(py::self != py::self)                              \
.def("__add__", [](const T& v1, const T& v2)            \
    { return T(v1 + v2); } )                            \
.def("__sub__", [](const T& v1, const T& v2)            \
    { return T(v1 - v2); } )                            \
.def("__mul__", [](const T& v1, const T& v2)            \
    { return T(v1 * v2); } )                            \
.def("__div__", [](const T& v1, const T& v2)            \
    { return T(v1 / v2); } )                            \
.def("__truediv__", [](const T& v1, const T& v2)        \
    { return T(v1 / v2); } )                            \
.def("__getitem__", [](T& vec, size_t i)                \
    { return vec[i]; } )                                \
.def("__setitem__", [](T& vec, size_t i, float f)       \
    { vec[i] = f; } )                                   \
.def("__str__", [](const T& vec)                        \
    { return mx::toValueString(vec); })                 \
.def("copy", [](const T& vec) { return T(vec); })       \
.def_static("__len__", &T::length)

#define BIND_MATRIX_SUBCLASS(T, V, N)                   \
.def(py::init<>())                                      \
.def(py::init<float>())                                 \
.def(py::self == py::self)                              \
.def(py::self != py::self)                              \
.def("__add__", [](const T& m1, const T& m2)            \
    { return T(m1 + m2); } )                            \
.def("__sub__", [](const T& m1, const T& m2)            \
    { return T(m1 - m2); } )                            \
.def("__mul__", [](const T& m1, const T& m2)            \
    { return T(m1 * m2); } )                            \
.def("__div__", [](const T& m1, const T& m2)            \
    { return T(m1 / m2); } )                            \
.def("__truediv__", [](const T& m1, const T& m2)        \
    { return T(m1 / m2); } )                            \
.def("__getitem__", [](const T &mat, IndexPair i)       \
    { return mat[i.first][i.second]; } )                \
.def("__setitem__", [](T &mat, IndexPair i, float f)    \
    { mat[i.first][i.second] = f; })                    \
.def("__str__", [](const T& mat)                        \
    { return mx::toValueString(mat); })                 \
.def("copy", [](const T& mat) { return T(mat); })       \
.def("getRow", &T::getRow)                              \
.def("getColumn", &T::getColumn)                        \
.def_static("numRows", &T::numRows)                     \
.def_static("numColumns", &T::numColumns)               \
.def_static("__len__", &T::numRows)

void bindPyTypes(py::module& mod)
{
    py::class_<mx::VectorBase>(mod, "VectorBase");
    py::class_<mx::MatrixBase>(mod, "MatrixBase");

    py::class_<mx::Vector2, mx::VectorBase>(mod, "Vector2")
        BIND_VECTOR_SUBCLASS(mx::Vector2, 2)
        .def(py::init<float, float>())
        .def("asTuple", [](const mx::Vector2 &vec) { return std::make_tuple(vec[0], vec[1]); });

    py::class_<mx::Vector3, mx::VectorBase>(mod, "Vector3")
        BIND_VECTOR_SUBCLASS(mx::Vector3, 3)
        .def(py::init<float, float, float>())
        .def("asTuple", [](const mx::Vector3 &vec) { return std::make_tuple(vec[0], vec[1], vec[2]); });

    py::class_<mx::Vector4, mx::VectorBase>(mod, "Vector4")
        BIND_VECTOR_SUBCLASS(mx::Vector4, 4)
        .def(py::init<float, float, float, float>())
        .def("asTuple", [](const mx::Vector4 &vec) { return std::make_tuple(vec[0], vec[1], vec[2], vec[3]); });

    py::class_<mx::Color2, mx::Vector2>(mod, "Color2")
        BIND_VECTOR_SUBCLASS(mx::Color2, 2)
        .def(py::init<float, float>());

    py::class_<mx::Color3, mx::Vector3>(mod, "Color3")
        BIND_VECTOR_SUBCLASS(mx::Color3, 3)
        .def(py::init<float, float, float>());

    py::class_<mx::Color4, mx::Vector4>(mod, "Color4")
        BIND_VECTOR_SUBCLASS(mx::Color4, 4)
        .def(py::init<float, float, float, float>());

    py::class_<mx::Matrix33, mx::MatrixBase>(mod, "Matrix33")
        BIND_MATRIX_SUBCLASS(mx::Matrix33, mx::Vector3, 3)
        .def(py::init<float, float, float,
                      float, float, float,
                      float, float, float>())
        .def_readonly_static("IDENTITY", &mx::Matrix33::IDENTITY);

    py::class_<mx::Matrix44, mx::MatrixBase>(mod, "Matrix44")
        BIND_MATRIX_SUBCLASS(mx::Matrix44, mx::Vector4, 4)
        .def(py::init<float, float, float, float,
                      float, float, float, float,
                      float, float, float, float,
                      float, float, float, float>())
        .def_readonly_static("IDENTITY", &mx::Matrix44::IDENTITY);
}
