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
    py::class_<mx::TypeDesc, std::unique_ptr<MaterialX::TypeDesc, py::nodelete>>(mod, "TypeDesc", "A type descriptor for MaterialX data types.\n\nAll types need to have a type descriptor registered in order for shader generators to know about the type. It can be used for type comparisons as well as getting more information about the type. Type descriptors for all standard library data types are defined by default and can be accessed from the Type namespace, e.g. Type::FLOAT. Custom struct types defined through typedef elements in a data library are loaded in and registered by calling the ShaderGenerator::registerTypeDefs method. The TypeSystem class, see below, is used to manage all type descriptions. It can be used to query the registered types.")
        .def("getName", &mx::TypeDesc::getName, "Return the ColorManagementSystem name.")
        .def("getBaseType", &mx::TypeDesc::getBaseType, "Return the base type of the image.")
        .def("getSemantic", &mx::TypeDesc::getSemantic, "Return the variable semantic of this port.")
        .def("getSize", &mx::TypeDesc::getSize, "Get the number of elements.")
        .def("isScalar", &mx::TypeDesc::isScalar, "Return true if the type is a scalar type.")
        .def("isAggregate", &mx::TypeDesc::isAggregate, "Return true if the type is an aggregate type.")
        .def("isArray", &mx::TypeDesc::isArray, "Return true if the type is an array type.")
        .def("isFloat2", &mx::TypeDesc::isFloat2, "Return true if the type is an aggregate of 2 floats.")
        .def("isFloat3", &mx::TypeDesc::isFloat3, "Return true if the type is an aggregate of 3 floats.")
        .def("isFloat4", &mx::TypeDesc::isFloat4, "Return true if the type is an aggregate of 4 floats.")
        .def("isClosure", &mx::TypeDesc::isClosure, "Return true if the type represents a closure.");
}
