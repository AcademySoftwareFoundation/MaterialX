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
        .def("_addNode", &mx::NodeGraph::addNode,
            py::arg("category"), py::arg("name") = mx::EMPTY_STRING, py::arg("type") = mx::DEFAULT_TYPE_STRING)
        .def("addNodeInstance", &mx::NodeGraph::addNodeInstance,
            py::arg("nodeDef"), py::arg("name") = mx::EMPTY_STRING)
        .def("getNode", &mx::NodeGraph::getNode)
        .def("getNodes", &mx::NodeGraph::getNodes,
            py::arg("category") = mx::EMPTY_STRING)
        .def("removeNode", &mx::NodeGraph::removeNode)
        .def("flattenSubgraphs", &mx::NodeGraph::flattenSubgraphs,
            py::arg("target") = mx::EMPTY_STRING)
        .def("topologicalSort", &mx::NodeGraph::topologicalSort)
        .def("asStringDot", &mx::NodeGraph::asStringDot);

    py::class_<mx::NodeGraph, mx::NodeGraphPtr, mx::GraphElement>(mod, "NodeGraph")
        .def("setNodeDefString", &mx::NodeGraph::setNodeDefString)
        .def("hasNodeDefString", &mx::NodeGraph::hasNodeDefString)
        .def("getNodeDefString", &mx::NodeGraph::getNodeDefString)
        .def("setNodeDef", &mx::NodeGraph::setNodeDef)
        .def("getNodeDef", &mx::NodeGraph::getNodeDef)
        .def_readonly_static("CATEGORY", &mx::NodeGraph::CATEGORY);
}
