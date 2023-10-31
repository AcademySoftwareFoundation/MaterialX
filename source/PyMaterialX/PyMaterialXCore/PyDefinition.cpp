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
    py::class_<mx::NodeDef, mx::NodeDefPtr, mx::InterfaceElement>(mod, "NodeDef")

        .def("setNodeString", &mx::NodeDef::setNodeString,
             py::arg("node"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the node string of the NodeDef.
)docstring"))

        .def("hasNodeString", &mx::NodeDef::hasNodeString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this NodeDef has a node string.
)docstring"))

        .def("getNodeString", &mx::NodeDef::getNodeString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the node string of the NodeDef.
)docstring"))

        .def("setNodeGroup", &mx::NodeDef::setNodeGroup,
             py::arg("category"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the node group of the NodeDef.
)docstring"))

        .def("hasNodeGroup", &mx::NodeDef::hasNodeGroup,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this NodeDef has a node group.
)docstring"))

        .def("getNodeGroup", &mx::NodeDef::getNodeGroup,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the node group of the NodeDef.
)docstring"))

        .def("getImplementation", &mx::NodeDef::getImplementation,
             py::arg("target") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the first implementation for this nodedef, optionally filtered
    by the given `target` name.

    :type target: str
    :param target: An optional target name, which will be used to filter
        the implementations that are considered.
    :returns: An implementation for this nodedef, or `None` if none was found.
        Note that a node implementation may be either an `Implementation`
        element or a `NodeGraph` element.
)docstring"))

        .def("isVersionCompatible", &mx::NodeDef::isVersionCompatible,
             py::arg("version"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the given `version` string is compatible with this NodeDef.
    This may be used to test, for example, whether a `NodeDef` and `Node` may
    be used together.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::NodeDef::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `NodeDef` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("NODE_ATTRIBUTE", &mx::NodeDef::NODE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which a definition's node string is stored as an attribute.

    :see: `setNodeString()`
    :see: `hasNodeString()`
    :see: `getNodeString()`
)docstring"))

        .def_readonly_static("TEXTURE_NODE_GROUP", &mx::NodeDef::TEXTURE_NODE_GROUP,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The node group name for texture nodes.

    :see: `setNodeGroup()`
    :see: `hasNodeGroup()`
    :see: `getNodeGroup()`
)docstring"))

        .def_readonly_static("PROCEDURAL_NODE_GROUP", &mx::NodeDef::PROCEDURAL_NODE_GROUP,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The node group name for procedural nodes.

    :see: `setNodeGroup()`
    :see: `hasNodeGroup()`
    :see: `getNodeGroup()`
)docstring"))

        .def_readonly_static("GEOMETRIC_NODE_GROUP", &mx::NodeDef::GEOMETRIC_NODE_GROUP,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The node group name for geometric nodes.

    :see: `setNodeGroup()`
    :see: `hasNodeGroup()`
    :see: `getNodeGroup()`
)docstring"))

        .def_readonly_static("ADJUSTMENT_NODE_GROUP", &mx::NodeDef::ADJUSTMENT_NODE_GROUP,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The node group name for adjustment nodes.

    :see: `setNodeGroup()`
    :see: `hasNodeGroup()`
    :see: `getNodeGroup()`
)docstring"))

        .def_readonly_static("CONDITIONAL_NODE_GROUP", &mx::NodeDef::CONDITIONAL_NODE_GROUP,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The node group name for conditional nodes.

    :see: `setNodeGroup()`
    :see: `hasNodeGroup()`
    :see: `getNodeGroup()`
)docstring"))

        .def_readonly_static("ORGANIZATION_NODE_GROUP", &mx::NodeDef::ORGANIZATION_NODE_GROUP,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The node group name for organizational nodes.

    :see: `setNodeGroup()`
    :see: `hasNodeGroup()`
    :see: `getNodeGroup()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a node definition element within a `Document`.

    A `NodeDef` provides the declaration of a node interface, which may then
    be instantiated as a `Node`.

    :see: https://materialx.org/docs/api/class_node_def.html
)docstring");

    py::class_<mx::Implementation, mx::ImplementationPtr, mx::InterfaceElement>(mod, "Implementation")

        .def("setFile", &mx::Implementation::setFile,
             py::arg("fileString"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the file string for the Implementation.
)docstring"))

        .def("hasFile", &mx::Implementation::hasFile,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this Implementation has a file string.
)docstring"))

        .def("getFile", &mx::Implementation::getFile,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the file string for the Implementation.
)docstring"))

        .def("setFunction", &mx::Implementation::setFunction,
             py::arg("function"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the function string for the Implementation.
)docstring"))

        .def("hasFunction", &mx::Implementation::hasFunction,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this Implementation has a function string.
)docstring"))

        .def("getFunction", &mx::Implementation::getFunction,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the function string for the Implementation.
)docstring"))

        .def("setNodeDef", &mx::Implementation::setNodeDef,
             py::arg("nodeDef"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the `NodeDef` element referenced by the Implementation.
)docstring"))

        .def("getNodeDef", &mx::Implementation::getNodeDef,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `NodeDef` element referenced by the Implementation.
)docstring"))

        .def("setNodeGraph", &mx::Implementation::setNodeGraph,
             py::arg("nodegraph"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the nodegraph string for the Implementation.
)docstring"))

        .def("hasNodeGraph", &mx::Implementation::hasNodeGraph,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this Implementation has a nodegraph string.
)docstring"))

        .def("getNodeGraph", &mx::Implementation::getNodeGraph,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the nodegraph string for the Implementation.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::Implementation::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Implementation` elements within a
    MaterialX document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("FILE_ATTRIBUTE", &mx::Implementation::FILE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an Implementation's file string is stored as an
    attribute.

    :see: `setFile()`
    :see: `hasFile()`
    :see: `getFile()`
)docstring"))

        .def_readonly_static("FUNCTION_ATTRIBUTE", &mx::Implementation::FUNCTION_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an Implementation's function string is stored as an
    attribute.

    :see: `setFunction()`
    :see: `hasFunction()`
    :see: `getFunction()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing an implementation element within a `Document`.

    An `Implementation` is used to associate external source code with a specific
    `NodeDef`, providing a definition for the node that may either be universal or
    restricted to a specific target.

    :see: https://materialx.org/docs/api/class_implementation.html
)docstring");

    py::class_<mx::TypeDef, mx::TypeDefPtr, mx::Element>(mod, "TypeDef")

        .def("setSemantic", &mx::TypeDef::setSemantic,
             py::arg("semantic"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the semantic string of the TypeDef.
)docstring"))

        .def("hasSemantic", &mx::TypeDef::hasSemantic,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this TypeDef has a semantic string.
)docstring"))

        .def("getSemantic", &mx::TypeDef::getSemantic,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the semantic string of the TypeDef.
)docstring"))

        .def("setContext", &mx::TypeDef::setContext,
             py::arg("context"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the context string of the TypeDef.
)docstring"))

        .def("hasContext", &mx::TypeDef::hasContext,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this TypeDef has a context string.
)docstring"))

        .def("getContext", &mx::TypeDef::getContext,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the context string of the TypeDef.
)docstring"))

        .def("addMember", &mx::TypeDef::addMember,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `Member` to the `TypeDef`.

    :type name: str
    :param name: The name of the new `Member`.
        If no name is specified, then a unique name will automatically be
        generated.
    :returns: The new `Member`.
)docstring"))

        .def("getMember", &mx::TypeDef::getMember,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Member`, if any, with the given `name`.
)docstring"))

        .def("getMembers", &mx::TypeDef::getMembers,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Member` elements in the TypeDef.
)docstring"))

        .def("removeMember", &mx::TypeDef::removeMember,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Member`, if any, with the given `name`.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::TypeDef::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `TypeDef` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("SEMANTIC_ATTRIBUTE", &mx::TypeDef::SEMANTIC_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which a TypeDef's semantic string is stored as an attribute.

    :see: `setSemantic()`
    :see: `hasSemantic()`
    :see: `getSemantic()`
)docstring"))

        .def_readonly_static("CONTEXT_ATTRIBUTE", &mx::TypeDef::CONTEXT_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which a TypeDef's context string is stored as an attribute.

    :see: `setContext()`
    :see: `hasContext()`
    :see: `getContext()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a type definition element within a `Document`.

    :see: https://materialx.org/docs/api/class_type_def.html
)docstring");

    py::class_<mx::Member, mx::MemberPtr, mx::TypedElement>(mod, "Member")

        .def_readonly_static("CATEGORY", &mx::Member::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Member` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a member element within a `TypeDef`.

    :see: https://materialx.org/docs/api/class_member.html
)docstring");

    py::class_<mx::Unit, mx::UnitPtr, mx::Element>(mod, "Unit")

        .def_readonly_static("CATEGORY", &mx::Unit::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Unit` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a unit declaration element within a `UnitDef`.

    :see: https://materialx.org/docs/api/class_unit.html
)docstring");

    py::class_<mx::UnitDef, mx::UnitDefPtr, mx::Element>(mod, "UnitDef")

        .def("setUnitType", &mx::UnitDef::setUnitType,
             py::arg("unitType"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the element's unittype string.
)docstring"))

        .def("hasUnitType", &mx::UnitDef::hasUnitType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a unittype string.
)docstring"))

        .def("getUnitType", &mx::UnitDef::getUnitType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element's unittype string.
)docstring"))

        .def("addUnit", &mx::UnitDef::addUnit,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `Unit` to the `UnitDef`.

    :type name: str
    :param name: The name of the new `Unit`.
    :returns: The new `Unit`.
    :raises LookupError: If an empty name is given.
)docstring"))

        .def("getUnit", &mx::UnitDef::getUnit,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Unit`, if any, with the given `name`.
)docstring"))

        .def("getUnits", &mx::UnitDef::getUnits,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Unit` elements in the `UnitDef`.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::UnitDef::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `UnitDef` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("UNITTYPE_ATTRIBUTE", &mx::UnitDef::UNITTYPE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which a UnitDef's unittype string is stored as an attribute.

    :see: `setUnitType()`
    :see: `hasUnitType()`
    :see: `getUnitType()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a unit definition element within a `Document`.

    :see: https://materialx.org/docs/api/class_unit_def.html
)docstring");

    py::class_<mx::UnitTypeDef, mx::UnitTypeDefPtr, mx::Element>(mod, "UnitTypeDef")

        .def("getUnitDefs", &mx::UnitTypeDef::getUnitDefs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `UnitDef` elements in this `UnitTypeDef`.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::UnitTypeDef::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `UnitTypeDef` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a unit type definition element within a `Document`.

    :see: https://materialx.org/docs/api/class_unit_type_def.html
)docstring");

    py::class_<mx::AttributeDef, mx::AttributeDefPtr, mx::TypedElement>(mod, "AttributeDef")

        .def("setAttrName", &mx::AttributeDef::setAttrName,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the element's attrname string.
)docstring"))

        .def("hasAttrName", &mx::AttributeDef::hasAttrName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has an attrname string.
)docstring"))

        .def("getAttrName", &mx::AttributeDef::getAttrName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element's attrname string.
)docstring"))

        .def("setValueString", &mx::AttributeDef::setValueString,
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the value string of the element.
)docstring"))

        .def("hasValueString", &mx::AttributeDef::hasValueString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has a value string.
)docstring"))

        .def("getValueString", &mx::AttributeDef::getValueString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the value string of the element.
)docstring"))

        .def("setElements", &mx::AttributeDef::setElements,
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the elements string of the element.
)docstring"))

        .def("hasElements", &mx::AttributeDef::hasElements,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this element has an elements string.
)docstring"))

        .def("getElements", &mx::AttributeDef::getElements,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the elements string of the element.
)docstring"))

        .def("setExportable", &mx::AttributeDef::setExportable,
             py::arg("value"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the exportable flag for the element.
)docstring"))

        .def("getExportable", &mx::AttributeDef::getExportable,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the exportable flag for the element.
    Defaults to `False` if exportable is not set.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::AttributeDef::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `AttributeDef` elements within a
    MaterialX document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("ATTRNAME_ATTRIBUTE",
                             &mx::AttributeDef::ATTRNAME_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an `AttributeDef` element's attribute name string is
    stored as an attribute.

    :see: `setAttrName()`
    :see: `hasAttrName()`
    :see: `getAttrName()`
)docstring"))

        .def_readonly_static("VALUE_ATTRIBUTE",
                             &mx::AttributeDef::VALUE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an `AttributeDef` element's value string is stored as
    an attribute.

    :see: `setValueString()`
    :see: `hasValueString()`
    :see: `getValueString()`
)docstring"))

        .def_readonly_static("ELEMENTS_ATTRIBUTE",
                             &mx::AttributeDef::ELEMENTS_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an `AttributeDef` element's elements string is stored
    as an attribute.

    :see: `setElements()`
    :see: `hasElements()`
    :see: `getElements()`
)docstring"))

        .def_readonly_static("EXPORTABLE_ATTRIBUTE",
                             &mx::AttributeDef::EXPORTABLE_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an `AttributeDef` element's exportable flag is stored
    as an attribute.

    :see: `setExportable()`
    :see: `getExportable()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing an attribute definition element within a `Document`.

    :see: https://materialx.org/docs/api/class_attribute_def.html
)docstring");

    py::class_<mx::TargetDef, mx::TargetDefPtr, mx::TypedElement>(mod, "TargetDef")

        .def("getMatchingTargets", &mx::TargetDef::getMatchingTargets,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of target names that are matching this `TargetDef`
    either by itself of by its inheritance.

    The list is ordered by priority starting with this `TargetDef`
    itself and then upwards in the inheritance hierarchy.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::TargetDef::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `TargetDef` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing the definition of an implementation target as a `TypedElement`.

    :see: https://materialx.org/docs/api/class_target_def.html
)docstring");
}
