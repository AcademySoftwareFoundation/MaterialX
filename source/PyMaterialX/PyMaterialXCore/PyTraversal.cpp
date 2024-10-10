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
    py::class_<mx::Edge>(mod, "Edge")
        .def("getDownstreamElement", &mx::Edge::getDownstreamElement)
        .def("getConnectingElement", &mx::Edge::getConnectingElement)
        .def("getUpstreamElement", &mx::Edge::getUpstreamElement)
        .def("getName", &mx::Edge::getName);
    mod.attr("Edge").doc() = R"docstring(
    An edge between two connected `Element` objects, returned during graph traversal.

    A valid `Edge` consists of a downstream element, an upstream element, and
    optionally a connecting element that binds them.  As an example, the edge
    between two `Node` elements will contain a connecting element for the `Input`
    of the downstream `Node`.

    :see: `Element.traverseGraph()`
    :see: https://materialx.org/docs/api/class_edge.html)docstring";

    py::class_<mx::TreeIterator>(mod, "TreeIterator")
        .def("getElement", &mx::TreeIterator::getElement)
        .def("getElementDepth", &mx::TreeIterator::getElementDepth)
        .def("setPruneSubtree", &mx::TreeIterator::setPruneSubtree)
        .def("getPruneSubtree", &mx::TreeIterator::getPruneSubtree)
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
    mod.attr("TreeIterator").doc() = R"docstring(
    An iterator representing the state of a tree traversal.

    :see: `Element.traverseTree()`
    :see: https://materialx.org/docs/api/class_tree_iterator.html)docstring";

    py::class_<mx::GraphIterator>(mod, "GraphIterator")
        .def("getDownstreamElement", &mx::GraphIterator::getDownstreamElement)
        .def("getConnectingElement", &mx::GraphIterator::getConnectingElement)
        .def("getUpstreamElement", &mx::GraphIterator::getUpstreamElement)
        .def("getUpstreamIndex", &mx::GraphIterator::getUpstreamIndex)
        .def("getElementDepth", &mx::GraphIterator::getElementDepth)
        .def("getNodeDepth", &mx::GraphIterator::getNodeDepth)
        .def("setPruneSubgraph", &mx::GraphIterator::setPruneSubgraph)
        .def("getPruneSubgraph", &mx::GraphIterator::getPruneSubgraph)
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
    mod.attr("GraphIterator").doc() = R"docstring(
    An iterator representing the state of an upstream graph traversal.

    :see: `Element.traverseGraph()`
    :see: https://materialx.org/docs/api/class_graph_iterator.html)docstring";

    py::class_<mx::InheritanceIterator>(mod, "InheritanceIterator")
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
    mod.attr("InheritanceIterator").doc() = R"docstring(
    An iterator representing the current state of an inheritance traversal.

    :see: `Element.traverseInheritance()`
    :see: https://materialx.org/docs/api/class_inheritance_iterator.html)docstring";

    py::register_exception<mx::ExceptionFoundCycle>(mod, "ExceptionFoundCycle");
    mod.attr("ExceptionFoundCycle").doc() = R"docstring(
    A type of exception that is raised when a traversal call encounters a cycle.

    :see: `Element.traverseGraph()`
    :see: `Element.traverseInheritance()`)docstring";
}
