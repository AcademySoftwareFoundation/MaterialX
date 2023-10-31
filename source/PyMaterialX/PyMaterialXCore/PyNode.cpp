//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Node.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyNode(py::module& mod)
{
    py::class_<mx::NodePredicate>(mod, "NodePredicate")
        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a function that takes a `Node` and returns a `bool`,
    to check whether some criteria has passed.
)docstring");

    py::class_<mx::Node, mx::NodePtr, mx::InterfaceElement>(mod, "Node")

        .def("setConnectedNode", &mx::Node::setConnectedNode,
             py::arg("inputName"),
             py::arg("node"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the node to which the given input is connected, creating a child input
    if needed. If the `node` argument is `None`, then any existing node
    connection on the input will be cleared.
)docstring"))

        .def("getConnectedNode", &mx::Node::getConnectedNode,
             py::arg("inputName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Node` connected to the specified input. If the specified input
    is not present, then `None` is returned.
)docstring"))

        .def("setConnectedNodeName", &mx::Node::setConnectedNodeName,
             py::arg("inputName"),
             py::arg("nodeName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the name of the `Node` connected to the specified input, creating a
    child element for the input if needed.
)docstring"))

        .def("getConnectedNodeName", &mx::Node::getConnectedNodeName,
             py::arg("inputName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the name of the `Node` connected to the specified input. If the
    specified input is not present, then an empty string is returned.
)docstring"))

        .def("getNodeDef", &mx::Node::getNodeDef,
             py::arg("target") = mx::EMPTY_STRING,
             py::arg("allowRoughMatch") = false,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the first `NodeDef` that declares this node, optionally filtered
    by the given `target` name.

    :type target: str
    :param target: An optional target name, which will be used to filter
        the nodedefs that are considered.
    :type allowRoughMatch: bool
    :param allowRoughMatch: If specified, then a rough match will be allowed
        when an exact match is not found. An exact match requires that each
        node input corresponds to a nodedef input of the same name and type.
    :returns: A `NodeDef` for this node, or `None` if none was found.
)docstring"))

        .def("getImplementation", &mx::Node::getImplementation,
             py::arg("target") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the first implementation for this node, optionally filtered by
    the given `target` and language names.

    :type target: str
    :param target: An optional target name, which will be used to filter
        the implementations that are considered.
    :returns: An implementation for this node, or `None` if none was found.
        Note that a node implementation may be either an `Implementation`
        element or a `NodeGraph` element.
)docstring"))

        .def("getDownstreamPorts", &mx::Node::getDownstreamPorts,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all downstream ports that connect to this node, ordered by
    the names of the port elements.
)docstring"))

        .def("addInputFromNodeDef", &mx::Node::addInputFromNodeDef,
             py::arg("inputName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add an input based on the corresponding input for the associated node
    definition. If the input already exists on the node, then it will just be
    returned.
)docstring"))

        .def("addInputsFromNodeDef", &mx::Node::addInputsFromNodeDef,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add inputs based on the corresponding associated node definition.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::Node::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Node` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A node element within a `NodeGraph` or `Document`.

    A `Node` represents an instance of a `NodeDef` within a graph, and its `Input`
    elements apply specific values and connections to that instance.

    :see: https://materialx.org/docs/api/class_node.html
)docstring");

    py::class_<mx::GraphElement, mx::GraphElementPtr, mx::InterfaceElement>(mod, "GraphElement")

        .def("addNode", &mx::GraphElement::addNode,
             py::arg("category"),
             py::arg("name") = mx::EMPTY_STRING,
             py::arg_v("nodeType",
                       mx::DEFAULT_TYPE_STRING,
                       "mx.DEFAULT_TYPE_STRING"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `Node` to the graph.

    :type category: str
    :param category: The category of the new `Node`.
    :type name: str
    :param name: The name of the new `Node`.
        If no name is specified, then a unique name will automatically be
        generated.
    :type nodeType: str
    :param nodeType: An optional type string.
    :returns: The new `Node`.
)docstring"))

        .def("addNodeInstance", &mx::GraphElement::addNodeInstance,
             py::arg("nodeDef"),
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `Node` that is an instance of the given `NodeDef`.
)docstring"))

        .def("getNode", &mx::GraphElement::getNode,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Node`, if any, with the given `name`.
)docstring"))

        .def("getNodes", &mx::GraphElement::getNodes,
             py::arg("category") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Node`s in the graph, optionally filtered by the
    given `category` string.
)docstring"))

        .def("removeNode", &mx::GraphElement::removeNode,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Node`, if any, with the given `name`.
)docstring"))

        .def("addMaterialNode", &mx::GraphElement::addMaterialNode,
             py::arg("name") = mx::EMPTY_STRING,
             py::arg("shaderNode") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a material node to the graph, optionally connecting it to the given
    shader node.
)docstring"))

        .def("getMaterialNodes", &mx::GraphElement::getMaterialNodes,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all material nodes.
)docstring"))

        .def("addBackdrop", &mx::GraphElement::addBackdrop,
             py::arg("name") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a `Backdrop` to the graph.
)docstring"))

        .def("getBackdrop", &mx::GraphElement::getBackdrop,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `Backdrop`, if any, with the given `name`.
)docstring"))

        .def("getBackdrops", &mx::GraphElement::getBackdrops,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all `Backdrop` elements in the graph.
)docstring"))

        .def("removeBackdrop", &mx::GraphElement::removeBackdrop,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the `Backdrop`, if any, with the given `name`.
)docstring"))

        .def("flattenSubgraphs", &mx::GraphElement::flattenSubgraphs,
             py::arg("target") = mx::EMPTY_STRING,
             py::arg("nodeFilter") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Flatten all subgraphs at the root scope of this graph element,
    recursively replacing each graph-defined node with its equivalent
    node network.

    :type target: str
    :param target: An optional target string to be used in specifying
        which node definitions are used in this process.
    :type nodeFilter: `NodePredicate`
    :param nodeFilter: An optional node predicate specifying which nodes
        should be included and excluded from this process.
)docstring"))

        .def("topologicalSort", &mx::GraphElement::topologicalSort,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all children (nodes and outputs) sorted in topological
    order.
)docstring"))

        .def("addGeomNode", &mx::GraphElement::addGeomNode,
             py::arg("geomPropDef"),
             py::arg("namePrefix"),
             PYMATERIALX_DOCSTRING(R"docstring(
    If not yet present, add a geometry node to this graph matching the given
    property definition and name prefix.
)docstring"))

        .def("asStringDot", &mx::GraphElement::asStringDot,
             PYMATERIALX_DOCSTRING(R"docstring(
    Convert this graph to a string in the DOT language syntax. This can be
    used to visualise the graph using Graphviz.

    If declarations for the contained nodes are provided as nodedefs in
    the owning document, then they will be used to provide additional
    formatting details.

    :see: https://www.graphviz.org
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for graph elements.

    Inherited by: `NodeGraph` and `Document`.

    :see: https://materialx.org/docs/api/class_graph_element.html
)docstring");

    py::class_<mx::NodeGraph, mx::NodeGraphPtr, mx::GraphElement>(mod, "NodeGraph")

        .def("getMaterialOutputs", &mx::NodeGraph::getMaterialOutputs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return all material-type outputs of the `NodeGraph`.
)docstring"))

        .def("setNodeDef", &mx::NodeGraph::setNodeDef,
             py::arg("nodeDef"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the `NodeDef` element referenced by this `NodeGraph`.
)docstring"))

        .def("getNodeDef", &mx::NodeGraph::getNodeDef,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the `NodeDef` element referenced by this `NodeGraph`.
)docstring"))

        .def("getDeclaration", &mx::NodeGraph::getDeclaration,
             py::arg("target") = mx::EMPTY_STRING,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the first declaration of this interface, optionally filtered by the
    given `target` name.
)docstring"))

        .def("addInterfaceName", &mx::NodeGraph::addInterfaceName,
             py::arg("inputPath"),
             py::arg("interfaceName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add an interface name to an existing `NodeDef` associated with this
    `NodeGraph`.

    :param inputPath: Path to an input descendant of this graph.
    :param interfaceName: The new interface name.
)docstring"))

        .def("removeInterfaceName", &mx::NodeGraph::removeInterfaceName,
             py::arg("inputPath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove an interface name from an existing `NodeDef` associated with this
    `NodeGraph`.

    :param inputPath: Path to an input descendant of this graph.
)docstring"))

        .def("modifyInterfaceName", &mx::NodeGraph::modifyInterfaceName,
             py::arg("inputPath"),
             py::arg("interfaceName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Modify the interface name on an existing `NodeDef` associated with this
    `NodeGraph`.

    :param inputPath: Path to an input descendant of this graph.
    :param interfaceName: The new interface name.
)docstring"))

        .def("getDownstreamPorts", &mx::NodeGraph::getDownstreamPorts,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all downstream ports that connect to this graph, ordered
    by the names of the port elements.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::NodeGraph::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `NodeGraph` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a node graph element within a `Document`.

    :see: https://materialx.org/docs/api/class_node_graph.html
)docstring");

    py::class_<mx::Backdrop, mx::BackdropPtr, mx::Element>(mod, "Backdrop")

        .def("setContainsString", &mx::Backdrop::setContainsString,
             py::arg("contains"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the contains string for this backdrop.
)docstring"))

        .def("hasContainsString", &mx::Backdrop::hasContainsString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this backdrop has a contains string.
)docstring"))

        .def("getContainsString", &mx::Backdrop::getContainsString,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the contains string for this backdrop.
)docstring"))

        .def("setWidth", &mx::Backdrop::setWidth,
             py::arg("width"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the width attribute of the backdrop.
)docstring"))

        .def("hasWidth", &mx::Backdrop::hasWidth,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this backdrop has a width attribute.
)docstring"))

        .def("getWidth", &mx::Backdrop::getWidth,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the width attribute of the backdrop.
)docstring"))

        .def("setHeight", &mx::Backdrop::setHeight,
             py::arg("height"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the height attribute of the backdrop.
)docstring"))

        .def("hasHeight", &mx::Backdrop::hasHeight,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this backdrop has a height attribute.
)docstring"))

        .def("getHeight", &mx::Backdrop::getHeight,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the height attribute of the backdrop.
)docstring"))

        .def("setContainsElements", &mx::Backdrop::setContainsElements,
             py::arg("nodes"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the list of elements that this backdrop contains.
)docstring"))

        .def("getContainsElements", &mx::Backdrop::getContainsElements,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the list of elements that this backdrop contains.
)docstring"))

        .def_readonly_static("CATEGORY", &mx::Backdrop::CATEGORY,
                             PYMATERIALX_DOCSTRING(R"docstring(
    A string representing the role of `Backdrop` elements within a MaterialX
    document.

    :see: `setCategory()`
    :see: `getCategory()`
)docstring"))

        .def_readonly_static("CONTAINS_ATTRIBUTE", &mx::Backdrop::CONTAINS_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's contains string is stored as an attribute.

    :see: `setContainsString()`
    :see: `hasContainsString()`
    :see: `getContainsString()`
)docstring"))

        .def_readonly_static("WIDTH_ATTRIBUTE", &mx::Backdrop::WIDTH_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's width attribute is stored.

    :see: `setWidth()`
    :see: `hasWidth()`
    :see: `getWidth()`
)docstring"))

        .def_readonly_static("HEIGHT_ATTRIBUTE", &mx::Backdrop::HEIGHT_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The name under which an element's height attribute is stored.

    :see: `setHeight()`
    :see: `hasHeight()`
    :see: `getHeight()`
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a layout element used to contain, group, and document
    nodes within a graph.

    :see: https://materialx.org/docs/api/class_backdrop.html
)docstring");
}
