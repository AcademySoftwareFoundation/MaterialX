//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Types.h>

#include <PyBind11/stl.h>
#include <PyBind11/operators.h>

#include <sstream>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyTypes(py::module& mod)
{
    py::class_<mx::VectorBase>(mod, "VectorBase");

    py::class_<mx::Vector2, mx::VectorBase>(mod, "Vector2")
        .def(py::init<>())
        .def(py::init<float, float>())
        .def(py::init<const mx::Vector2&>())
        .def(py::init<const std::array<float, 2>&>())
        .def(py::init<const std::vector<float>&>())
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("asTuple", [](const mx::Vector2 &vec) { return std::make_tuple(vec[0], vec[1]); })
        .def("__getitem__", [](mx::Vector2& vec, size_t i) { return vec[i]; } )
        .def("__setitem__", [](mx::Vector2& vec, size_t i, float value) { vec[i] = value; } )
        .def("__len__", [](const mx::Vector2& vec) { return vec.length(); } )
        .def("__iter__", [](const mx::Vector2& vec) { return py::make_iterator(vec.data.begin(), vec.data.end()); },
            py::keep_alive<0, 1>())
        .def("__str__", [](const mx::Vector2 &vec)
            {
                std::ostringstream output;
                output << vec;
                return py::bytes(output.str());
            }
        );

    py::class_<mx::Vector3, mx::VectorBase>(mod, "Vector3")
        .def(py::init<>())
        .def(py::init<float, float, float>())
        .def(py::init<const mx::Vector3&>())
        .def(py::init<const std::array<float, 3>&>())
        .def(py::init<const std::vector<float>&>())
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("asTuple", [](const mx::Vector3 &vec) { return std::make_tuple(vec[0], vec[1], vec[2]); })
        .def("__getitem__", [](mx::Vector3& vec, size_t i) { return vec[i]; } )
        .def("__setitem__", [](mx::Vector3& vec, size_t i, float value) { vec[i] = value; } )
        .def("__len__", [](const mx::Vector3& vec) { return vec.length(); } )
        .def("__iter__", [](const mx::Vector3& vec) { return py::make_iterator(vec.data.begin(), vec.data.end()); },
            py::keep_alive<0, 1>())
        .def("__str__", [](const mx::Vector3 &vec)
            {
                std::ostringstream output;
                output << vec;
                return py::bytes(output.str());
            }
        );

    py::class_<mx::Vector4, mx::VectorBase>(mod, "Vector4")
        .def(py::init<>())
        .def(py::init<float, float, float, float>())
        .def(py::init<const mx::Vector4&>())
        .def(py::init<const std::array<float, 4>&>())
        .def(py::init<const std::vector<float>&>())
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("asTuple", [](const mx::Vector4 &vec) { return std::make_tuple(vec[0], vec[1], vec[2], vec[3]); })
        .def("__getitem__", [](mx::Vector4& vec, size_t i) { return vec[i]; } )
        .def("__setitem__", [](mx::Vector4& vec, size_t i, float value) { vec[i] = value; } )
        .def("__len__", [](const mx::Vector4& vec) { return vec.length(); })
        .def("__iter__", [](const mx::Vector4& vec) { return py::make_iterator(vec.data.begin(), vec.data.end()); },
            py::keep_alive<0, 1>())
        .def("__str__", [](const mx::Vector4 &vec)
            {
                std::ostringstream output;
                output << vec;
                return py::bytes(output.str());
            }
        );

    py::class_<mx::Matrix3x3, mx::VectorBase>(mod, "Matrix3x3")
        .def(py::init<>())
        .def(py::init<const mx::Matrix3x3&>())
        .def(py::init<const std::array<float, 9>&>())
        .def(py::init<const std::vector<float>&>())
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__getitem__", [](mx::Matrix3x3& vec, size_t i) { return vec[i]; } )
        .def("__setitem__", [](mx::Matrix3x3& vec, size_t i, float value) { vec[i] = value; } )
        .def("__len__", [](const mx::Matrix3x3& vec) { return vec.length(); })
        .def("__iter__", [](const mx::Matrix3x3& vec) { return py::make_iterator(vec.data.begin(), vec.data.end()); },
            py::keep_alive<0, 1>())
        .def("__str__", [](const mx::Matrix3x3 &vec)
            {
                std::ostringstream output;
                output << vec;
                return py::bytes(output.str());
            }
        );

    py::class_<mx::Matrix4x4, mx::VectorBase>(mod, "Matrix4x4")
        .def(py::init<>())
        .def(py::init<const mx::Matrix4x4&>())
        .def(py::init<const std::array<float, 16>&>())
        .def(py::init<const std::vector<float>&>())
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__getitem__", [](mx::Matrix4x4& vec, size_t i) { return vec[i]; } )
        .def("__setitem__", [](mx::Matrix4x4& vec, size_t i, float value) { vec[i] = value; } )
        .def("__len__", [](const mx::Matrix4x4& vec) { return vec.length(); })
        .def("__iter__", [](const mx::Matrix4x4& vec) { return py::make_iterator(vec.data.begin(), vec.data.end()); },
            py::keep_alive<0, 1>())
        .def("__str__", [](const mx::Matrix4x4 &vec)
            {
                std::ostringstream output;
                output << vec;
                return py::bytes(output.str());
            }
        );

    py::class_<mx::Color2, mx::Vector2>(mod, "Color2")
        .def(py::init<>())
        .def(py::init<float, float>())
        .def(py::init<const mx::Color2&>())
        .def(py::init<const std::array<float, 2>&>())
        .def(py::init<const std::vector<float>&>())
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::class_<mx::Color3, mx::Vector3>(mod, "Color3")
        .def(py::init<>())
        .def(py::init<float, float, float>())
        .def(py::init<const mx::Color3&>())
        .def(py::init<const std::array<float, 3>&>())
        .def(py::init<const std::vector<float>&>())
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::class_<mx::Color4, mx::Vector4>(mod, "Color4")
        .def(py::init<>())
        .def(py::init<float, float, float, float>())
        .def(py::init<const mx::Color4&>())
        .def(py::init<const std::array<float, 4>&>())
        .def(py::init<const std::vector<float>&>())
        .def(py::self == py::self)
        .def(py::self != py::self);
}
