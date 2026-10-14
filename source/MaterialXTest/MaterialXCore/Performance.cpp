//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#ifdef CATCH_CONFIG_ENABLE_BENCHMARKING

    #include <MaterialXCore/Definition.h>
    #include <MaterialXCore/Document.h>
    #include <MaterialXCore/Node.h>

namespace mx = MaterialX;

// Build a document that scales the two axes feeding NodeGraph::getNodeDef
// from PortElement::validate:
//   * numImplStubs     - document-root <implementation> elements; sizes the
//                        Document::Cache nodegraph -> implementation map.
//   * numNodegraphRefs - inputs that resolve through a nodegraph; each one
//                        performs a cache lookup.
static mx::DocumentPtr buildScalingDocument(size_t numImplStubs, size_t numNodegraphRefs)
{
    mx::DocumentPtr doc = mx::createDocument();

    // A nodegraph with one internal node and one output, so that
    // PortElement::getConnectedNode() resolves transitively through the
    // output (see Input::getConnectedNode in Interface.cpp).
    mx::NodeGraphPtr ng = doc->addNodeGraph("ng");
    mx::NodePtr inner = ng->addNode("constant", "inner", "color3");
    mx::OutputPtr out = ng->addOutput("out", "color3");
    out->setNodeName(inner->getName());

    // A host node parenting many inputs that reference the nodegraph.
    mx::NodePtr host = doc->addNode("surface", "host", "surfaceshader");
    for (size_t i = 0; i < numNodegraphRefs; ++i)
    {
        mx::InputPtr in = host->addInput("in" + std::to_string(i), "color3");
        in->setNodeGraphString("ng");
        in->setOutputString("out");
    }

    for (size_t i = 0; i < numImplStubs; ++i)
    {
        mx::ImplementationPtr impl = doc->addImplementation("i" + std::to_string(i));
        impl->setNodeDefString("n" + std::to_string(i));
    }

    return doc;
}

TEST_CASE("NodeGraph getNodeDef performance", "[node][performance]")
{
    BENCHMARK("validate, stubs=200 refs=100")
    {
        mx::DocumentPtr doc = buildScalingDocument(200, 100);
        return doc->validate();
    };

    BENCHMARK("validate, stubs=1000 refs=100")
    {
        mx::DocumentPtr doc = buildScalingDocument(1000, 100);
        return doc->validate();
    };

    BENCHMARK("validate, stubs=10000 refs=100")
    {
        mx::DocumentPtr doc = buildScalingDocument(10000, 100);
        return doc->validate();
    };
}

#endif // CATCH_CONFIG_ENABLE_BENCHMARKING
