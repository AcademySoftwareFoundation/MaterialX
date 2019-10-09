//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Definition.h>

#include <MaterialXCore/Material.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyDefinition(py::module& mod)
{
    py::class_<mx::NodeDef, mx::NodeDefPtr, mx::InterfaceElement>(mod, "NodeDef")
        .def("setNodeString", &mx::NodeDef::setNodeString)
        .def("hasNodeString", &mx::NodeDef::hasNodeString)
        .def("getNodeString", &mx::NodeDef::getNodeString)
        .def("setNodeGroup", &mx::NodeDef::setNodeGroup)
        .def("hasNodeGroup", &mx::NodeDef::hasNodeGroup)
        .def("getNodeGroup", &mx::NodeDef::getNodeGroup)
        .def("getImplementation", &mx::NodeDef::getImplementation)
        .def("getImplementation", &mx::NodeDef::getImplementation,
            py::arg("target") = mx::EMPTY_STRING,
            py::arg("language") = mx::EMPTY_STRING)
        .def("getInstantiatingShaderRefs", &mx::NodeDef::getInstantiatingShaderRefs)
        .def("isVersionCompatible", &mx::NodeDef::isVersionCompatible)
        .def_readonly_static("CATEGORY", &mx::NodeDef::CATEGORY)
        .def_readonly_static("NODE_ATTRIBUTE", &mx::NodeDef::NODE_ATTRIBUTE);

    py::class_<mx::Implementation, mx::ImplementationPtr, mx::InterfaceElement>(mod, "Implementation")
        .def("setFile", &mx::Implementation::setFile)
        .def("hasFile", &mx::Implementation::hasFile)
        .def("getFile", &mx::Implementation::getFile)
        .def("setFunction", &mx::Implementation::setFunction)
        .def("hasFunction", &mx::Implementation::hasFunction)
        .def("getFunction", &mx::Implementation::getFunction)
        .def("setLanguage", &mx::Implementation::setLanguage)
        .def("hasLanguage", &mx::Implementation::hasLanguage)
        .def("getLanguage", &mx::Implementation::getLanguage)
        .def("setNodeDef", &mx::Implementation::setNodeDef)
        .def("getNodeDef", &mx::Implementation::getNodeDef)
        .def_readonly_static("CATEGORY", &mx::Implementation::CATEGORY)
        .def_readonly_static("FILE_ATTRIBUTE", &mx::Implementation::FILE_ATTRIBUTE)
        .def_readonly_static("FUNCTION_ATTRIBUTE", &mx::Implementation::FUNCTION_ATTRIBUTE)
        .def_readonly_static("LANGUAGE_ATTRIBUTE", &mx::Implementation::LANGUAGE_ATTRIBUTE);

    py::class_<mx::TypeDef, mx::TypeDefPtr, mx::Element>(mod, "TypeDef")
        .def("setSemantic", &mx::TypeDef::setSemantic)
        .def("hasSemantic", &mx::TypeDef::hasSemantic)
        .def("getSemantic", &mx::TypeDef::getSemantic)
        .def("setContext", &mx::TypeDef::setContext)
        .def("hasContext", &mx::TypeDef::hasContext)
        .def("getContext", &mx::TypeDef::getContext)
        .def("addMember", &mx::TypeDef::addMember,
            py::arg("name") = mx::EMPTY_STRING)
        .def("getMember", &mx::TypeDef::getMember)
        .def("getMembers", &mx::TypeDef::getMembers)
        .def("removeMember", &mx::TypeDef::removeMember)
        .def_readonly_static("CATEGORY", &mx::TypeDef::CATEGORY)
        .def_readonly_static("SEMANTIC_ATTRIBUTE", &mx::TypeDef::SEMANTIC_ATTRIBUTE)
        .def_readonly_static("CONTEXT_ATTRIBUTE", &mx::TypeDef::CONTEXT_ATTRIBUTE);

    py::class_<mx::Member, mx::MemberPtr, mx::TypedElement>(mod, "Member")
        .def_readonly_static("CATEGORY", &mx::TypeDef::CATEGORY);

    py::class_<mx::Unit, mx::UnitPtr, mx::Element>(mod, "Unit")
        .def_readonly_static("CATEGORY", &mx::Unit::CATEGORY);

    py::class_<mx::UnitDef, mx::UnitDefPtr, mx::Element>(mod, "UnitDef")
        .def("setUnitType", &mx::UnitDef::hasUnitType)
        .def("hasUnitType", &mx::UnitDef::hasUnitType)
        .def("getUnitType", &mx::UnitDef::getUnitType)
        .def("addUnit", &mx::UnitDef::addUnit)
        .def("getUnit", &mx::UnitDef::getUnit)
        .def("getUnits", &mx::UnitDef::getUnits)
        .def_readonly_static("CATEGORY", &mx::UnitDef::CATEGORY)
        .def_readonly_static("UNITTYPE_ATTRIBUTE", &mx::UnitDef::UNITTYPE_ATTRIBUTE);

    py::class_<mx::UnitTypeDef, mx::UnitTypeDefPtr, mx::Element>(mod, "UnitTypeDef")
        .def("setDefault", &mx::UnitTypeDef::setDefault)
        .def("hasDefault", &mx::UnitTypeDef::hasDefault)
        .def("getDefault", &mx::UnitTypeDef::getDefault)
        .def("getUnitDefs", &mx::UnitTypeDef::getUnitDefs)
        .def_readonly_static("CATEGORY", &mx::UnitTypeDef::CATEGORY)
        .def_readonly_static("DEFAULT_ATTRIBUTE", &mx::UnitTypeDef::DEFAULT_ATTRIBUTE);
}
