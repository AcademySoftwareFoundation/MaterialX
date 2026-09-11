//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/XmlIo.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/ShaderGraphRefactor.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>

#ifdef MATERIALX_BUILD_GEN_GLSL
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
#include <MaterialXGenOsl/OslShaderGenerator.h>
#ifdef MATERIALX_BUILD_OSOS
#include <MaterialXGenOsl/OslNetworkShaderGenerator.h>
#endif
#endif
#ifdef MATERIALX_BUILD_GEN_MSL
#include <MaterialXGenMsl/MslShaderGenerator.h>
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
#include <MaterialXGenMdl/MdlShaderGenerator.h>
#endif

#include <set>
#include <string>
#include <vector>

namespace mx = MaterialX;

//
// Test harness
//

// A document plus the graph built from one of its elements. The document must outlive
// the graph, so both are held together.
struct TestGraph
{
    mx::DocumentPtr doc;
    mx::ShaderGraphPtr graph;
};

TestGraph createGraphFromFile(mx::DocumentPtr libraries, const std::string& relativePath,
                              const std::string& elementPath, mx::GenContext& context)
{
    TestGraph result;
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath file = searchPath.find(relativePath);
    REQUIRE(file.exists());

    result.doc = mx::createDocument();
    mx::readFromXmlFile(result.doc, file);
    result.doc->setDataLibrary(libraries);

    mx::ElementPtr element = result.doc->getDescendant(elementPath);
    REQUIRE(element != nullptr);

    result.graph = mx::ShaderGraph::create(nullptr, "test", element, context);
    REQUIRE(result.graph != nullptr);
    return result;
}

TestGraph createGraphFromString(mx::DocumentPtr libraries, const std::string& xml,
                                const std::string& elementPath, mx::GenContext& context)
{
    TestGraph result;
    result.doc = mx::createDocument();
    mx::readFromXmlString(result.doc, xml);
    result.doc->setDataLibrary(libraries);

    mx::ElementPtr element = result.doc->getDescendant(elementPath);
    REQUIRE(element != nullptr);

    result.graph = mx::ShaderGraph::create(nullptr, "test", element, context);
    REQUIRE(result.graph != nullptr);
    return result;
}

// Return the graph implemented by a node, or nullptr if the node is not a compound node.
mx::ShaderGraph* compoundGraphOf(const mx::ShaderNode* node)
{
    return node->getImplementation().getGraph();
}

size_t countCompoundNodes(const mx::ShaderGraph& graph)
{
    size_t count = 0;
    for (const mx::ShaderNode* node : graph.getNodes())
    {
        if (compoundGraphOf(node))
        {
            ++count;
        }
    }
    return count;
}

// Verify the invariants that the node map and node order must always satisfy together.
// The node map holds the only owning reference to each node while the node order holds
// raw pointers, so any divergence between them is a use-after-free waiting to happen.
void checkNodeStorageConsistent(mx::ShaderGraph& graph)
{
    std::set<const mx::ShaderNode*> seen;
    std::set<std::string> seenNames;
    for (mx::ShaderNode* node : graph.getNodes())
    {
        REQUIRE(node != nullptr);

        // Every node in the order must be the same object the map owns under its id.
        CHECK(graph.getNode(node->getUniqueId()) == node);

        // No node may appear in the order twice, and unique ids must be unique.
        CHECK(seen.insert(node).second);

        // Node names are visible to backends and must not alias.
        CHECK(seenNames.insert(node->getName()).second);
    }
}

// Walk upstream from every output socket, checking that the graph is fully connected and
// contains no compound nodes.
void checkFullyFlattened(mx::ShaderGraph& graph)
{
    CHECK(countCompoundNodes(graph) == 0);
    checkNodeStorageConsistent(graph);

    for (mx::ShaderGraphOutputSocket* outputSocket : graph.getOutputSockets())
    {
        mx::ShaderOutput* upstream = outputSocket->getConnection();
        if (!upstream || upstream->getNode() == &graph)
        {
            continue;
        }
        // The output socket must resolve to a node that is still owned by the graph.
        CHECK(graph.getNode(upstream->getNode()->getUniqueId()) == upstream->getNode());
    }

    // Every connection in the graph must point at a node the graph still owns.
    for (mx::ShaderNode* node : graph.getNodes())
    {
        for (mx::ShaderInput* input : node->getInputs())
        {
            mx::ShaderOutput* connection = input->getConnection();
            if (!connection || connection->getNode() == &graph)
            {
                continue;
            }
            CHECK(graph.getNode(connection->getNode()->getUniqueId()) == connection->getNode());
        }
    }
}

void testFlattenNodeDefImplementedByNodeGraph(mx::DocumentPtr libraries, mx::GenContext& context)
{
    // ND_layered is implemented by the NG_layered nodegraph, so the shader_layered
    // instance below resolves to a compound node.
    TestGraph test = createGraphFromFile(
        libraries, "resources/Materials/TestSuite/stdlib/definition/definition_using_definitions.mtlx",
        "shader_layered", context);

    REQUIRE(countCompoundNodes(*test.graph) > 0);

    mx::CompoundFlatteningRefactor pass;
    const size_t numExpanded = pass.execute(*test.graph, context);

    CHECK(numExpanded > 0);
    checkFullyFlattened(*test.graph);
}

void testFlattenNestedNodeGraphs(mx::DocumentPtr libraries, mx::GenContext& context)
{

    // outer -> middle -> inner, three levels deep. Each level is its own nodedef
    // implemented by a nodegraph, since only a nodedef backed nodegraph becomes a
    // compound node: a plain nodegraph is resolved through during graph construction
    // and its interior nodes land in the parent graph before this pass ever runs.
    //
    // Every level also carries a multiply whose in2 value identifies it, so the test
    // can confirm that all three were inlined rather than just the outermost.
    const std::string xml = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_inner" node="inner">
    <input name="in" type="color3" value="0, 0, 0" />
    <output name="out" type="color3" />
  </nodedef>
  <nodegraph name="NG_inner" nodedef="ND_inner">
    <multiply name="mul" type="color3">
      <input name="in1" type="color3" interfacename="in" />
      <input name="in2" type="color3" value="1, 1, 1" />
    </multiply>
    <output name="out" type="color3" nodename="mul" />
  </nodegraph>

  <nodedef name="ND_middle" node="middle">
    <input name="in" type="color3" value="0, 0, 0" />
    <output name="out" type="color3" />
  </nodedef>
  <nodegraph name="NG_middle" nodedef="ND_middle">
    <inner name="nested" type="color3">
      <input name="in" type="color3" interfacename="in" />
    </inner>
    <multiply name="mul" type="color3">
      <input name="in1" type="color3" nodename="nested" />
      <input name="in2" type="color3" value="2, 2, 2" />
    </multiply>
    <output name="out" type="color3" nodename="mul" />
  </nodegraph>

  <nodedef name="ND_outer" node="outer">
    <input name="in" type="color3" value="0, 0, 0" />
    <output name="out" type="color3" />
  </nodedef>
  <nodegraph name="NG_outer" nodedef="ND_outer">
    <middle name="nested" type="color3">
      <input name="in" type="color3" interfacename="in" />
    </middle>
    <multiply name="mul" type="color3">
      <input name="in1" type="color3" nodename="nested" />
      <input name="in2" type="color3" value="3, 3, 3" />
    </multiply>
    <output name="out" type="color3" nodename="mul" />
  </nodegraph>

  <outer name="inst" type="color3">
    <input name="in" type="color3" value="0.5, 0.5, 0.5" />
  </outer>
  <output name="root" type="color3" nodename="inst" />
</materialx>
)";

    TestGraph test = createGraphFromString(libraries, xml, "root", context);

    // Only the outermost compound is visible at the start; the two nested ones are
    // still hidden inside its graph.
    const size_t numCompoundBefore = countCompoundNodes(*test.graph);
    REQUIRE(numCompoundBefore == 1);

    mx::CompoundFlatteningRefactor pass;
    const size_t numExpanded = pass.execute(*test.graph, context);

    // Nested compound nodes are only discovered as their parents are expanded, so
    // more nodes must have been expanded than were visible at the start. A single
    // execute() call must reach all three levels.
    CHECK(numExpanded == 3);
    checkFullyFlattened(*test.graph);

    // Each level contributed its own multiply, so all three values must be present
    // in the flattened graph exactly once.
    std::set<std::string> mulValues;
    for (mx::ShaderNode* node : test.graph->getNodes())
    {
        mx::ShaderInput* in2 = node->getInput("in2");
        if (in2 && in2->getValue())
        {
            mulValues.insert(in2->getValue()->getValueString());
        }
    }
    CHECK(mulValues.count("1, 1, 1") == 1);
    CHECK(mulValues.count("2, 2, 2") == 1);
    CHECK(mulValues.count("3, 3, 3") == 1);

    // A second run has nothing left to do.
    CHECK(pass.execute(*test.graph, context) == 0);
}

void testFlattenCascadedNodeGraphs(mx::DocumentPtr libraries, mx::GenContext& context)
{

    // The same nodedef backed nodegraph is instantiated more than once, so the same
    // interior nodes are copied more than once in a single run. A CompoundNode holds
    // one ShaderGraph for its nodedef and every instance shares it, so the interior
    // nodes being copied are literally the same objects with the same names and unique
    // ids each time. Each instance must still end up with its own live copies carrying
    // its own interface values.
    //
    // "pair" instantiates "stage" twice, and "pair" is itself instantiated twice, so
    // NG_pair is expanded twice and NG_stage four times. Each of the four leaves gets a
    // different interface value, so the copies can be told apart.
    const std::string xml = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_stage" node="stage">
    <input name="in" type="color3" value="0, 0, 0" />
    <output name="out" type="color3" />
  </nodedef>
  <nodegraph name="NG_stage" nodedef="ND_stage">
    <multiply name="mul" type="color3">
      <input name="in1" type="color3" interfacename="in" />
      <input name="in2" type="color3" value="0.5, 0.5, 0.5" />
    </multiply>
    <output name="out" type="color3" nodename="mul" />
  </nodegraph>

  <nodedef name="ND_pair" node="pair">
    <input name="inA" type="color3" value="0, 0, 0" />
    <input name="inB" type="color3" value="0, 0, 0" />
    <output name="out" type="color3" />
  </nodedef>
  <nodegraph name="NG_pair" nodedef="ND_pair">
    <stage name="s1" type="color3">
      <input name="in" type="color3" interfacename="inA" />
    </stage>
    <stage name="s2" type="color3">
      <input name="in" type="color3" interfacename="inB" />
    </stage>
    <add name="sum" type="color3">
      <input name="in1" type="color3" nodename="s1" />
      <input name="in2" type="color3" nodename="s2" />
    </add>
    <output name="out" type="color3" nodename="sum" />
  </nodegraph>

  <pair name="pairA" type="color3">
    <input name="inA" type="color3" value="1, 0, 0" />
    <input name="inB" type="color3" value="0, 1, 0" />
  </pair>
  <pair name="pairB" type="color3">
    <input name="inA" type="color3" value="0, 0, 1" />
    <input name="inB" type="color3" value="1, 1, 0" />
  </pair>
  <add name="rootNode" type="color3">
    <input name="in1" type="color3" nodename="pairA" />
    <input name="in2" type="color3" nodename="pairB" />
  </add>
  <output name="root" type="color3" nodename="rootNode" />
</materialx>
)";

    // A reduced interface keeps the unbound interface values on the compound nodes'
    // inputs rather than publishing them as graph input sockets, so each leaf copy
    // ends up carrying the value that identifies it below.
    const mx::ShaderInterfaceType oldInterfaceType = context.getOptions().shaderInterfaceType;
    context.getOptions().shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;

    TestGraph test = createGraphFromString(libraries, xml, "root", context);

    // Both nodedefs share one graph per definition, so the two pair instances expand
    // the same interior nodes.
    REQUIRE(countCompoundNodes(*test.graph) == 2);
    std::vector<mx::ShaderNode*> pairs;
    for (mx::ShaderNode* node : test.graph->getNodes())
    {
        if (compoundGraphOf(node))
        {
            pairs.push_back(node);
        }
    }
    REQUIRE(pairs.size() == 2);
    CHECK(compoundGraphOf(pairs[0]) == compoundGraphOf(pairs[1]));

    mx::CompoundFlatteningRefactor pass;

    // Two pairs plus the four stages they contain.
    CHECK(pass.execute(*test.graph, context) == 6);

    // checkFullyFlattened asserts that unique ids and names are distinct and that
    // every connection still points at a live, graph-owned node.
    checkFullyFlattened(*test.graph);

    // Each of the four leaf copies must be its own node carrying its own value, rather
    // than four consumers sharing one copy of the interior multiply.
    std::set<std::string> leafValues;
    for (mx::ShaderNode* node : test.graph->getNodes())
    {
        mx::ShaderInput* in1 = node->getInput("in1");
        if (in1 && !in1->getConnection() && in1->getValue())
        {
            leafValues.insert(in1->getValue()->getValueString());
        }
    }
    CHECK(leafValues.count("1, 0, 0") == 1);
    CHECK(leafValues.count("0, 1, 0") == 1);
    CHECK(leafValues.count("0, 0, 1") == 1);
    CHECK(leafValues.count("1, 1, 0") == 1);

    context.getOptions().shaderInterfaceType = oldInterfaceType;
}

void testFlattenCollidingCopyNames(mx::DocumentPtr libraries, mx::GenContext& context)
{

    // Regression test for keying the copies by unique id rather than by name.
    //
    // Copies are named "<parent>_<child>", so a name derived key is ambiguous about
    // where the boundary between the two halves falls. Here NG_ci_outer holds a
    // compound node "nested" alongside a sibling "nested_mul", and NG_ci_inner holds
    // a "mul":
    //
    //   expanding "inst"        copies "nested"     -> key "inst_nested"
    //                           copies "nested_mul" -> key "inst_nested_mul"
    //   expanding "inst_nested" copies "mul"        -> key "inst_nested" + "_" + "mul"
    //
    // The last two keys are the same string, so under name keying the second copy
    // would displace the first in the node map while the node order still held a raw
    // pointer to it. Unique ids are element name paths, which do not concatenate
    // ambiguously, so the two stay apart.
    //
    // The definitions need names of their own rather than reusing another fixture's:
    // a CompoundNode is cached in the GenContext against its definition, so a shared
    // name would hand this graph the nodegraph belonging to whichever test ran first.
    const std::string xml = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_ci_inner" node="ci_inner">
    <input name="in" type="color3" value="0, 0, 0" />
    <output name="out" type="color3" />
  </nodedef>
  <nodegraph name="NG_ci_inner" nodedef="ND_ci_inner">
    <multiply name="mul" type="color3">
      <input name="in1" type="color3" interfacename="in" />
      <input name="in2" type="color3" value="1, 0, 0" />
    </multiply>
    <output name="out" type="color3" nodename="mul" />
  </nodegraph>

  <nodedef name="ND_ci_outer" node="ci_outer">
    <input name="in" type="color3" value="0, 0, 0" />
    <output name="out" type="color3" />
  </nodedef>
  <nodegraph name="NG_ci_outer" nodedef="ND_ci_outer">
    <ci_inner name="nested" type="color3">
      <input name="in" type="color3" interfacename="in" />
    </ci_inner>
    <multiply name="nested_mul" type="color3">
      <input name="in1" type="color3" nodename="nested" />
      <input name="in2" type="color3" value="0, 1, 0" />
    </multiply>
    <output name="out" type="color3" nodename="nested_mul" />
  </nodegraph>

  <ci_outer name="inst" type="color3">
    <input name="in" type="color3" value="0, 0, 1" />
  </ci_outer>
  <output name="root" type="color3" nodename="inst" />
</materialx>
)";

    const mx::ShaderInterfaceType oldInterfaceType = context.getOptions().shaderInterfaceType;
    context.getOptions().shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;

    TestGraph test = createGraphFromString(libraries, xml, "root", context);

    REQUIRE(countCompoundNodes(*test.graph) == 1);

    mx::CompoundFlatteningRefactor pass;
    CHECK(pass.execute(*test.graph, context) == 2);
    checkFullyFlattened(*test.graph);

    // Both multiplies must have survived as separate nodes, identifiable by the values
    // that distinguish them. Under name keying one of the two is lost.
    std::set<std::string> mulValues;
    for (mx::ShaderNode* node : test.graph->getNodes())
    {
        mx::ShaderInput* in2 = node->getInput("in2");
        if (in2 && in2->getValue())
        {
            mulValues.insert(in2->getValue()->getValueString());
        }
    }
    CHECK(mulValues.count("1, 0, 0") == 1);
    CHECK(mulValues.count("0, 1, 0") == 1);

    context.getOptions().shaderInterfaceType = oldInterfaceType;
}

void testFlattenMultiOutputNodeGraph(mx::DocumentPtr libraries, mx::GenContext& context)
{

    // A self-contained definition rather than a stdlib one, so the test asserts on
    // exactly two interior nodes whose identity it controls. Only nodedef backed
    // nodegraphs become compound nodes; a plain nodegraph is resolved through during
    // graph construction and never reaches this pass.
    //
    // "outa" and "outb" are fed by different interior nodes, distinguishable by their
    // in2 values, so each consumer must end up on its own copy.
    const std::string xml = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_twoout" node="twoout">
    <input name="in" type="float" value="1" />
    <output name="outa" type="float" />
    <output name="outb" type="float" />
  </nodedef>
  <nodegraph name="NG_twoout" nodedef="ND_twoout">
    <add name="a" type="float">
      <input name="in1" type="float" interfacename="in" />
      <input name="in2" type="float" value="10" />
    </add>
    <add name="b" type="float">
      <input name="in1" type="float" interfacename="in" />
      <input name="in2" type="float" value="20" />
    </add>
    <output name="outa" type="float" nodename="a" />
    <output name="outb" type="float" nodename="b" />
  </nodegraph>
  <twoout name="inst" type="multioutput" />
  <multiply name="consumerA" type="float">
    <input name="in1" type="float" nodename="inst" output="outa" />
    <input name="in2" type="float" value="1" />
  </multiply>
  <multiply name="consumerB" type="float">
    <input name="in1" type="float" nodename="inst" output="outb" />
    <input name="in2" type="float" value="1" />
  </multiply>
  <add name="rootNode" type="float">
    <input name="in1" type="float" nodename="consumerA" />
    <input name="in2" type="float" nodename="consumerB" />
  </add>
  <output name="root" type="float" nodename="rootNode" />
</materialx>
)";

    TestGraph test = createGraphFromString(libraries, xml, "root", context);

    REQUIRE(countCompoundNodes(*test.graph) > 0);

    mx::CompoundFlatteningRefactor pass;
    CHECK(pass.execute(*test.graph, context) > 0);
    checkFullyFlattened(*test.graph);

    auto upstreamOf = [&test](const std::string& consumerName) -> mx::ShaderNode*
    {
        for (mx::ShaderNode* node : test.graph->getNodes())
        {
            if (node->getName() == consumerName)
            {
                mx::ShaderInput* in1 = node->getInput("in1");
                return (in1 && in1->getConnection()) ? in1->getConnection()->getNode() : nullptr;
            }
        }
        return nullptr;
    };

    mx::ShaderNode* sourceA = upstreamOf("consumerA");
    mx::ShaderNode* sourceB = upstreamOf("consumerB");
    REQUIRE(sourceA != nullptr);
    REQUIRE(sourceB != nullptr);

    // Each output socket must resolve to its own interior node, not to a shared one.
    CHECK(sourceA != sourceB);

    // Identify the copies by the interior value that distinguishes them.
    REQUIRE(sourceA->getInput("in2") != nullptr);
    REQUIRE(sourceA->getInput("in2")->getValue() != nullptr);
    CHECK(sourceA->getInput("in2")->getValue()->getValueString() == "10");

    REQUIRE(sourceB->getInput("in2") != nullptr);
    REQUIRE(sourceB->getInput("in2")->getValue() != nullptr);
    CHECK(sourceB->getInput("in2")->getValue()->getValueString() == "20");
}

void testCompoundFlattening(mx::DocumentPtr libraries, mx::GenContext& context)
{
    testFlattenNodeDefImplementedByNodeGraph(libraries, context);
    testFlattenNestedNodeGraphs(libraries, context);
    testFlattenCascadedNodeGraphs(libraries, context);
    testFlattenCollidingCopyNames(libraries, context);
    testFlattenMultiOutputNodeGraph(libraries, context);
}

TEST_CASE("GenShader: Flatten Compound Nodes", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlattening(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlattening(libraries, context);
    }
#ifdef MATERIALX_BUILD_OSOS
    {
        mx::GenContext context(mx::OslNetworkShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlattening(libraries, context);
    }
#endif
#endif
#ifdef MATERIALX_BUILD_GEN_MSL
    {
        mx::GenContext context(mx::MslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlattening(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlattening(libraries, context);
    }
#endif
#if !defined(MATERIALX_BUILD_GEN_GLSL) && !defined(MATERIALX_BUILD_GEN_OSL) && \
    !defined(MATERIALX_BUILD_GEN_MSL) && !defined(MATERIALX_BUILD_GEN_MDL)
    // Guard against this test reporting success when it covered nothing.
    FAIL("No shader generator was built, so nothing was tested.");
#endif
}

void testFlattenPassThroughNodeGraph(mx::DocumentPtr libraries, mx::GenContext& context)
{

    // A pass-through is an output socket fed by an input socket rather than by an
    // interior node, so the compound graph does nothing but forward a value. It arises
    // from node elision: the constant nodes below sit between an interface input and an
    // output, and NodeElisionRefactor bypasses them while the compound graph is being
    // finalized, leaving the output socket wired straight to the input socket.
    //
    // "passval" carries a plain value while "passconn" is driven by a node outside the
    // compound, so both halves of the splice are covered. "outnode" is a normal output
    // fed by a surviving interior node, as a control.
    const std::string xml = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_pt" node="pt">
    <input name="passval" type="color3" value="0.1, 0.2, 0.3" />
    <input name="passconn" type="color3" value="0, 0, 0" />
    <output name="outval" type="color3" />
    <output name="outconn" type="color3" />
    <output name="outnode" type="color3" />
  </nodedef>
  <nodegraph name="NG_pt" nodedef="ND_pt">
    <constant name="passvalConstant" type="color3">
      <input name="value" type="color3" interfacename="passval" />
    </constant>
    <constant name="passconnConstant" type="color3">
      <input name="value" type="color3" interfacename="passconn" />
    </constant>
    <multiply name="interior" type="color3">
      <input name="in1" type="color3" interfacename="passval" />
      <input name="in2" type="color3" value="2, 2, 2" />
    </multiply>
    <output name="outval" type="color3" nodename="passvalConstant" />
    <output name="outconn" type="color3" nodename="passconnConstant" />
    <output name="outnode" type="color3" nodename="interior" />
  </nodegraph>
  <multiply name="external" type="color3">
    <input name="in1" type="color3" value="0.5, 0.5, 0.5" />
    <input name="in2" type="color3" value="2, 2, 2" />
  </multiply>
  <pt name="inst" type="multioutput">
    <input name="passconn" type="color3" nodename="external" />
  </pt>
  <add name="consumerVal" type="color3">
    <input name="in1" type="color3" nodename="inst" output="outval" />
    <input name="in2" type="color3" value="0, 0, 0" />
  </add>
  <add name="consumerConn" type="color3">
    <input name="in1" type="color3" nodename="inst" output="outconn" />
    <input name="in2" type="color3" value="0, 0, 0" />
  </add>
  <add name="consumerNode" type="color3">
    <input name="in1" type="color3" nodename="inst" output="outnode" />
    <input name="in2" type="color3" value="0, 0, 0" />
  </add>
  <add name="rootNode" type="color3">
    <input name="in1" type="color3" nodename="consumerVal" />
    <input name="in2" type="color3" nodename="consumerConn" />
  </add>
  <add name="rootNode2" type="color3">
    <input name="in1" type="color3" nodename="rootNode" />
    <input name="in2" type="color3" nodename="consumerNode" />
  </add>
  <output name="root" type="color3" nodename="rootNode2" />
</materialx>
)";

    // A reduced interface keeps the compound node's unbound input carrying a value
    // rather than a connection to a published graph socket, so the value carrying
    // half of the splice is genuinely exercised.
    context.getOptions().shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;

    TestGraph test = createGraphFromString(libraries, xml, "root", context);

    REQUIRE(countCompoundNodes(*test.graph) > 0);

    // Confirm the fixture actually produced a pass-through, so the test cannot
    // quietly stop covering it if elision behaviour changes.
    size_t numPassThroughSockets = 0;
    for (mx::ShaderNode* node : test.graph->getNodes())
    {
        mx::ShaderGraph* compound = compoundGraphOf(node);
        if (!compound)
        {
            continue;
        }
        for (mx::ShaderGraphOutputSocket* outputSocket : compound->getOutputSockets())
        {
            if (outputSocket->getConnection() &&
                outputSocket->getConnection()->getNode() == compound)
            {
                ++numPassThroughSockets;
            }
        }
    }
    REQUIRE(numPassThroughSockets == 2);

    mx::CompoundFlatteningRefactor pass;
    CHECK(pass.execute(*test.graph, context) > 0);
    checkFullyFlattened(*test.graph);

    auto findNode = [&test](const std::string& name) -> mx::ShaderNode*
    {
        for (mx::ShaderNode* node : test.graph->getNodes())
        {
            if (node->getName() == name)
            {
                return node;
            }
        }
        return nullptr;
    };

    // A pass-through fed by a value leaves that value on the external consumer, with
    // no connection left pointing at the node that was expanded away.
    mx::ShaderNode* consumerVal = findNode("consumerVal");
    REQUIRE(consumerVal != nullptr);
    mx::ShaderInput* valIn = consumerVal->getInput("in1");
    REQUIRE(valIn != nullptr);
    CHECK(valIn->getConnection() == nullptr);
    REQUIRE(valIn->getValue() != nullptr);
    CHECK(valIn->getValue()->getValueString() == "0.1, 0.2, 0.3");

    // A pass-through fed by an external node splices that node straight onto the
    // external consumer.
    mx::ShaderNode* consumerConn = findNode("consumerConn");
    REQUIRE(consumerConn != nullptr);
    mx::ShaderInput* connIn = consumerConn->getInput("in1");
    REQUIRE(connIn != nullptr);
    REQUIRE(connIn->getConnection() != nullptr);
    CHECK(connIn->getConnection()->getNode()->getName() == "external");

    // A normal output socket still resolves to the copy of its interior node.
    mx::ShaderNode* consumerNode = findNode("consumerNode");
    REQUIRE(consumerNode != nullptr);
    mx::ShaderInput* nodeIn = consumerNode->getInput("in1");
    REQUIRE(nodeIn != nullptr);
    REQUIRE(nodeIn->getConnection() != nullptr);
    mx::ShaderNode* interior = nodeIn->getConnection()->getNode();
    CHECK(test.graph->getNode(interior->getUniqueId()) == interior);
    REQUIRE(interior->getInput("in2") != nullptr);
    REQUIRE(interior->getInput("in2")->getValue() != nullptr);
    CHECK(interior->getInput("in2")->getValue()->getValueString() == "2, 2, 2");
}

void testFlattenInterfaceValues(mx::DocumentPtr libraries, mx::GenContext& context)
{

    // "cc" exposes two interface inputs. The instance binds "gain" explicitly and leaves
    // "offset" at the definition's default. After flattening, each interior input that
    // read an interface input must carry exactly the value and the authored state that
    // the compound node's matching input had. Forcing the authored flag on would make
    // backends emit parameters that should stay at their defaults.
    //
    // A reduced shader interface is required here: under the default complete interface
    // every unconnected node input is published as a graph input socket and connected to
    // it, so the compound node's inputs would carry connections rather than values and
    // the value propagation path would never run.
    const std::string xml = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_cc" node="cc">
    <input name="gain" type="color3" value="1, 1, 1" />
    <input name="offset" type="color3" value="0.25, 0.25, 0.25" />
    <output name="out" type="color3" />
  </nodedef>
  <nodegraph name="NG_cc" nodedef="ND_cc">
    <multiply name="mul" type="color3">
      <input name="in1" type="color3" value="0.5, 0.5, 0.5" />
      <input name="in2" type="color3" interfacename="gain" />
    </multiply>
    <add name="addNode" type="color3">
      <input name="in1" type="color3" nodename="mul" />
      <input name="in2" type="color3" interfacename="offset" />
    </add>
    <output name="out" type="color3" nodename="addNode" />
  </nodegraph>
  <cc name="inst" type="color3">
    <input name="gain" type="color3" value="0.75, 0.75, 0.75" />
  </cc>
  <output name="root" type="color3" nodename="inst" />
</materialx>
)";

    context.getOptions().shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;

    TestGraph test = createGraphFromString(libraries, xml, "root", context);

    // Record what the compound node's interface inputs carry before it is expanded,
    // so the copies are compared against the real source rather than a guess.
    mx::ShaderNode* compound = nullptr;
    for (mx::ShaderNode* node : test.graph->getNodes())
    {
        if (compoundGraphOf(node) && node->getInput("gain") && node->getInput("offset"))
        {
            compound = node;
            break;
        }
    }
    REQUIRE(compound != nullptr);

    struct PortState
    {
        std::string value;
        bool authored;
    };
    auto capture = [](const mx::ShaderInput* input) -> PortState
    {
        REQUIRE(input->getValue() != nullptr);
        return { input->getValue()->getValueString(), input->hasAuthoredValue() };
    };

    const PortState gainBefore = capture(compound->getInput("gain"));
    const PortState offsetBefore = capture(compound->getInput("offset"));

    // The bound input is authored while the one left at its default is not, so the
    // two cases below genuinely differ.
    CHECK(gainBefore.authored);
    CHECK_FALSE(offsetBefore.authored);
    CHECK(gainBefore.value == "0.75, 0.75, 0.75");

    mx::CompoundFlatteningRefactor pass;
    CHECK(pass.execute(*test.graph, context) > 0);
    checkFullyFlattened(*test.graph);

    auto findInput = [&test](const std::string& nodeSuffix,
                             const std::string& inputName) -> mx::ShaderInput*
    {
        for (mx::ShaderNode* node : test.graph->getNodes())
        {
            const std::string& name = node->getName();
            if (name.size() >= nodeSuffix.size() &&
                name.compare(name.size() - nodeSuffix.size(), nodeSuffix.size(), nodeSuffix) == 0)
            {
                return node->getInput(inputName);
            }
        }
        return nullptr;
    };

    // Each interior input that read an interface input now carries that input's
    // value, with the authored state carried across unchanged.
    mx::ShaderInput* gain = findInput("mul", "in2");
    REQUIRE(gain != nullptr);
    CHECK(gain->getConnection() == nullptr);
    REQUIRE(gain->getValue() != nullptr);
    CHECK(gain->getValue()->getValueString() == gainBefore.value);
    CHECK(gain->hasAuthoredValue() == gainBefore.authored);

    mx::ShaderInput* offset = findInput("addNode", "in2");
    REQUIRE(offset != nullptr);
    CHECK(offset->getConnection() == nullptr);
    REQUIRE(offset->getValue() != nullptr);
    CHECK(offset->getValue()->getValueString() == offsetBefore.value);
    CHECK(offset->hasAuthoredValue() == offsetBefore.authored);

    // An interior input that does not read an interface input keeps its own value,
    // rather than being overwritten by anything on the parent.
    mx::ShaderInput* mulIn1 = findInput("mul", "in1");
    REQUIRE(mulIn1 != nullptr);
    REQUIRE(mulIn1->getValue() != nullptr);
    CHECK(mulIn1->getValue()->getValueString() == "0.5, 0.5, 0.5");
}

void testFlattenOutputFanOut(mx::DocumentPtr libraries, mx::GenContext& context)
{

    // One compound node output drives three separate downstream inputs. All three must
    // be rewired onto the same copy of the interior node, rather than each getting its
    // own copy or one of them being left behind on the node that was expanded away.
    //
    // The interior node is a multiply rather than a constant: NodeElisionRefactor runs
    // by default while the compound graph is finalized and would bypass a constant,
    // leaving nothing to fan out.
    const std::string xml = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_src" node="src">
    <input name="in" type="color3" value="0.4, 0.5, 0.6" />
    <output name="out" type="color3" />
  </nodedef>
  <nodegraph name="NG_src" nodedef="ND_src">
    <multiply name="interior" type="color3">
      <input name="in1" type="color3" interfacename="in" />
      <input name="in2" type="color3" value="2, 2, 2" />
    </multiply>
    <output name="out" type="color3" nodename="interior" />
  </nodegraph>
  <src name="inst" type="color3" />
  <mix name="fanout" type="color3">
    <input name="fg" type="color3" nodename="inst" />
    <input name="bg" type="color3" nodename="inst" />
    <input name="mix" type="float" value="0.5" />
  </mix>
  <add name="rootNode" type="color3">
    <input name="in1" type="color3" nodename="fanout" />
    <input name="in2" type="color3" nodename="inst" />
  </add>
  <output name="root" type="color3" nodename="rootNode" />
</materialx>
)";

    // Set explicitly rather than inherited from whatever ran before, so the compound
    // node's unbound input carries a value instead of a published graph socket.
    context.getOptions().shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;

    TestGraph test = createGraphFromString(libraries, xml, "root", context);

    REQUIRE(countCompoundNodes(*test.graph) == 1);

    mx::CompoundFlatteningRefactor pass;
    CHECK(pass.execute(*test.graph, context) == 1);
    checkFullyFlattened(*test.graph);

    auto upstreamOf = [&test](const std::string& nodeName,
                              const std::string& inputName) -> mx::ShaderNode*
    {
        for (mx::ShaderNode* node : test.graph->getNodes())
        {
            if (node->getName() == nodeName)
            {
                mx::ShaderInput* input = node->getInput(inputName);
                return (input && input->getConnection()) ? input->getConnection()->getNode()
                                                         : nullptr;
            }
        }
        return nullptr;
    };

    mx::ShaderNode* fg = upstreamOf("fanout", "fg");
    mx::ShaderNode* bg = upstreamOf("fanout", "bg");
    mx::ShaderNode* in2 = upstreamOf("rootNode", "in2");
    REQUIRE(fg != nullptr);
    REQUIRE(bg != nullptr);
    REQUIRE(in2 != nullptr);

    // All three consumers must land on one shared copy, which the graph still owns.
    CHECK(fg == bg);
    CHECK(fg == in2);
    CHECK(test.graph->getNode(fg->getUniqueId()) == fg);

    // And that copy must be the interior node, not the compound node left in place.
    CHECK(compoundGraphOf(fg) == nullptr);
    REQUIRE(fg->getInput("in2") != nullptr);
    REQUIRE(fg->getInput("in2")->getValue() != nullptr);
    CHECK(fg->getInput("in2")->getValue()->getValueString() == "2, 2, 2");
}

void testFlattenValueOnlyOutputSocket(mx::DocumentPtr libraries, mx::GenContext& context)
{

    // An output socket can end up carrying a value rather than a connection. Eliding a
    // constant pushes the constant's value onto whatever it fed and breaks the
    // connection, and NodeElisionRefactor runs by default while the compound graph is
    // finalized, so an interior constant wired to an output leaves that output socket
    // unconnected with the value sitting on it.
    //
    // Such a socket has no interior node to splice onto the external consumers, but its
    // value must still reach them: they are about to be disconnected from the node being
    // expanded, and would otherwise silently fall back to their own nodedef defaults.
    //
    // "outconst" covers that case, and "outnode" is a control fed by a surviving interior
    // node. Both a node input and the graph's own output socket consume "outconst", since
    // the two are rewired by the same loop.
    const std::string xml = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_vo" node="vo">
    <input name="in" type="color3" value="0, 0, 0" />
    <output name="outconst" type="color3" />
    <output name="outnode" type="color3" />
  </nodedef>
  <nodegraph name="NG_vo" nodedef="ND_vo">
    <constant name="k" type="color3">
      <input name="value" type="color3" value="0.7, 0.8, 0.9" />
    </constant>
    <multiply name="interior" type="color3">
      <input name="in1" type="color3" interfacename="in" />
      <input name="in2" type="color3" value="2, 2, 2" />
    </multiply>
    <output name="outconst" type="color3" nodename="k" />
    <output name="outnode" type="color3" nodename="interior" />
  </nodegraph>
  <vo name="inst" type="multioutput">
    <input name="in" type="color3" value="0.3, 0.3, 0.3" />
  </vo>
  <add name="consumerConst" type="color3">
    <input name="in1" type="color3" nodename="inst" output="outconst" />
    <input name="in2" type="color3" value="0, 0, 0" />
  </add>
  <add name="consumerNode" type="color3">
    <input name="in1" type="color3" nodename="inst" output="outnode" />
    <input name="in2" type="color3" nodename="consumerConst" />
  </add>
  <output name="root" type="color3" nodename="consumerNode" />
</materialx>
)";

    // A reduced interface keeps the compound node's unbound input carrying a value
    // rather than a connection to a published graph socket.
    const mx::ShaderInterfaceType oldInterfaceType = context.getOptions().shaderInterfaceType;
    context.getOptions().shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;

    TestGraph test = createGraphFromString(libraries, xml, "root", context);

    REQUIRE(countCompoundNodes(*test.graph) == 1);

    // Confirm the fixture actually produced a value carrying output socket, so the test
    // cannot quietly stop covering it if elision behaviour changes. Capture the value
    // from the socket itself rather than restating the literal above.
    std::string socketValue;
    bool socketAuthored = false;
    size_t numValueOnlySockets = 0;
    for (mx::ShaderNode* node : test.graph->getNodes())
    {
        mx::ShaderGraph* compound = compoundGraphOf(node);
        if (!compound)
        {
            continue;
        }
        for (mx::ShaderGraphOutputSocket* outputSocket : compound->getOutputSockets())
        {
            if (!outputSocket->getConnection() && outputSocket->getValue())
            {
                ++numValueOnlySockets;
                socketValue = outputSocket->getValue()->getValueString();
                socketAuthored = outputSocket->hasAuthoredValue();
            }
        }
    }
    REQUIRE(numValueOnlySockets == 1);
    REQUIRE(socketValue == "0.7, 0.8, 0.9");

    mx::CompoundFlatteningRefactor pass;
    CHECK(pass.execute(*test.graph, context) == 1);
    checkFullyFlattened(*test.graph);

    auto findNode = [&test](const std::string& name) -> mx::ShaderNode*
    {
        for (mx::ShaderNode* node : test.graph->getNodes())
        {
            if (node->getName() == name)
            {
                return node;
            }
        }
        return nullptr;
    };

    // The socket's value must have landed on the external consumer, carrying its
    // authored state across. Without this the consumer keeps the add nodedef's own
    // default and the interior constant is silently lost.
    mx::ShaderNode* consumerConst = findNode("consumerConst");
    REQUIRE(consumerConst != nullptr);
    mx::ShaderInput* constIn = consumerConst->getInput("in1");
    REQUIRE(constIn != nullptr);
    CHECK(constIn->getConnection() == nullptr);
    REQUIRE(constIn->getValue() != nullptr);
    CHECK(constIn->getValue()->getValueString() == socketValue);
    CHECK(constIn->hasAuthoredValue() == socketAuthored);

    // A normal output socket still resolves to the copy of its interior node.
    mx::ShaderNode* consumerNode = findNode("consumerNode");
    REQUIRE(consumerNode != nullptr);
    mx::ShaderInput* nodeIn = consumerNode->getInput("in1");
    REQUIRE(nodeIn != nullptr);
    REQUIRE(nodeIn->getConnection() != nullptr);
    mx::ShaderNode* interior = nodeIn->getConnection()->getNode();
    CHECK(test.graph->getNode(interior->getUniqueId()) == interior);
    REQUIRE(interior->getInput("in2") != nullptr);
    REQUIRE(interior->getInput("in2")->getValue() != nullptr);
    CHECK(interior->getInput("in2")->getValue()->getValueString() == "2, 2, 2");

    context.getOptions().shaderInterfaceType = oldInterfaceType;
}

void testFlattenValueOnlyOutputToGraphSocket(mx::DocumentPtr libraries, mx::GenContext& context)
{

    // The same value carrying output socket, but consumed by the graph's own output
    // socket rather than by a node input. Graph output sockets are stored as inputs on
    // the graph, so they are rewired by the same loop, but they are not reachable
    // through getNodes() and so are not covered by the case above.
    //
    // The definition needs a second output even though only the first is consumed here.
    // A compound node inherits the classification of its interior graph, so one wrapping
    // nothing but a constant is itself classified as a constant, and NodeElisionRefactor
    // bypasses any constant with a single input and a single output. With two outputs it
    // survives long enough for this pass to see it.
    const std::string xml = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_vog" node="vog">
    <input name="in" type="color3" value="0, 0, 0" />
    <output name="outconst" type="color3" />
    <output name="outnode" type="color3" />
  </nodedef>
  <nodegraph name="NG_vog" nodedef="ND_vog">
    <constant name="k" type="color3">
      <input name="value" type="color3" value="0.2, 0.4, 0.6" />
    </constant>
    <multiply name="interior" type="color3">
      <input name="in1" type="color3" interfacename="in" />
      <input name="in2" type="color3" value="2, 2, 2" />
    </multiply>
    <output name="outconst" type="color3" nodename="k" />
    <output name="outnode" type="color3" nodename="interior" />
  </nodegraph>
  <vog name="inst" type="multioutput" />
  <output name="root" type="color3" nodename="inst" output="outconst" />
</materialx>
)";

    const mx::ShaderInterfaceType oldInterfaceType = context.getOptions().shaderInterfaceType;
    context.getOptions().shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;

    TestGraph test = createGraphFromString(libraries, xml, "root", context);

    REQUIRE(countCompoundNodes(*test.graph) == 1);

    mx::ShaderGraphOutputSocket* rootSocket = test.graph->getOutputSocket();
    REQUIRE(rootSocket != nullptr);
    REQUIRE(rootSocket->getConnection() != nullptr);

    mx::CompoundFlatteningRefactor pass;
    CHECK(pass.execute(*test.graph, context) == 1);
    checkFullyFlattened(*test.graph);

    // The graph's output socket is left unconnected, so the value has to be on it for
    // the generator to emit anything but the type's default.
    CHECK(rootSocket->getConnection() == nullptr);
    REQUIRE(rootSocket->getValue() != nullptr);
    CHECK(rootSocket->getValue()->getValueString() == "0.2, 0.4, 0.6");

    context.getOptions().shaderInterfaceType = oldInterfaceType;
}

void testCompoundFlatteningConnections(mx::DocumentPtr libraries, mx::GenContext& context)
{
    testFlattenPassThroughNodeGraph(libraries, context);
    testFlattenInterfaceValues(libraries, context);
    testFlattenOutputFanOut(libraries, context);
    testFlattenValueOnlyOutputSocket(libraries, context);
    testFlattenValueOnlyOutputToGraphSocket(libraries, context);
}

TEST_CASE("GenShader: Flatten Compound Node Connections", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlatteningConnections(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlatteningConnections(libraries, context);
    }
#ifdef MATERIALX_BUILD_OSOS
    {
        mx::GenContext context(mx::OslNetworkShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlatteningConnections(libraries, context);
    }
#endif
#endif
#ifdef MATERIALX_BUILD_GEN_MSL
    {
        mx::GenContext context(mx::MslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlatteningConnections(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlatteningConnections(libraries, context);
    }
#endif
#if !defined(MATERIALX_BUILD_GEN_GLSL) && !defined(MATERIALX_BUILD_GEN_OSL) && \
    !defined(MATERIALX_BUILD_GEN_MSL) && !defined(MATERIALX_BUILD_GEN_MDL)
    // Guard against this test reporting success when it covered nothing.
    FAIL("No shader generator was built, so nothing was tested.");
#endif
}

void testFlattenWithoutCompoundNodes(mx::DocumentPtr libraries, mx::GenContext& context)
{

    // A plain <output> over nodes with source code implementations. No surface shader is
    // used here, since standard_surface is implemented by a nodegraph and would make the
    // graph compound.
    const std::string xml = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <multiply name="mul" type="color3">
    <input name="in1" type="color3" value="0.5, 0.5, 0.5" />
    <input name="in2" type="color3" value="2, 2, 2" />
  </multiply>
  <add name="addNode" type="color3">
    <input name="in1" type="color3" nodename="mul" />
    <input name="in2" type="color3" value="0.1, 0.1, 0.1" />
  </add>
  <output name="root" type="color3" nodename="addNode" />
</materialx>
)";

    TestGraph test = createGraphFromString(libraries, xml, "root", context);

    REQUIRE(countCompoundNodes(*test.graph) == 0);

    const std::vector<mx::ShaderNode*> before = test.graph->getNodes();
    REQUIRE_FALSE(before.empty());

    mx::CompoundFlatteningRefactor pass;
    CHECK(pass.execute(*test.graph, context) == 0);

    // The node order must be untouched, not merely the same size.
    CHECK(test.graph->getNodes() == before);
    checkNodeStorageConsistent(*test.graph);
}

void testFlattenedGraphGeneratesSource(mx::DocumentPtr libraries, mx::GenContext& context)
{

    // A smoke test rather than an equivalence proof: flattened output is inlined rather
    // than emitted as nodegraph function calls, so the two forms are not textually
    // comparable. What must hold is that a flattened graph is still a well formed graph
    // that the generator can walk and emit from without error.
    const std::vector<std::string> files = {
        "resources/Materials/TestSuite/stdlib/nodegraphs/nodegraph_multioutput.mtlx",
        "resources/Materials/TestSuite/stdlib/nodegraphs/nodegraph_nodegraph.mtlx",
        "resources/Materials/TestSuite/stdlib/nodegraphs/cascade_nodegraphs.mtlx",
    };
    const std::vector<std::string> elements = {
        "white_multiout_shader",
        "default_shader_top",
        "standard_surface",
    };

    for (size_t i = 0; i < files.size(); ++i)
    {
        TestGraph test = createGraphFromFile(libraries, files[i], elements[i], context);

        mx::CompoundFlatteningRefactor pass;
        CHECK_NOTHROW(pass.execute(*test.graph, context));
        checkFullyFlattened(*test.graph);

        // Topological sorting a flattened graph must succeed, which means the
        // rewired connections form a DAG with no dangling edges.
        CHECK_NOTHROW(test.graph->topologicalSort());
        CHECK_NOTHROW(test.graph->removeUnusedNodes());
        checkNodeStorageConsistent(*test.graph);
    }
}

void testCompoundFlatteningInvariants(mx::DocumentPtr libraries, mx::GenContext& context)
{
    testFlattenWithoutCompoundNodes(libraries, context);
    testFlattenedGraphGeneratesSource(libraries, context);
}

TEST_CASE("GenShader: Flatten Compound Node Graph Invariants", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlatteningInvariants(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlatteningInvariants(libraries, context);
    }
#ifdef MATERIALX_BUILD_OSOS
    {
        mx::GenContext context(mx::OslNetworkShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlatteningInvariants(libraries, context);
    }
#endif
#endif
#ifdef MATERIALX_BUILD_GEN_MSL
    {
        mx::GenContext context(mx::MslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlatteningInvariants(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testCompoundFlatteningInvariants(libraries, context);
    }
#endif
#if !defined(MATERIALX_BUILD_GEN_GLSL) && !defined(MATERIALX_BUILD_GEN_OSL) && \
    !defined(MATERIALX_BUILD_GEN_MSL) && !defined(MATERIALX_BUILD_GEN_MDL)
    // Guard against this test reporting success when it covered nothing.
    FAIL("No shader generator was built, so nothing was tested.");
#endif
}

void testNodeStorageConsistency(mx::DocumentPtr libraries, mx::GenContext& context)
{

    const std::string xml = R"(<?xml version="1.0"?>
<materialx version="1.39">
  <multiply name="mul" type="color3">
    <input name="in1" type="color3" value="0.5, 0.5, 0.5" />
    <input name="in2" type="color3" value="2.0, 2.0, 2.0" />
  </multiply>
  <standard_surface name="surface" type="surfaceshader">
    <input name="base_color" type="color3" nodename="mul" />
  </standard_surface>
</materialx>
)";

    // Each check below starts from a fresh graph, since they all mutate node storage.
    // Catch sections are deliberately not used here: the generator loop would enter the
    // same section once per backend within a single run.

    // removeNode ignores an unknown id without corrupting the map.
    {
        TestGraph test = createGraphFromString(libraries, xml, "surface", context);
        mx::ShaderGraph& graph = *test.graph;
        REQUIRE_FALSE(graph.getNodes().empty());
        mx::ShaderNodeImplPtr impl = graph.getNodes().front()->getImplementationPtr();
        const size_t numNodesBefore = graph.getNodes().size();

        graph.removeNode("no-such-node");

        CHECK(graph.getNodes().size() == numNodesBefore);
        CHECK(graph.getNode("no-such-node") == nullptr);

        // The failed removal must not have left an empty entry behind: creating a
        // node under that same id has to succeed. Looking the id up with operator[]
        // rather than find() would insert a null entry and make this throw.
        mx::ShaderNode* created = nullptr;
        CHECK_NOTHROW(created = graph.createNode("newNode", "no-such-node", impl,
                                                 mx::ShaderNode::Classification::TEXTURE));
        REQUIRE(created != nullptr);
        CHECK(graph.getNode("no-such-node") == created);
        CHECK(graph.getNodes().size() == numNodesBefore + 1);
        checkNodeStorageConsistent(graph);
    }

    // removeNode erases from both the map and the order, and breaks connections.
    {
        TestGraph test = createGraphFromString(libraries, xml, "surface", context);
        mx::ShaderGraph& graph = *test.graph;
        REQUIRE_FALSE(graph.getNodes().empty());

        mx::ShaderNode* existing = graph.getNodes().front();
        const std::string existingId = existing->getUniqueId();
        const mx::ShaderInputVec downstream = existing->getOutput(0)->getConnections();
        const size_t numNodesBefore = graph.getNodes().size();

        graph.removeNode(existingId);

        CHECK(graph.getNode(existingId) == nullptr);
        CHECK(graph.getNodes().size() == numNodesBefore - 1);
        for (mx::ShaderNode* node : graph.getNodes())
        {
            CHECK(node->getUniqueId() != existingId);
        }
        for (mx::ShaderInput* input : downstream)
        {
            CHECK(input->getConnection() == nullptr);
        }
        checkNodeStorageConsistent(graph);
    }

    // createNode rejects a duplicate unique id rather than silently overwriting the
    // owning entry and leaving a dangling pointer in the node order.
    {
        TestGraph test = createGraphFromString(libraries, xml, "surface", context);
        mx::ShaderGraph& graph = *test.graph;
        REQUIRE_FALSE(graph.getNodes().empty());

        mx::ShaderNode* existing = graph.getNodes().front();
        const std::string existingId = existing->getUniqueId();
        mx::ShaderNodeImplPtr impl = existing->getImplementationPtr();

        CHECK_THROWS_AS(graph.createNode("other", existingId, impl,
                                         mx::ShaderNode::Classification::TEXTURE),
                        mx::ExceptionShaderGenError);

        CHECK(graph.getNode(existingId) == existing);
        checkNodeStorageConsistent(graph);
    }

    // removeNodes erases a batch in one pass and ignores unknown ids.
    {
        TestGraph test = createGraphFromString(libraries, xml, "surface", context);
        mx::ShaderGraph& graph = *test.graph;
        REQUIRE_FALSE(graph.getNodes().empty());

        mx::StringSet ids;
        for (mx::ShaderNode* node : graph.getNodes())
        {
            ids.insert(node->getUniqueId());
        }
        const std::string knownId = *ids.begin();
        ids.insert("no-such-node");

        graph.removeNodes(ids);

        CHECK(graph.getNodes().empty());
        CHECK(graph.getNode(knownId) == nullptr);
    }
}

void testShaderGraphNodeStorage(mx::DocumentPtr libraries, mx::GenContext& context)
{
    testNodeStorageConsistency(libraries, context);
}

TEST_CASE("GenShader: Shader Graph Node Storage", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testShaderGraphNodeStorage(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testShaderGraphNodeStorage(libraries, context);
    }
#ifdef MATERIALX_BUILD_OSOS
    {
        mx::GenContext context(mx::OslNetworkShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testShaderGraphNodeStorage(libraries, context);
    }
#endif
#endif
#ifdef MATERIALX_BUILD_GEN_MSL
    {
        mx::GenContext context(mx::MslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testShaderGraphNodeStorage(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testShaderGraphNodeStorage(libraries, context);
    }
#endif
#if !defined(MATERIALX_BUILD_GEN_GLSL) && !defined(MATERIALX_BUILD_GEN_OSL) && \
    !defined(MATERIALX_BUILD_GEN_MSL) && !defined(MATERIALX_BUILD_GEN_MDL)
    // Guard against this test reporting success when it covered nothing.
    FAIL("No shader generator was built, so nothing was tested.");
#endif
}
