//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/TypeDesc.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyTypeDesc(py::module& mod)
{
    // Set nodelete as destructor on returned TypeDescs since they are owned
    // by the container they are stored in and should not be destroyed when 
    // garbage collected by the python interpreter
    py::class_<mx::TypeDesc, std::unique_ptr<MaterialX::TypeDesc, py::nodelete>>(mod, "TypeDesc")

        .def_static("get", &mx::TypeDesc::get,
                    py::arg("name"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Return a type descriptor for the given `name`.

    :param name: The name of the type descriptor to return.
    :type name: str
    :return: A type descriptor for the given `name`, or `None` if no type with
        the given `name` could be found.
)docstring"))

        .def("getName", &mx::TypeDesc::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the name of the type.
)docstring"))

        .def("getBaseType", &mx::TypeDesc::getBaseType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the basetype for the type.
)docstring"))

        .def("getChannelIndex", &mx::TypeDesc::getChannelIndex,
             py::arg("channel"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the channel index for the supplied channel name.

    :param channel: The name of the channel whose index to return.
    :type channel: str
    :return: The index that corresponds to the given `channel` name, or `-1` on
        failure to find a matching index.
)docstring"))

        .def("getSemantic", &mx::TypeDesc::getSemantic,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the semantic for the type.
)docstring"))

        .def("getSize", &mx::TypeDesc::getSize,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the number of elements the type is composed of.

    Will return `1` for scalar types and a size greater than `1` for aggregate
    types.

    For array types, `0` is returned, since the number of elements is undefined
    until an array is instantiated.
)docstring"))

        .def("isEditable", &mx::TypeDesc::isEditable,
             PYMATERIALX_DOCSTRING(R"docstring(
    Returns `True` if the type is editable by users.

    Editable types are allowed to be published as shader uniforms and hence
    must be presentable in a user interface.
)docstring"))

        .def("isScalar", &mx::TypeDesc::isScalar,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the type is a scalar type.
)docstring"))

        .def("isAggregate", &mx::TypeDesc::isAggregate,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the type is an aggregate type.
)docstring"))

        .def("isArray", &mx::TypeDesc::isArray,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the type is an array type.
)docstring"))

        .def("isFloat2", &mx::TypeDesc::isFloat2,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the type is an aggregate of 2 floats.
)docstring"))

        .def("isFloat3", &mx::TypeDesc::isFloat3,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the type is an aggregate of 3 floats.
)docstring"))

        .def("isFloat4", &mx::TypeDesc::isFloat4,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the type is an aggregate of 4 floats.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing a type descriptor for MaterialX data types.

    All types need to have a type descriptor registered in order for shader
    generators to know about the type.

    A unique type descriptor is the identifier used for types, and can be used
    for type comparisons as well as getting more information about the type.

    All standard library data types are registered by default, and their type
    descriptors can be accessed from the `Type` namespace in C++, e.g.
    `MaterialX::Type::FLOAT`.

    If custom types are used, they must be registered by calling
    `TypeDesc::registerType()` in C++.

    Descriptors for registered types can be retrieved using `TypeDesc.get()`.

    :see: https://materialx.org/docs/api/class_type_desc.html
)docstring");
}
