//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Node.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyNode(py::module& mod)
{
    py::class_<mx::Node, mx::NodePtr, mx::InterfaceElement>(mod, "Node")
        .def("setConnectedNode", &mx::Node::setConnectedNode)
        .def("getConnectedNode", &mx::Node::getConnectedNode)
        .def("setConnectedNodeName", &mx::Node::setConnectedNodeName)
        .def("getConnectedNodeName", &mx::Node::getConnectedNodeName)
        .def("getNodeDef", &mx::Node::getNodeDef,
            py::arg("target") = mx::EMPTY_STRING)
        .def("getImplementation", &mx::Node::getImplementation,
            py::arg("target") = mx::EMPTY_STRING,
            py::arg("language") = mx::EMPTY_STRING)
        .def("getDownstreamPorts", &mx::Node::getDownstreamPorts)
        .def_readonly_static("CATEGORY", &mx::Node::CATEGORY);

    py::class_<mx::GraphElement, mx::GraphElementPtr, mx::InterfaceElement>(mod, "GraphElement")
        .def("_addNode", &mx::GraphElement::addNode,
            py::arg("category"), py::arg("name") = mx::EMPTY_STRING, py::arg("type") = mx::DEFAULT_TYPE_STRING)
        .def("addNodeInstance", &mx::GraphElement::addNodeInstance,
            py::arg("nodeDef"), py::arg("name") = mx::EMPTY_STRING)
        .def("getNode", &mx::GraphElement::getNode)
        .def("getNodes", &mx::GraphElement::getNodes,
            py::arg("category") = mx::EMPTY_STRING)
        .def("removeNode", &mx::GraphElement::removeNode)
        .def("addBackdrop", &mx::GraphElement::addBackdrop,
            py::arg("name") = mx::EMPTY_STRING)
        .def("getBackdrop", &mx::GraphElement::getBackdrop)
        .def("getBackdrops", &mx::GraphElement::getBackdrops)
        .def("removeBackdrop", &mx::GraphElement::removeBackdrop)
        .def("flattenSubgraphs", &mx::GraphElement::flattenSubgraphs,
            py::arg("target") = mx::EMPTY_STRING)
        .def("topologicalSort", &mx::GraphElement::topologicalSort)
        .def("asStringDot", &mx::GraphElement::asStringDot);

    py::class_<mx::NodeGraph, mx::NodeGraphPtr, mx::GraphElement>(mod, "NodeGraph")
        .def("setNodeDef", &mx::NodeGraph::setNodeDef)
        .def("getNodeDef", &mx::NodeGraph::getNodeDef)
        .def_readonly_static("CATEGORY", &mx::NodeGraph::CATEGORY);

    py::class_<mx::Backdrop, mx::BackdropPtr, mx::Element>(mod, "Backdrop")
        .def("getContains", &mx::Backdrop::getContains)
        .def("setContains", &mx::Backdrop::setContains)
        .def("getWidth", &mx::Backdrop::getWidth)
        .def("setWidth", &mx::Backdrop::setWidth)
        .def("getHeight", &mx::Backdrop::getHeight)
        .def("setHeight", &mx::Backdrop::setHeight)
        .def_readonly_static("CATEGORY", &mx::Backdrop::CATEGORY)
        .def_readonly_static("CONTAINS_ATTRIBUTE", &mx::Backdrop::CONTAINS_ATTRIBUTE)
        .def_readonly_static("WIDTH_ATTRIBUTE", &mx::Backdrop::WIDTH_ATTRIBUTE)
        .def_readonly_static("HEIGHT_ATTRIBUTE", &mx::Backdrop::HEIGHT_ATTRIBUTE);
}
