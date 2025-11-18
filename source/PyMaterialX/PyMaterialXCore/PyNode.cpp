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
    py::class_<mx::NodePredicate>(mod, "NodePredicate");

    py::class_<mx::Node, mx::NodePtr, mx::InterfaceElement>(mod, "Node", "A node element within a NodeGraph or Document.\n\nA Node represents an instance of a NodeDef within a graph, and its Input elements apply specific values and connections to that instance.")
        .def("setConnectedNode", &mx::Node::setConnectedNode, "Set the node to which the given input is connected, creating a child input if needed.\n\nIf the node argument is null, then any existing node connection on the input will be cleared.")
        .def("getConnectedNode", &mx::Node::getConnectedNode, "Return the Node connected to the given input.\n\nIf the given input is not present, then an empty NodePtr is returned.")
        .def("setConnectedNodeName", &mx::Node::setConnectedNodeName, "Set the name of the Node connected to the given input, creating a child element for the input if needed.")
        .def("getConnectedNodeName", &mx::Node::getConnectedNodeName, "Return the name of the Node connected to the given input.\n\nIf the given input is not present, then an empty string is returned.")
        .def("getNodeDef", &mx::Node::getNodeDef, py::arg("target") = mx::EMPTY_STRING, py::arg("allowRoughMatch") = false, "Return the first NodeDef that declares this node, optionally filtered by the given target name.\n\nArgs:\n    target: An optional target name, which will be used to filter the nodedefs that are considered.\n    allowRoughMatch: If specified, then a rough match will be allowed when an exact match is not found. An exact match requires that each node input corresponds to a nodedef input of the same name and type.\n\nReturns:\n    A NodeDef for this node, or an empty shared pointer if none was found.")
        .def("getImplementation", &mx::Node::getImplementation, py::arg("target") = mx::EMPTY_STRING, "Return the first implementation for this node, optionally filtered by the given target and language names.\n\nArgs:\n    target: An optional target name, which will be used to filter the implementations that are considered.\n\nReturns:\n    An implementation for this node, or an empty shared pointer if none was found. Note that a node implementation may be either an Implementation element or a NodeGraph element.")
        .def("getDownstreamPorts", &mx::Node::getDownstreamPorts, "Return a vector of all downstream ports that connect to this node, ordered by the names of the port elements.")
        .def("addInputFromNodeDef", &mx::Node::addInputFromNodeDef, "Add an input based on the corresponding input for the associated node definition.\n\nIf the input already exists on the node it will just be returned.")
        .def("addInputsFromNodeDef", &mx::Node::addInputsFromNodeDef, "Add inputs based on the corresponding associated node definition.")
        .def_readonly_static("CATEGORY", &mx::Node::CATEGORY);

    py::class_<mx::GraphElement, mx::GraphElementPtr, mx::InterfaceElement>(mod, "GraphElement", "The base class for graph elements such as NodeGraph and Document.")
        .def("addNode", &mx::GraphElement::addNode, py::arg("category"), py::arg("name") = mx::EMPTY_STRING, py::arg("type") = mx::DEFAULT_TYPE_STRING, "Add a Node to the graph.\n\nArgs:\n    category: The category of the new Node.\n    name: The name of the new Node. If no name is specified, then a unique name will automatically be generated.\n    type: An optional type string.\n\nReturns:\n    A shared pointer to the new Node.")
        .def("addNodeInstance", &mx::GraphElement::addNodeInstance, py::arg("nodeDef"), py::arg("name") = mx::EMPTY_STRING, "Add a Node that is an instance of the given NodeDef.")
        .def("getNode", &mx::GraphElement::getNode, "Return the Node, if any, with the given name.")
        .def("getNodes", &mx::GraphElement::getNodes, py::arg("category") = mx::EMPTY_STRING, "Return a vector of all Nodes in the graph, optionally filtered by the given category string.")
        .def("removeNode", &mx::GraphElement::removeNode, "Remove the Node, if any, with the given name.")
        .def("addMaterialNode", &mx::GraphElement::addMaterialNode, py::arg("name") = mx::EMPTY_STRING, py::arg("shaderNode") = nullptr, "Add a material node to the graph, optionally connecting it to the given shader node.")
        .def("getMaterialNodes", &mx::GraphElement::getMaterialNodes, "Return a vector of all material nodes.")
        .def("addBackdrop", &mx::GraphElement::addBackdrop, py::arg("name") = mx::EMPTY_STRING, "Add a Backdrop to the graph.")
        .def("getBackdrop", &mx::GraphElement::getBackdrop, "Return the Backdrop, if any, with the given name.")
        .def("getBackdrops", &mx::GraphElement::getBackdrops, "Return a vector of all Backdrop elements in the graph.")
        .def("removeBackdrop", &mx::GraphElement::removeBackdrop, "Remove the Backdrop, if any, with the given name.")
        .def("flattenSubgraphs", &mx::GraphElement::flattenSubgraphs, py::arg("target") = mx::EMPTY_STRING, py::arg("filter") = nullptr, "Flatten all subgraphs at the root scope of this graph element, recursively replacing each graph-defined node with its equivalent node network.\n\nArgs:\n    target: An optional target string to be used in specifying which node definitions are used in this process.\n    filter: An optional node predicate specifying which nodes should be included and excluded from this process.")
        .def("topologicalSort", &mx::GraphElement::topologicalSort, "Return a vector of all children (nodes and outputs) sorted in topological order.")
        .def("addGeomNode", &mx::GraphElement::addGeomNode, "If not yet present, add a geometry node to this graph matching the given property definition and name prefix.")
        .def("asStringDot", &mx::GraphElement::asStringDot, "Convert this graph to a string in the DOT language syntax.\n\nThis can be used to visualise the graph using GraphViz (http://www.graphviz.org).\n\nIf declarations for the contained nodes are provided as nodedefs in the owning document, then they will be used to provide additional formatting details.");

    py::class_<mx::NodeGraph, mx::NodeGraphPtr, mx::GraphElement>(mod, "NodeGraph", "A node graph element within a Document.")
        .def("getMaterialOutputs", &mx::NodeGraph::getMaterialOutputs, "Return all material-type outputs of the nodegraph.")        
        .def("setNodeDef", &mx::NodeGraph::setNodeDef, "Set the NodeDef element referenced by this NodeGraph.")
        .def("getNodeDef", &mx::NodeGraph::getNodeDef, "Return the NodeDef element referenced by this NodeGraph.")
        .def("getDeclaration", &mx::NodeGraph::getDeclaration, "Return the first declaration of this interface, optionally filtered by the given target name.")
        .def("addInterfaceName", &mx::NodeGraph::addInterfaceName, "Add an interface name to an existing NodeDef associated with this NodeGraph.\n\nArgs:\n    inputPath: Path to an input descendant of this graph.\n    interfaceName: The new interface name.\n\nReturns:\n    Interface input.")
        .def("removeInterfaceName", &mx::NodeGraph::removeInterfaceName, "Remove an interface name from an existing NodeDef associated with this NodeGraph.\n\nArgs:\n    inputPath: Path to an input descendant of this graph.")
        .def("modifyInterfaceName", &mx::NodeGraph::modifyInterfaceName, "Modify the interface name on an existing NodeDef associated with this NodeGraph.\n\nArgs:\n    inputPath: Path to an input descendant of this graph.\n    interfaceName: The new interface name.")
        .def("getDownstreamPorts", &mx::NodeGraph::getDownstreamPorts, "Return a vector of all downstream ports that connect to this graph, ordered by the names of the port elements.")
        .def_readonly_static("CATEGORY", &mx::NodeGraph::CATEGORY);

    py::class_<mx::Backdrop, mx::BackdropPtr, mx::Element>(mod, "Backdrop", "A layout element used to contain, group and document nodes within a graph.")
        .def("setContainsString", &mx::Backdrop::setContainsString, "Set the contains string for this backdrop.")
        .def("hasContainsString", &mx::Backdrop::hasContainsString, "Return true if this backdrop has a contains string.")
        .def("getContainsString", &mx::Backdrop::getContainsString, "Return the contains string for this backdrop.")
        .def("setWidth", &mx::Backdrop::setWidth, "Set the width attribute of the backdrop.")
        .def("hasWidth", &mx::Backdrop::hasWidth, "Return true if this backdrop has a width attribute.")
        .def("getWidth", &mx::Backdrop::getWidth, "Return the width attribute of the backdrop.")
        .def("setHeight", &mx::Backdrop::setHeight, "Set the height attribute of the backdrop.")
        .def("hasHeight", &mx::Backdrop::hasHeight, "Return true if this backdrop has a height attribute.")
        .def("getHeight", &mx::Backdrop::getHeight, "Return the height attribute of the backdrop.")
        .def("setContainsElements", &mx::Backdrop::setContainsElements, "Set the vector of elements that this backdrop contains.")
        .def("getContainsElements", &mx::Backdrop::getContainsElements, "Return the vector of elements that this backdrop contains.")
        .def_readonly_static("CATEGORY", &mx::Backdrop::CATEGORY)
        .def_readonly_static("CONTAINS_ATTRIBUTE", &mx::Backdrop::CONTAINS_ATTRIBUTE)
        .def_readonly_static("WIDTH_ATTRIBUTE", &mx::Backdrop::WIDTH_ATTRIBUTE)
        .def_readonly_static("HEIGHT_ATTRIBUTE", &mx::Backdrop::HEIGHT_ATTRIBUTE);
}
