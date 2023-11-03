//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Types.h>

#include <MaterialXCore/Value.h>

#include <sstream>

namespace py = pybind11;
namespace mx = MaterialX;

using IndexPair = std::pair<size_t, size_t>;

#define BIND_VECTOR_SUBCLASS(V, N)                                     \
.def(py::init<>(),                                                     \
     "Initialize an instance of this class.")                          \
.def(py::init<float>(),                                                \
     py::arg("value"),                                                 \
     "Initialize an instance of this class using the given value.")    \
.def(py::init<const std::array<float, N>&>(),                          \
     py::arg("values"),                                                \
     "Initialize an instance of this class using the given values.")   \
.def(py::init<const std::vector<float>&>(),                            \
     py::arg("values"),                                                \
     "Initialize an instance of this class using the given values.")   \
.def(py::self == py::self,                                             \
     py::arg("other"))                                                 \
.def(py::self != py::self,                                             \
     py::arg("other"))                                                 \
.def(py::self + py::self,                                              \
     py::arg("rhs"))                                                   \
.def(py::self - py::self,                                              \
     py::arg("rhs"))                                                   \
.def(py::self * py::self,                                              \
     py::arg("rhs"))                                                   \
.def(py::self / py::self,                                              \
     py::arg("rhs"))                                                   \
.def(py::self * float(),                                               \
     py::arg("value"))                                                 \
.def(py::self / float(),                                               \
     py::arg("value"))                                                 \
.def("getMagnitude", &V::getMagnitude,                                 \
     "Return the magnitude of the vector.")                            \
.def("getNormalized", &V::getNormalized,                               \
     "Return a normalized vector.")                                    \
.def("dot", &V::dot,                                                   \
     py::arg("rhs"),                                                   \
     "Return the dot product of two vectors.")                         \
.def("__getitem__", [](const V& v, size_t i)                           \
     { return v[i]; },                                                 \
     py::arg("index"))                                                 \
.def("__setitem__", [](V& v, size_t i, float f)                        \
     { v[i] = f; },                                                    \
     py::arg("index"),                                                 \
     py::arg("value"))                                                 \
.def("__str__", [](const V& v)                                         \
    { return mx::toValueString(v); })                                  \
.def("copy", [](const V& v) { return V(v); },                          \
     "Create a copy of the vector.")                                   \
.def_static("__len__", &V::numElements)

#define BIND_MATRIX_SUBCLASS(M, N)                                     \
.def(py::init<>())                                                     \
.def(py::init<float>(),                                                \
     py::arg("value"))                                                 \
.def(py::self == py::self,                                             \
     py::arg("other"))                                                 \
.def(py::self != py::self,                                             \
     py::arg("other"))                                                 \
.def(py::self + py::self,                                              \
     py::arg("rhs"))                                                   \
.def(py::self - py::self,                                              \
     py::arg("rhs"))                                                   \
.def(py::self * py::self,                                              \
     py::arg("rhs"))                                                   \
.def(py::self / py::self,                                              \
     py::arg("rhs"))                                                   \
.def(py::self * float(),                                               \
     py::arg("value"))                                                 \
.def(py::self / float(),                                               \
     py::arg("value"))                                                 \
.def("__getitem__", [](const M& m, IndexPair i)                        \
     { return m[i.first][i.second]; },                                 \
     py::arg("indexPair"))                                             \
.def("__setitem__", [](M& m, IndexPair i, float f)                     \
     { m[i.first][i.second] = f; },                                    \
     py::arg("indexPair"),                                             \
     py::arg("value"))                                                 \
.def("__str__", [](const M& m)                                         \
     { return mx::toValueString(m); })                                 \
.def("copy", [](const M& m) { return M(m); },                          \
     "Create a copy of the matrix.")                                   \
.def("isEquivalent", &M::isEquivalent,                                 \
     py::arg("rhs"),                                                   \
     py::arg("tolerance"),                                             \
     "Return `True` if the given matrix is equivalent to this one "    \
     "within a given floating-point `tolerance`.")                     \
.def("getTranspose", &M::getTranspose,                                 \
     "Return the transpose of the matrix.")                            \
.def("getDeterminant", &M::getDeterminant,                             \
     "Return the determinant of the matrix.")                          \
.def("getAdjugate", &M::getAdjugate,                                   \
     "Return the adjugate of the matrix.")                             \
.def("getInverse", &M::getInverse,                                     \
     "Return the inverse of the matrix.")                              \
.def_static("createScale", &M::createScale,                            \
            py::arg("v"),                                              \
            "Create a scale matrix.")                                  \
.def_static("createTranslation", &M::createTranslation,                \
            py::arg("v"),                                              \
            "Create a translation matrix.")                            \
.def_static("numRows", &M::numRows,                                    \
            "Return the number of rows in this matrix.")               \
.def_static("numColumns", &M::numColumns,                              \
            "Return the number of columns in this matrix.")            \
.def_static("__len__", &M::numRows)

void bindPyTypes(py::module& mod)
{
    py::class_<mx::VectorBase>(mod, "VectorBase",
                               "Base class for vectors of scalar values.");
    py::class_<mx::MatrixBase>(mod, "MatrixBase",
                               "Base class for square matrices of scalar values.");

    py::class_<mx::Vector2, mx::VectorBase>(mod, "Vector2")
        BIND_VECTOR_SUBCLASS(mx::Vector2, 2)

        .def(py::init<float, float>(),
             py::arg("x"),
             py::arg("y"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class based on the given X and Y coordinate
    values.
)docstring"))

        .def("cross", &mx::Vector2::cross,
             py::arg("rhs"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the cross product of two vectors.
)docstring"))

        .def("asTuple", [](const mx::Vector2& v) { return std::make_tuple(v[0], v[1]); },
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the vector components as a tuple of floating-point numbers.
)docstring"))

        .doc() = "A vector of two floating-point values.";

    py::class_<mx::Vector3, mx::VectorBase>(mod, "Vector3")
        BIND_VECTOR_SUBCLASS(mx::Vector3, 3)

        .def(py::init<float, float, float>(),
             py::arg("x"), py::arg("y"), py::arg("z"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class based on the given X, Y, and Z
    coordinate values.
)docstring"))

        .def("cross", &mx::Vector3::cross,
             py::arg("rhs"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the cross product of two vectors.
)docstring"))

        .def("asTuple", [](const mx::Vector3& v) { return std::make_tuple(v[0], v[1], v[2]); },
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the vector components as a 3-tuple of floating-point numbers.
)docstring"))

        .doc() = "A vector of three floating-point values.";

    py::class_<mx::Vector4, mx::VectorBase>(mod, "Vector4")
        BIND_VECTOR_SUBCLASS(mx::Vector4, 4)

        .def(py::init<float, float, float, float>(),
             py::arg("x"), py::arg("y"), py::arg("z"), py::arg("w"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class based on the given X, Y, Z, and W
    coordinate values.
)docstring"))

        .def("asTuple", [](const mx::Vector4& v) { return std::make_tuple(v[0], v[1], v[2], v[3]); },
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the vector components as a 4-tuple of floating-point numbers.
)docstring"))

        .doc() = "A vector of four floating-point values.";

    py::class_<mx::Color3, mx::VectorBase>(mod, "Color3")
        BIND_VECTOR_SUBCLASS(mx::Color3, 3)

        .def(py::init<float, float, float>(),
             py::arg("r"), py::arg("g"), py::arg("b"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class based on the given red, green, and
    blue color component values.
)docstring"))

        .def("linearToSrgb", &mx::Color3::linearToSrgb,
             PYMATERIALX_DOCSTRING(R"docstring(
    Transform this color from linear RGB to the sRGB encoding,
    returning the result as a new value.
)docstring"))

        .def("srgbToLinear", &mx::Color3::srgbToLinear,
             PYMATERIALX_DOCSTRING(R"docstring(
    Transform this color from the sRGB encoding to linear RGB,
    returning the result as a new value.
)docstring"))

        .def("asTuple", [](const mx::Color3& v) { return std::make_tuple(v[0], v[1], v[2]); },
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the color components as a 3-tuple of floating-point numbers.
)docstring"))

        .doc() = "A three-component RGB color value.";

    py::class_<mx::Color4, mx::VectorBase>(mod, "Color4")
        BIND_VECTOR_SUBCLASS(mx::Color4, 4)

        .def(py::init<float, float, float, float>(),
             py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class based on the given red, green, blue,
    and alpha color component values.
)docstring"))

        .def("asTuple", [](const mx::Color4& v) { return std::make_tuple(v[0], v[1], v[2], v[3]); },
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the color components as a 4-tuple of floating-point numbers.
)docstring"))

        .doc() = "A four-component RGBA color value";

    py::class_<mx::Matrix33, mx::MatrixBase>(mod, "Matrix33")
        BIND_MATRIX_SUBCLASS(mx::Matrix33, 3)

        .def(py::init<float, float, float,
                      float, float, float,
                      float, float, float>(),
             py::arg("m00"), py::arg("m01"), py::arg("m02"),
             py::arg("m10"), py::arg("m11"), py::arg("m12"),
             py::arg("m20"), py::arg("m21"), py::arg("m22"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class based on the 9 given matrix component
    values.
)docstring"))

        .def("multiply", &mx::Matrix33::multiply,
             py::arg("v"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the product of this matrix and a 3D vector.
)docstring"))

        .def("transformPoint", &mx::Matrix33::transformPoint,
             py::arg("v"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Transform the given 2D point.
)docstring"))

        .def("transformVector", &mx::Matrix33::transformVector,
             py::arg("v"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Transform the given 2D direction vector.
)docstring"))

        .def("transformNormal", &mx::Matrix33::transformNormal,
             py::arg("v"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Transform the given 3D normal vector.
)docstring"))

        .def_static("createRotation", &mx::Matrix33::createRotation,
                    py::arg("angle"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a rotation matrix.

    :type angle: float
    :param angle: Angle in radians.
)docstring"))

        .def_readonly_static("IDENTITY", &mx::Matrix33::IDENTITY,
                             "The 3x3 identity matrix.")

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A 3x3 matrix of floating-point values.

    Vector transformation methods follow the row-vector convention,
    with matrix-vector multiplication computed as `v' = vM`.

    :see: https://materialx.org/docs/api/class_matrix33.html
)docstring");

    py::class_<mx::Matrix44, mx::MatrixBase>(mod, "Matrix44")
        BIND_MATRIX_SUBCLASS(mx::Matrix44, 4)

        .def(py::init<float, float, float, float,
                      float, float, float, float,
                      float, float, float, float,
                      float, float, float, float>(),
             py::arg("m00"), py::arg("m01"), py::arg("m02"), py::arg("m03"),
             py::arg("m10"), py::arg("m11"), py::arg("m12"), py::arg("m13"),
             py::arg("m20"), py::arg("m21"), py::arg("m22"), py::arg("m23"),
             py::arg("m30"), py::arg("m31"), py::arg("m32"), py::arg("m33"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class based on the 16 given matrix component
    values.
)docstring"))

        .def("multiply", &mx::Matrix44::multiply,
             py::arg("v"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the product of this matrix and a 4D vector.
)docstring"))

        .def("transformPoint", &mx::Matrix44::transformPoint,
             py::arg("v"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Transform the given 3D point.
)docstring"))

        .def("transformVector", &mx::Matrix44::transformVector,
             py::arg("v"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Transform the given 3D direction vector.
)docstring"))

        .def("transformNormal", &mx::Matrix44::transformNormal,
             py::arg("v"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Transform the given 3D normal vector.
)docstring"))

        .def_static("createRotationX", &mx::Matrix44::createRotationX,
                    py::arg("angle"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a rotation matrix around the X-axis.

    :type angle: float
    :param angle: Angle in radians.
)docstring"))

        .def_static("createRotationY", &mx::Matrix44::createRotationY,
                    py::arg("angle"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a rotation matrix around the Y-axis.

    :type angle: float
    :param angle: Angle in radians.
)docstring"))

        .def_static("createRotationZ", &mx::Matrix44::createRotationZ,
                    py::arg("angle"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a rotation matrix around the Z-axis.

    :type angle: float
    :param angle: Angle in radians.
)docstring"))

        .def_readonly_static("IDENTITY", &mx::Matrix44::IDENTITY,
                             "The 4x4 identity matrix.")

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A 4x4 matrix of floating-point values.

    Vector transformation methods follow the row-vector convention,
    with matrix-vector multiplication computed as `v' = vM`.

    :see: https://materialx.org/docs/api/class_matrix44.html
)docstring");

    mod.attr("DEFAULT_TYPE_STRING") = mx::DEFAULT_TYPE_STRING;
    mod.attr("FILENAME_TYPE_STRING") = mx::FILENAME_TYPE_STRING;
    mod.attr("GEOMNAME_TYPE_STRING") = mx::GEOMNAME_TYPE_STRING;
    mod.attr("STRING_TYPE_STRING") = mx::STRING_TYPE_STRING;
    mod.attr("BSDF_TYPE_STRING") = mx::BSDF_TYPE_STRING;
    mod.attr("EDF_TYPE_STRING") = mx::EDF_TYPE_STRING;
    mod.attr("VDF_TYPE_STRING") = mx::VDF_TYPE_STRING;
    mod.attr("SURFACE_SHADER_TYPE_STRING") = mx::SURFACE_SHADER_TYPE_STRING;
    mod.attr("DISPLACEMENT_SHADER_TYPE_STRING") = mx::DISPLACEMENT_SHADER_TYPE_STRING;
    mod.attr("VOLUME_SHADER_TYPE_STRING") = mx::VOLUME_SHADER_TYPE_STRING;
    mod.attr("LIGHT_SHADER_TYPE_STRING") = mx::LIGHT_SHADER_TYPE_STRING;
    mod.attr("MATERIAL_TYPE_STRING") = mx::MATERIAL_TYPE_STRING;
    mod.attr("SURFACE_MATERIAL_NODE_STRING") = mx::SURFACE_MATERIAL_NODE_STRING;
    mod.attr("VOLUME_MATERIAL_NODE_STRING") = mx::VOLUME_MATERIAL_NODE_STRING;
    mod.attr("MULTI_OUTPUT_TYPE_STRING") = mx::MULTI_OUTPUT_TYPE_STRING;
    mod.attr("NONE_TYPE_STRING") = mx::NONE_TYPE_STRING;
    mod.attr("VALUE_STRING_TRUE") = mx::VALUE_STRING_TRUE;
    mod.attr("VALUE_STRING_FALSE") = mx::VALUE_STRING_FALSE;
    mod.attr("NAME_PREFIX_SEPARATOR") = mx::NAME_PREFIX_SEPARATOR;
    mod.attr("NAME_PATH_SEPARATOR") = mx::NAME_PATH_SEPARATOR;
    mod.attr("ARRAY_VALID_SEPARATORS") = mx::ARRAY_VALID_SEPARATORS;
    mod.attr("ARRAY_PREFERRED_SEPARATOR") = mx::ARRAY_PREFERRED_SEPARATOR;
}
