//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Node.h>

#include <PyBind11/stl.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyNode(py::module& mod)
{
    py::class_<mx::Node, mx::NodePtr, mx::InterfaceElement>(mod, "Node")
        .def("setConnectedNode", &mx::Node::setConnectedNode)
        .def("getConnectedNode", &mx::Node::getConnectedNode)
        .def("setConnectedNodeName", &mx::Node::setConnectedNodeName)
        .def("getConnectedNodeName", &mx::Node::getConnectedNodeName)
        .def("getReferencedNodeDef", &mx::Node::getReferencedNodeDef)
        .def("getImplementation", &mx::Node::getImplementation)
        .def("getDownstreamPorts", &mx::Node::getDownstreamPorts)
        .def_readonly_static("CATEGORY", &mx::Node::CATEGORY);

    py::class_<mx::NodeGraph, mx::NodeGraphPtr, mx::Element>(mod, "NodeGraph")
        .def("setNodeDef", &mx::NodeGraph::setNodeDef)
        .def("hasNodeDef", &mx::NodeGraph::hasNodeDef)
        .def("getNodeDef", &mx::NodeGraph::getNodeDef)
        .def("_addNode", &mx::NodeGraph::addNode,
            py::arg("category"), py::arg("name") = mx::EMPTY_STRING, py::arg("type") = mx::DEFAULT_TYPE_STRING)
        .def("getNode", &mx::NodeGraph::getNode)
        .def("getNodes", &mx::NodeGraph::getNodes)
        .def("removeNode", &mx::NodeGraph::removeNode)
        .def("addOutput", &mx::NodeGraph::addOutput,
            py::arg("name") = mx::EMPTY_STRING, py::arg("type") = mx::DEFAULT_TYPE_STRING)
        .def("getOutput", &mx::NodeGraph::getOutput)
        .def("getOutputs", &mx::NodeGraph::getOutputs)
        .def("removeOutput", &mx::NodeGraph::removeOutput)
        .def("flattenSubgraphs", &mx::NodeGraph::flattenSubgraphs,
            py::arg("target") = mx::EMPTY_STRING)
        .def("topologicalSort", &mx::NodeGraph::topologicalSort)
        .def_readonly_static("CATEGORY", &mx::NodeGraph::CATEGORY);
}
