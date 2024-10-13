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
        .def_static("get", &mx::TypeDesc::get)
        .def("getName", &mx::TypeDesc::getName)
        .def("getBaseType", &mx::TypeDesc::getBaseType)
        .def("getSemantic", &mx::TypeDesc::getSemantic)
        .def("getSize", &mx::TypeDesc::getSize)
        .def("isScalar", &mx::TypeDesc::isScalar)
        .def("isAggregate", &mx::TypeDesc::isAggregate)
        .def("isArray", &mx::TypeDesc::isArray)
        .def("isFloat2", &mx::TypeDesc::isFloat2)
        .def("isFloat3", &mx::TypeDesc::isFloat3)
        .def("isFloat4", &mx::TypeDesc::isFloat4)
        .def("isClosure", &mx::TypeDesc::isClosure);
    mod.attr("TypeDesc").doc() = R"docstring(
    A type descriptor for MaterialX data types.

    All types need to have a type descriptor registered in order for shader generators
    to know about the type. It can be used for type comparisons as well as getting more
    information about the type. Type descriptors for all standard library data types are
    registered by default and can be accessed from the `Type` namespace, e.g. `Type::FLOAT`.

    To register custom types use the macro `TYPEDESC_DEFINE_TYPE` to define it in a header
    and the macro `TYPEDESC_REGISTER_TYPE` to register it in the type registry. Registration
    must be done in order to access the type's name later using `getName()` and to find the
    type by name using `TypeDesc.get()`.

    The class is a POD type of 64-bits and can efficiently be stored and passed by value.
    Type compare operations and hash operations are done using a precomputed hash value.

    :see: https://materialx.org/docs/api/class_type_desc.html)docstring";
}
