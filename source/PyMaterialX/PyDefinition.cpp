//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Definition.h>

#include <MaterialXCore/Material.h>

#include <PyBind11/stl.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyDefinition(py::module& mod)
{
    py::class_<mx::NodeDef, mx::NodeDefPtr, mx::InterfaceElement>(mod, "NodeDef", py::metaclass())
        .def("setNode", &mx::NodeDef::setNode)
        .def("hasNode", &mx::NodeDef::hasNode)
        .def("getNode", &mx::NodeDef::getNode)
        .def("getInstantiatingShaderRefs", &mx::NodeDef::getInstantiatingShaderRefs)
        .def_readonly_static("CATEGORY", &mx::NodeDef::CATEGORY);

    py::class_<mx::TypeDef, mx::TypeDefPtr, mx::Element>(mod, "TypeDef", py::metaclass())
        .def("setSemantic", &mx::TypeDef::setSemantic)
        .def("hasSemantic", &mx::TypeDef::hasSemantic)
        .def("getSemantic", &mx::TypeDef::getSemantic)
        .def("setContext", &mx::TypeDef::setContext)
        .def("hasContext", &mx::TypeDef::hasContext)
        .def("getContext", &mx::TypeDef::getContext)
        .def_readonly_static("CATEGORY", &mx::TypeDef::CATEGORY);

    py::class_<mx::Implementation, mx::ImplementationPtr, mx::InterfaceElement>(mod, "Implementation", py::metaclass())
        .def("setNodeDef", &mx::Implementation::setNodeDef)
        .def("hasNodeDef", &mx::Implementation::hasNodeDef)
        .def("getNodeDef", &mx::Implementation::getNodeDef)
        .def("setFile", &mx::Implementation::setFile)
        .def("hasFile", &mx::Implementation::hasFile)
        .def("getFile", &mx::Implementation::getFile)
        .def("setFunction", &mx::Implementation::setFunction)
        .def("hasFunction", &mx::Implementation::hasFunction)
        .def("getFunction", &mx::Implementation::getFunction)
        .def("setLanguage", &mx::Implementation::setLanguage)
        .def("hasLanguage", &mx::Implementation::hasLanguage)
        .def("getLanguage", &mx::Implementation::getLanguage)
        .def_readonly_static("CATEGORY", &mx::Implementation::CATEGORY);
}
