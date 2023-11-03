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

        .def("getDownstreamElement", &mx::Edge::getDownstreamElement,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the downstream element of the edge.
)docstring"))

        .def("getConnectingElement", &mx::Edge::getConnectingElement,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the connecting element of the edge, if any.
)docstring"))

        .def("getUpstreamElement", &mx::Edge::getUpstreamElement,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the upstream element of the edge.
)docstring"))

        .def("getName", &mx::Edge::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the name of this edge, if any.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing an edge between two connected `Element` objects,
    returned during graph traversal.

    A valid `Edge` consists of a downstream element, an upstream element, and
    optionally a connecting element that binds them.  As an example, the edge
    between two `Node` elements will contain a connecting element for the `Input`
    of the downstream `Node`.

    :see: `Element.traverseGraph()`
    :see: https://materialx.org/docs/api/class_edge.html
)docstring");

    py::class_<mx::TreeIterator>(mod, "TreeIterator")

        .def("getElement", &mx::TreeIterator::getElement,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the current element in the traversal.
)docstring"))

        .def("getElementDepth", &mx::TreeIterator::getElementDepth,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element depth of the current traversal, where the starting
    element represents a depth of zero.
)docstring"))

        .def("setPruneSubtree", &mx::TreeIterator::setPruneSubtree,
             py::arg("prune"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the prune subtree flag, which controls whether the current subtree
    is pruned from traversal.

    :type prune: bool
    :param prune: If set to `True`, then the current subtree will be pruned.
)docstring"))

        .def("getPruneSubtree", &mx::TreeIterator::getPruneSubtree,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the prune subtree flag, which controls whether the current subtree
    is pruned from traversal.
)docstring"))

        .def("__iter__", [](mx::TreeIterator& it) -> mx::TreeIterator&
            {
                return it.begin(1);
            })

        .def("__next__", [](mx::TreeIterator& it)
            {
                if (++it == it.end())
                    throw py::stop_iteration();
                return *it;
            })

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing an iterator representing the state of a tree traversal.

    :see: `Element.traverseTree()`
    :see: https://materialx.org/docs/api/class_tree_iterator.html
)docstring");

    py::class_<mx::GraphIterator>(mod, "GraphIterator")

        .def("getDownstreamElement", &mx::GraphIterator::getDownstreamElement,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the downstream element of the current edge.
)docstring"))

        .def("getConnectingElement", &mx::GraphIterator::getConnectingElement,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the connecting element, if any, of the current edge.
)docstring"))

        .def("getUpstreamElement", &mx::GraphIterator::getUpstreamElement,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the upstream element of the current edge.
)docstring"))

        .def("getUpstreamIndex", &mx::GraphIterator::getUpstreamIndex,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the index of the current edge within the range of upstream edges
    available to the downstream element.
)docstring"))

        .def("getElementDepth", &mx::GraphIterator::getElementDepth,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the element depth of the current traversal, where a single edge
    between two elements represents a depth of one.
)docstring"))

        .def("getNodeDepth", &mx::GraphIterator::getNodeDepth,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the node depth of the current traversal, where a single edge
    between two nodes represents a depth of one.
)docstring"))

        .def("setPruneSubgraph", &mx::GraphIterator::setPruneSubgraph,
             py::arg("prune"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the prune subgraph flag, which controls whether the current subgraph
    is pruned from traversal.

    :type prune: bool
    :param prune: If set to `True`, then the current subgraph will be pruned.
)docstring"))

        .def("getPruneSubgraph", &mx::GraphIterator::getPruneSubgraph,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the prune subgraph flag, which controls whether the current
    subgraph is pruned from traversal.
)docstring"))

        .def("__iter__", [](mx::GraphIterator& it) -> mx::GraphIterator&
            {
                return it.begin(1);
            })

        .def("__next__", [](mx::GraphIterator& it)
            {
                if (++it == it.end())
                    throw py::stop_iteration();
                return *it;
            })

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing an iterator representing the state of an upstream graph traversal.

    :see: `Element.traverseGraph()`
    :see: https://materialx.org/docs/api/class_graph_iterator.html
)docstring");

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
            })

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing an iterator representing the current state of an inheritance traversal.

    :see: `Element.traverseInheritance()`
    :see: https://materialx.org/docs/api/class_inheritance_iterator.html
)docstring");

    py::register_exception<mx::ExceptionFoundCycle>(mod, "ExceptionFoundCycle")

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A type of exception that is raised when a traversal call encounters a cycle.

    :see: `Element.traverseGraph()`
    :see: `Element.traverseInheritance()`
)docstring");
}
