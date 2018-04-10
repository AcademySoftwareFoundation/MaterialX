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

#define BIND_VECTOR_SUBCLASS(V, N)                      \
.def(py::init<>())                                      \
.def(py::init<float>())                                 \
.def(py::init<const std::array<float, N>&>())           \
.def(py::init<const std::vector<float>&>())             \
.def(py::self == py::self)                              \
.def(py::self != py::self)                              \
.def(py::self + py::self)                               \
.def(py::self - py::self)                               \
.def(py::self * py::self)                               \
.def(py::self / py::self)                               \
.def("__getitem__", [](V& v, size_t i)                  \
    { return v[i]; } )                                  \
.def("__setitem__", [](V& v, size_t i, float f)         \
    { v[i] = f; } )                                     \
.def("__str__", [](const V& v)                          \
    { return mx::toValueString(v); })                   \
.def("copy", [](const V& v) { return V(v); })           \
.def_static("__len__", &V::length)

#define BIND_MATRIX_SUBCLASS(M, N)                      \
.def(py::init<>())                                      \
.def(py::init<float>())                                 \
.def(py::self == py::self)                              \
.def(py::self != py::self)                              \
.def(py::self + py::self)                               \
.def(py::self - py::self)                               \
.def(py::self * py::self)                               \
.def(py::self / py::self)                               \
.def("__getitem__", [](const M& m, IndexPair i)         \
    { return m[i.first][i.second]; } )                  \
.def("__setitem__", [](M& m, IndexPair i, float f)      \
    { m[i.first][i.second] = f; })                      \
.def("__str__", [](const M& m)                          \
    { return mx::toValueString(m); })                   \
.def("copy", [](const M& m) { return M(m); })           \
.def("getRow", &M::getRow)                              \
.def("getColumn", &M::getColumn)                        \
.def_static("numRows", &M::numRows)                     \
.def_static("numColumns", &M::numColumns)               \
.def_static("__len__", &M::numRows)

void bindPyTypes(py::module& mod)
{
    py::class_<mx::VectorBase>(mod, "VectorBase");
    py::class_<mx::MatrixBase>(mod, "MatrixBase");

    py::class_<mx::Vector2, mx::VectorBase>(mod, "Vector2")
        BIND_VECTOR_SUBCLASS(mx::Vector2, 2)
        .def(py::init<float, float>())
        .def("asTuple", [](const mx::Vector2& v) { return std::make_tuple(v[0], v[1]); });

    py::class_<mx::Vector3, mx::VectorBase>(mod, "Vector3")
        BIND_VECTOR_SUBCLASS(mx::Vector3, 3)
        .def(py::init<float, float, float>())
        .def("asTuple", [](const mx::Vector3& v) { return std::make_tuple(v[0], v[1], v[2]); });

    py::class_<mx::Vector4, mx::VectorBase>(mod, "Vector4")
        BIND_VECTOR_SUBCLASS(mx::Vector4, 4)
        .def(py::init<float, float, float, float>())
        .def("asTuple", [](const mx::Vector4& v) { return std::make_tuple(v[0], v[1], v[2], v[3]); });

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
        BIND_MATRIX_SUBCLASS(mx::Matrix33, 3)
        .def(py::init<float, float, float,
                      float, float, float,
                      float, float, float>())
        .def_readonly_static("IDENTITY", &mx::Matrix33::IDENTITY);

    py::class_<mx::Matrix44, mx::MatrixBase>(mod, "Matrix44")
        BIND_MATRIX_SUBCLASS(mx::Matrix44, 4)
        .def(py::init<float, float, float, float,
                      float, float, float, float,
                      float, float, float, float,
                      float, float, float, float>())
        .def_readonly_static("IDENTITY", &mx::Matrix44::IDENTITY);
}
