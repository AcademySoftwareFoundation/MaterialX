//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Definition.h>

#include <MaterialXCore/Material.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyDefinition(py::module& mod)
{
    py::class_<mx::NodeDef, mx::NodeDefPtr, mx::InterfaceElement>(mod, "NodeDef", "A node definition element within a Document.\n\nA NodeDef provides the declaration of a node interface, which may then be instantiated as a Node.")
        .def("setNodeString", &mx::NodeDef::setNodeString, "Set the node string of the NodeDef.")
        .def("hasNodeString", &mx::NodeDef::hasNodeString, "Return true if the given NodeDef has a node string.")
        .def("getNodeString", &mx::NodeDef::getNodeString, "Return the node string of the NodeDef.")
        .def("setNodeGroup", &mx::NodeDef::setNodeGroup, "Set the node group of the NodeDef.")
        .def("hasNodeGroup", &mx::NodeDef::hasNodeGroup, "Return true if the given NodeDef has a node group.")
        .def("getNodeGroup", &mx::NodeDef::getNodeGroup, "Return the node group of the NodeDef.")
        .def("getImplementation", &mx::NodeDef::getImplementation, "Return the Implementation, if any, with the given name.")
        .def("getImplementation", &mx::NodeDef::getImplementation,
            py::arg("target") = mx::EMPTY_STRING, "Return the Implementation, if any, with the given name.")
        .def("isVersionCompatible", &mx::NodeDef::isVersionCompatible, "Return true if the given version string is compatible with this NodeDef.\n\nThis may be used to test, for example, whether a NodeDef and Node may be used together.")
        .def_readonly_static("CATEGORY", &mx::NodeDef::CATEGORY)
        .def_readonly_static("NODE_ATTRIBUTE", &mx::NodeDef::NODE_ATTRIBUTE)
        .def_readonly_static("TEXTURE_NODE_GROUP", &mx::NodeDef::TEXTURE_NODE_GROUP)
        .def_readonly_static("PROCEDURAL_NODE_GROUP", &mx::NodeDef::PROCEDURAL_NODE_GROUP)
        .def_readonly_static("GEOMETRIC_NODE_GROUP", &mx::NodeDef::GEOMETRIC_NODE_GROUP)
        .def_readonly_static("ADJUSTMENT_NODE_GROUP", &mx::NodeDef::ADJUSTMENT_NODE_GROUP)
        .def_readonly_static("CONDITIONAL_NODE_GROUP", &mx::NodeDef::CONDITIONAL_NODE_GROUP)
        .def_readonly_static("CHANNEL_NODE_GROUP", &mx::NodeDef::CHANNEL_NODE_GROUP)
        .def_readonly_static("ORGANIZATION_NODE_GROUP", &mx::NodeDef::ORGANIZATION_NODE_GROUP)
        .def_readonly_static("TRANSLATION_NODE_GROUP", &mx::NodeDef::TRANSLATION_NODE_GROUP);

    py::class_<mx::Implementation, mx::ImplementationPtr, mx::InterfaceElement>(mod, "Implementation", "An implementation element within a Document.\n\nAn Implementation is used to associate external source code with a specific NodeDef, providing a definition for the node that may either be universal or restricted to a specific target.")
        .def("setFile", &mx::Implementation::setFile, "Set the file string for the Implementation.")
        .def("hasFile", &mx::Implementation::hasFile, "Return true if the given Implementation has a file string.")
        .def("getFile", &mx::Implementation::getFile, "Return the file string for the Implementation.")
        .def("setFunction", &mx::Implementation::setFunction, "Set the function string for the Implementation.")
        .def("hasFunction", &mx::Implementation::hasFunction, "Return true if the given Implementation has a function string.")
        .def("getFunction", &mx::Implementation::getFunction, "Return the function string for the Implementation.")
        .def("setNodeDef", &mx::Implementation::setNodeDef, "Set the NodeDef element referenced by the Implementation.")
        .def("getNodeDef", &mx::Implementation::getNodeDef, "Returns a nodedef for a given transform.")
        .def("setNodeGraph", &mx::Implementation::setNodeGraph, "Set the nodegraph string for the Implementation.")
        .def("hasNodeGraph", &mx::Implementation::hasNodeGraph, "Return true if the given Implementation has a nodegraph string.")        
        .def("getNodeGraph", &mx::Implementation::getNodeGraph, "Return the NodeGraph, if any, with the given name.")
        .def_readonly_static("CATEGORY", &mx::Implementation::CATEGORY)
        .def_readonly_static("FILE_ATTRIBUTE", &mx::Implementation::FILE_ATTRIBUTE)
        .def_readonly_static("FUNCTION_ATTRIBUTE", &mx::Implementation::FUNCTION_ATTRIBUTE);

    py::class_<mx::TypeDef, mx::TypeDefPtr, mx::Element>(mod, "TypeDef", "A type definition element within a Document.")
        .def("setSemantic", &mx::TypeDef::setSemantic, "Set the variable semantic of this port.")
        .def("hasSemantic", &mx::TypeDef::hasSemantic, "Return true if the given TypeDef has a semantic string.")
        .def("getSemantic", &mx::TypeDef::getSemantic, "Return the variable semantic of this port.")
        .def("setContext", &mx::TypeDef::setContext, "Set the context string of the TypeDef.")
        .def("hasContext", &mx::TypeDef::hasContext, "Return true if the given TypeDef has a context string.")
        .def("getContext", &mx::TypeDef::getContext, "Return the context string of the TypeDef.")
        .def("addMember", &mx::TypeDef::addMember,
            py::arg("name") = mx::EMPTY_STRING, "Add a Member to the TypeDef.\n\nArgs:\n    name: The name of the new Member. If no name is specified, then a unique name will automatically be generated.\n\nReturns:\n    A shared pointer to the new Member.")
        .def("getMember", &mx::TypeDef::getMember, "Return the Member, if any, with the given name.")
        .def("getMembers", &mx::TypeDef::getMembers)
        .def("removeMember", &mx::TypeDef::removeMember, "Remove the Member, if any, with the given name.")
        .def_readonly_static("CATEGORY", &mx::TypeDef::CATEGORY)
        .def_readonly_static("SEMANTIC_ATTRIBUTE", &mx::TypeDef::SEMANTIC_ATTRIBUTE)
        .def_readonly_static("CONTEXT_ATTRIBUTE", &mx::TypeDef::CONTEXT_ATTRIBUTE);

    py::class_<mx::Member, mx::MemberPtr, mx::TypedElement>(mod, "Member", "A member element within a TypeDef.")
        .def_readonly_static("CATEGORY", &mx::TypeDef::CATEGORY);

    py::class_<mx::Unit, mx::UnitPtr, mx::Element>(mod, "Unit", "A unit declaration within a UnitDef.")
        .def_readonly_static("CATEGORY", &mx::Unit::CATEGORY);

    py::class_<mx::UnitDef, mx::UnitDefPtr, mx::Element>(mod, "UnitDef", "A unit definition element within a Document.")
        .def("setUnitType", &mx::UnitDef::setUnitType, "Set the element's unittype string.")
        .def("hasUnitType", &mx::UnitDef::hasUnitType, "Return true if the given element has a unittype string.")
        .def("getUnitType", &mx::UnitDef::getUnitType, "Return the unit type string.")
        .def("addUnit", &mx::UnitDef::addUnit, "Add a Unit to the UnitDef.\n\nArgs:\n    name: The name of the new Unit. An exception is thrown if the name provided is an empty string.\n\nReturns:\n    A shared pointer to the new Unit.")
        .def("getUnit", &mx::UnitDef::getUnit, "Return the unit type for the value on this port.")
        .def("getUnits", &mx::UnitDef::getUnits, "Return a vector of all Unit elements in the UnitDef.")
        .def_readonly_static("CATEGORY", &mx::UnitDef::CATEGORY)
        .def_readonly_static("UNITTYPE_ATTRIBUTE", &mx::UnitDef::UNITTYPE_ATTRIBUTE);

    py::class_<mx::UnitTypeDef, mx::UnitTypeDefPtr, mx::Element>(mod, "UnitTypeDef", "A unit type definition element within a Document.")
        .def("getUnitDefs", &mx::UnitTypeDef::getUnitDefs, "Return a vector of all Member elements in the TypeDef.")
        .def_readonly_static("CATEGORY", &mx::UnitTypeDef::CATEGORY);

    py::class_<mx::AttributeDef, mx::AttributeDefPtr, mx::TypedElement>(mod, "AttributeDef", "An attribute definition element within a Document.")
        .def("setAttrName", &mx::AttributeDef::setAttrName, "Set the element's attrname string.")
        .def("hasAttrName", &mx::AttributeDef::hasAttrName, "Return true if this element has an attrname string.")
        .def("getAttrName", &mx::AttributeDef::getAttrName, "Return the element's attrname string.")
        .def("setValueString", &mx::AttributeDef::setValueString, "Set the value string of an element.")
        .def("hasValueString", &mx::AttributeDef::hasValueString, "Return true if the given element has a value string.")
        .def("getValueString", &mx::AttributeDef::getValueString, "Return value string.") 
        .def("setExportable", &mx::AttributeDef::setExportable, "Set the exportable boolean for the element.")         
        .def("getExportable", &mx::AttributeDef::getExportable, "Return the exportable boolean for the element.\n\nDefaults to false if exportable is not set.")         
        .def_readonly_static("CATEGORY", &mx::AttributeDef::CATEGORY);

    py::class_<mx::TargetDef, mx::TargetDefPtr, mx::TypedElement>(mod, "TargetDef", "A definition of an implementation target.")
        .def("getMatchingTargets", &mx::TargetDef::getMatchingTargets, "Return a vector of target names that is matching this targetdef either by itself of by its inheritance.\n\nThe vector is ordered by priority starting with this targetdef itself and then upwards in the inheritance hierarchy.")
        .def_readonly_static("CATEGORY", &mx::TargetDef::CATEGORY);
}
