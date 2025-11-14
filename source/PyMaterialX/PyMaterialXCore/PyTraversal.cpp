//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Traversal.h>

#include <MaterialXCore/Material.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyTraversal(py::module& mod)
{
    py::class_<mx::Edge>(mod, "Edge", "An edge between two connected Elements, returned during graph traversal.")
        .def("getDownstreamElement", &mx::Edge::getDownstreamElement, "Return the downstream element of the edge.")
        .def("getConnectingElement", &mx::Edge::getConnectingElement, "Return the connecting element of the edge, if any.")
        .def("getUpstreamElement", &mx::Edge::getUpstreamElement, "Return the upstream element of the edge.")
        .def("getName", &mx::Edge::getName, "Return the ColorManagementSystem name.");

    py::class_<mx::TreeIterator>(mod, "TreeIterator", "An iterator object representing the state of a tree traversal.")
        .def("getElement", &mx::TreeIterator::getElement)
        .def("getElementDepth", &mx::TreeIterator::getElementDepth, "Return the element depth of the current traversal, where a single edge between two elements represents a depth of one.")
        .def("setPruneSubtree", &mx::TreeIterator::setPruneSubtree, "Set the prune subtree flag, which controls whether the current subtree is pruned from traversal.\n\nArgs:\n    prune: If set to true, then the current subtree will be pruned.")
        .def("getPruneSubtree", &mx::TreeIterator::getPruneSubtree, "Return the prune subtree flag, which controls whether the current subtree is pruned from traversal.")
        .def("__iter__", [](mx::TreeIterator& it) -> mx::TreeIterator&
            {
                return it.begin(1);
            })
        .def("__next__", [](mx::TreeIterator& it)
            {
                if (++it == it.end())
                    throw py::stop_iteration();
                return *it;
            });

    py::class_<mx::GraphIterator>(mod, "GraphIterator", "An iterator object representing the state of an upstream graph traversal.")
        .def("getDownstreamElement", &mx::GraphIterator::getDownstreamElement, "Return the downstream element of the edge.")
        .def("getConnectingElement", &mx::GraphIterator::getConnectingElement, "Return the connecting element of the edge, if any.")
        .def("getUpstreamElement", &mx::GraphIterator::getUpstreamElement, "Return the upstream element of the edge.")
        .def("getUpstreamIndex", &mx::GraphIterator::getUpstreamIndex, "Return the index of the current edge within the range of upstream edges available to the downstream element.")
        .def("getElementDepth", &mx::GraphIterator::getElementDepth, "Return the element depth of the current traversal, where a single edge between two elements represents a depth of one.")
        .def("getNodeDepth", &mx::GraphIterator::getNodeDepth, "Return the node depth of the current traversal, where a single edge between two nodes represents a depth of one.")
        .def("setPruneSubgraph", &mx::GraphIterator::setPruneSubgraph, "Set the prune subgraph flag, which controls whether the current subgraph is pruned from traversal.\n\nArgs:\n    prune: If set to true, then the current subgraph will be pruned.")
        .def("getPruneSubgraph", &mx::GraphIterator::getPruneSubgraph, "Return the prune subgraph flag, which controls whether the current subgraph is pruned from traversal.")
        .def("__iter__", [](mx::GraphIterator& it) -> mx::GraphIterator&
            {
                return it.begin(1);
            })
        .def("__next__", [](mx::GraphIterator& it)
            {
                if (++it == it.end())
                    throw py::stop_iteration();
                return *it;
            });

    py::class_<mx::InheritanceIterator>(mod, "InheritanceIterator", "An iterator object representing the current state of an inheritance traversal.")
        .def("__iter__", [](mx::InheritanceIterator& it) -> mx::InheritanceIterator&
            {
                return it.begin(1);
            })
        .def("__next__", [](mx::InheritanceIterator& it)
            {
                if (++it == it.end())
                    throw py::stop_iteration();
                return *it;
            });

    py::register_exception<mx::ExceptionFoundCycle>(mod, "ExceptionFoundCycle");
}
