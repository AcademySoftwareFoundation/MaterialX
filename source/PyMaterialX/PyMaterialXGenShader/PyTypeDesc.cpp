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

    :type name: `str`
    :param name: The name of the type descriptor to return.
    :return: A type descriptor for the given `name`, or `None` if no type with
        the given `name` could be found.
)docstring"))
        .def("getName", &mx::TypeDesc::getName)
        .def("getBaseType", &mx::TypeDesc::getBaseType)
        .def("getChannelIndex", &mx::TypeDesc::getChannelIndex,
             py::arg("channel"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the channel index for the supplied channel name.

    :type channel: `str`
    :param channel: The name of the channel whose index to return.
    :return: The index that corresponds to the given `channel` name, or `-1` on
        failure to find a matching index.
)docstring"))
        .def("getSemantic", &mx::TypeDesc::getSemantic)
        .def("getSize", &mx::TypeDesc::getSize)
        .def("isEditable", &mx::TypeDesc::isEditable)
        .def("isScalar", &mx::TypeDesc::isScalar)
        .def("isAggregate", &mx::TypeDesc::isAggregate)
        .def("isArray", &mx::TypeDesc::isArray)
        .def("isFloat2", &mx::TypeDesc::isFloat2)
        .def("isFloat3", &mx::TypeDesc::isFloat3)
        .def("isFloat4", &mx::TypeDesc::isFloat4)
        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing a type descriptor for MaterialX data types.

    All types need to have a type descriptor registered in order for shader generators
    to know about the type. A unique type descriptor pointer is the identifier used for
    types, and can be used for type comparisons as well as getting more information
    about the type. All standard library data types are registered by default and their
    type descriptors can be accessed from the `Type` namespace, e.g. `MaterialX::Type::FLOAT`.
    If custom types are used they must be registered by calling `TypeDesc.registerType()`.
    Descriptors for registered types can be retrieved using `TypeDesc.get()`, see below.

    :see: https://materialx.org/docs/api/class_type_desc.html
)docstring");
}
