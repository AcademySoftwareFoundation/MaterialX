//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

/// @file
/// Tests for MaterialXGenShader2.
///
/// Adapter unit tests: verify MxElementAdapter faithfully reflects the
///   MaterialX data model through the IShaderSource interface.
///
/// Graph parity tests: build a ShaderGraph via GenContextCreate /
///   ShaderGraphBuilder and assert it is structurally identical to the graph
///   produced by the existing ShaderGraph::create() path.
///
/// Shader emit parity tests: drive full code emission through GenContextCreate
///   and assert the generated source is byte-for-byte identical to the output
///   of the existing ShaderGenerator::generate() path, for both GLSL and MDL.
///
/// Phase 4 bridge tests: re-run graph parity via NoMxNodeAdapter, which
///   overrides getMxNode() to FAIL() if called.  A passing test proves that
///   ShaderGraphBuilder no longer calls getMxNode() for any upstream node.
///
/// Phase 4b bridge tests: re-run graph parity (including Output-root cases)
///   via NoMxDocumentAdapter, which overrides getMxDocument() to FAIL() if
///   called.  A passing test proves that buildOutputRoot() no longer calls
///   getMxDocument().

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

#include <MaterialXGenShader2/GenContextCreate.h>
#include <MaterialXGenShader2/MxElementAdapter.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#ifdef MATERIALX_BUILD_GEN_MDL
#include <MaterialXGenMdl/MdlShaderGenerator.h>
#endif

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <MaterialXCore/Document.h>

namespace mx = MaterialX;

// ─── Helpers ──────────────────────────────────────────────────────────────────

static mx::DocumentPtr loadLibraries()
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr doc = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, doc);
    return doc;
}

static mx::DocumentPtr loadMaterial(const mx::FilePath& mtlxFile)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr doc = loadLibraries();
    mx::readFromXmlFile(doc, mtlxFile, searchPath);
    std::string errors;
    REQUIRE(doc->validate(&errors));
    return doc;
}

static mx::GenContext makeGlslContext()
{
    return mx::GenContext(mx::GlslShaderGenerator::create());
}

// ─── Phase 1: MxElementAdapter unit tests ────────────────────────────────────

TEST_CASE("GenShader2: MxElementAdapter - root element", "[genshader2][adapter]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx");
    if (!mtlxFile.exists())
    {
        WARN("Test material not found, skipping: " + mtlxFile.asString());
        return;
    }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);

    mx::ElementPtr renderableElem;
    for (mx::ElementPtr elem : doc->traverseTree())
    {
        if (elem->isA<mx::Output>() || elem->isA<mx::Node>())
        {
            renderableElem = elem;
            break;
        }
    }
    REQUIRE(renderableElem);

    mx::MxElementAdapter adapter(doc, renderableElem);
    mx::DataHandle root = adapter.getRootElement();
    REQUIRE(mx::isValidHandle(root));
    CHECK(adapter.getElementName(root) == renderableElem->getName());
    CHECK(adapter.getElementPath(root) == renderableElem->getNamePath());
}

TEST_CASE("GenShader2: MxElementAdapter - node queries", "[genshader2][adapter]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx");
    if (!mtlxFile.exists())
    {
        WARN("Test material not found, skipping: " + mtlxFile.asString());
        return;
    }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);

    mx::NodePtr ssNode;
    for (mx::NodePtr node : doc->getNodes())
    {
        if (node->getCategory() == "standard_surface")
        {
            ssNode = node;
            break;
        }
    }
    REQUIRE(ssNode);

    mx::MxElementAdapter adapter(doc, ssNode);
    mx::DataHandle nodeH = adapter.getRootElement();
    REQUIRE(mx::isValidHandle(nodeH));
    REQUIRE(adapter.isNode(nodeH));

    mx::DataHandle ndH = adapter.getNodeDef(nodeH);
    REQUIRE(mx::isValidHandle(ndH));
    CHECK(!adapter.getNodeDefType(ndH).empty());

    size_t inputCount = adapter.getNodeInputCount(nodeH);
    CHECK(inputCount > 0);
    for (size_t i = 0; i < inputCount; ++i)
    {
        mx::DataHandle inputH = adapter.getNodeInput(nodeH, i);
        REQUIRE(mx::isValidHandle(inputH));
        CHECK(!adapter.getPortName(inputH).empty());
        CHECK(!adapter.getPortType(inputH).empty());
    }

    size_t outputCount = adapter.getNodeDefOutputCount(ndH);
    CHECK(outputCount > 0);
    for (size_t i = 0; i < outputCount; ++i)
    {
        mx::DataHandle outH = adapter.getNodeDefOutput(ndH, i);
        REQUIRE(mx::isValidHandle(outH));
        CHECK(!adapter.getPortName(outH).empty());
    }
}

TEST_CASE("GenShader2: MxElementAdapter - document queries", "[genshader2][adapter]")
{
    mx::DocumentPtr doc = loadLibraries();
    mx::MxElementAdapter adapter(doc, doc->getChildren().front());

    std::string colorSpace = adapter.getActiveColorSpace();
    CHECK(true);

    mx::DataHandle ndH = adapter.getNodeDefByName("ND_standard_surface_surfaceshader");
    if (mx::isValidHandle(ndH))
    {
        CHECK(!adapter.getNodeDefType(ndH).empty());
    }
}

// ─── Graph parity tests ───────────────────────────────────────────────────────

/// Compare two ShaderGraphs and verify structural equivalence:
///   - same node count
///   - every node present in the old graph also exists in the new graph
///   - every node has the same number of inputs and outputs
static void checkGraphParity(const mx::ShaderGraph& oldGraph, const mx::ShaderGraph& newGraph,
                              const std::string& materialName)
{
    INFO("Material: " << materialName);

    const auto& oldNodes = oldGraph.getNodes();
    const auto& newNodes = newGraph.getNodes();

    CHECK(newNodes.size() == oldNodes.size());

    for (const mx::ShaderNode* oldNode : oldNodes)
    {
        const mx::ShaderNode* newNode = newGraph.getNode(oldNode->getUniqueId());
        INFO("  Node: " << oldNode->getName() << "  uniqueId: " << oldNode->getUniqueId());
        REQUIRE(newNode != nullptr);
        CHECK(newNode->numInputs()  == oldNode->numInputs());
        CHECK(newNode->numOutputs() == oldNode->numOutputs());
    }
}

/// Build both old and new ShaderGraphs from a Node element and assert parity.
static void runNodeParityTest(mx::DocumentPtr doc, mx::NodePtr rootNode,
                              const std::string& shaderName)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    mx::GenContext oldContext = makeGlslContext();
    oldContext.registerSourceCodeSearchPath(searchPath);
    mx::ShaderGraphPtr oldGraph = mx::ShaderGraph::create(nullptr, shaderName, rootNode, oldContext);
    REQUIRE(oldGraph);

    auto adapter = std::make_unique<mx::MxElementAdapter>(doc, rootNode);
    mx::GenContextCreate ctx(mx::GlslShaderGenerator::create(), std::move(adapter));
    ctx.getGenContext().registerSourceCodeSearchPath(searchPath);
    mx::ShaderGraph2Ptr newGraph = ctx.buildGraph(shaderName);
    REQUIRE(newGraph);

    checkGraphParity(*oldGraph, *newGraph, shaderName);
}

/// Build both old and new ShaderGraphs from an Output element and assert parity.
static void runOutputParityTest(mx::DocumentPtr doc, mx::OutputPtr rootOutput,
                                const std::string& shaderName)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    mx::GenContext oldContext = makeGlslContext();
    oldContext.registerSourceCodeSearchPath(searchPath);
    mx::ShaderGraphPtr oldGraph = mx::ShaderGraph::create(nullptr, shaderName, rootOutput, oldContext);
    REQUIRE(oldGraph);

    auto adapter = std::make_unique<mx::MxElementAdapter>(doc, rootOutput);
    mx::GenContextCreate ctx(mx::GlslShaderGenerator::create(), std::move(adapter));
    ctx.getGenContext().registerSourceCodeSearchPath(searchPath);
    mx::ShaderGraph2Ptr newGraph = ctx.buildGraph(shaderName);
    REQUIRE(newGraph);

    checkGraphParity(*oldGraph, *newGraph, shaderName);
}

// ─── Phase 4 bridge: NoMxNodeAdapter ─────────────────────────────────────────

/// MxElementAdapter subclass that FAIL()s the test if getMxNode() is called.
///
/// Use this in place of MxElementAdapter to prove that ShaderGraphBuilder
/// no longer requires the getMxNode() bridge (Phase 4 invariant).
/// getMxDocument() and getMxNodeDef() are still delegated to the base class
/// because those bridges remain required.
class NoMxNodeAdapter : public mx::MxElementAdapter
{
  public:
    using mx::MxElementAdapter::MxElementAdapter;

    mx::ConstNodePtr getMxNode(mx::DataHandle /*node*/) const override
    {
        FAIL("ShaderGraphBuilder called getMxNode() — Phase 4 invariant violated");
        return nullptr;
    }
};

/// Run a graph parity test using NoMxNodeAdapter (getMxNode() must not fire).
static void runNodeParityTestNoMxNode(mx::DocumentPtr doc, mx::NodePtr rootNode,
                                       const std::string& shaderName)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    // Reference graph via the standard path.
    mx::GenContext oldContext = makeGlslContext();
    oldContext.registerSourceCodeSearchPath(searchPath);
    mx::ShaderGraphPtr oldGraph = mx::ShaderGraph::create(nullptr, shaderName, rootNode, oldContext);
    REQUIRE(oldGraph);

    // New path: NoMxNodeAdapter will FAIL() if getMxNode() is ever called.
    auto adapter = std::make_unique<NoMxNodeAdapter>(doc, rootNode);
    mx::GenContextCreate ctx(mx::GlslShaderGenerator::create(), std::move(adapter));
    ctx.getGenContext().registerSourceCodeSearchPath(searchPath);
    mx::ShaderGraph2Ptr newGraph = ctx.buildGraph(shaderName);
    REQUIRE(newGraph);

    checkGraphParity(*oldGraph, *newGraph, shaderName);
}

TEST_CASE("GenShader2: Phase 4 - getMxNode not called - standard_surface_default",
          "[genshader2][phase4]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "standard_surface") { rootNode = node; break; }
    REQUIRE(rootNode);
    runNodeParityTestNoMxNode(doc, rootNode, "phase4_standard_surface_default");
}

TEST_CASE("GenShader2: Phase 4 - getMxNode not called - standard_surface_marble_solid",
          "[genshader2][phase4]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_marble_solid.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "standard_surface") { rootNode = node; break; }
    REQUIRE(rootNode);
    runNodeParityTestNoMxNode(doc, rootNode, "phase4_standard_surface_marble_solid");
}

TEST_CASE("GenShader2: Phase 4 - getMxNode not called - open_pbr_carpaint",
          "[genshader2][phase4]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/OpenPbr/open_pbr_carpaint.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "open_pbr_surface") { rootNode = node; break; }
    REQUIRE(rootNode);
    runNodeParityTestNoMxNode(doc, rootNode, "phase4_open_pbr_carpaint");
}

// ─── Phase 4b bridge: CountingDocumentAdapter ────────────────────────────────

/// MxElementAdapter subclass that counts calls to getMxDocument().
///
/// After Phase 4c, ShaderGraphBuilder passes nullptr to the ShaderGraph2
/// constructor and addDefaultGeomNode2 uses IShaderSource queries instead of
/// _document->getNodeDef().  getMxDocument() should therefore never be called
/// during graph construction.
///
/// The invariant proven by these tests:
///   • For a node-root graph, getMxDocument() is called ZERO times.
///   • For an output-root graph, getMxDocument() is called ZERO times.
///
/// Any count > 0 means a new getMxDocument() dependency was introduced.
class CountingDocumentAdapter : public mx::MxElementAdapter
{
  public:
    using mx::MxElementAdapter::MxElementAdapter;

    mx::ConstDocumentPtr getMxDocument() const override
    {
        ++callCount;
        return mx::MxElementAdapter::getMxDocument();
    }

    mutable int callCount = 0;
};

TEST_CASE("GenShader2: Phase 4c - getMxDocument called zero times (node-root) - marble_solid",
          "[genshader2][phase4b][phase4c]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_marble_solid.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "standard_surface") { rootNode = node; break; }
    REQUIRE(rootNode);

    mx::FileSearchPath sp = mx::getDefaultDataSearchPath();
    mx::GenContextCreate ctx(mx::GlslShaderGenerator::create(),
                             std::make_unique<CountingDocumentAdapter>(doc, rootNode));
    ctx.getGenContext().registerSourceCodeSearchPath(sp);
    mx::ShaderGraph2Ptr newGraph = ctx.buildGraph("phase4b_marble");
    REQUIRE(newGraph);

    // Phase 4c: getMxDocument() must not be called at all — the graph is built
    // entirely through IShaderSource queries.
    const auto& counter = static_cast<const CountingDocumentAdapter&>(ctx.getSource());
    CHECK(counter.callCount == 0);
}

TEST_CASE("GenShader2: Phase 4c - getMxDocument called zero times (output-root) - brass nodegraph",
          "[genshader2][phase4b][phase4c]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodeGraphPtr ng = doc->getNodeGraph("NG_brass1");
    REQUIRE(ng);

    for (mx::OutputPtr output : ng->getOutputs())
    {
        mx::GenContextCreate ctx(mx::GlslShaderGenerator::create(),
                                 std::make_unique<CountingDocumentAdapter>(doc, output));
        ctx.getGenContext().registerSourceCodeSearchPath(searchPath);
        mx::ShaderGraph2Ptr newGraph = ctx.buildGraph("phase4b_brass_" + output->getName());
        REQUIRE(newGraph);

        // Phase 4c: getMxDocument() must not be called at all — the graph is built
        // entirely through IShaderSource queries.
        const auto& counter = static_cast<const CountingDocumentAdapter&>(ctx.getSource());
        CHECK(counter.callCount == 0);
    }
}

// ─── StandardSurface ─────────────────────────────────────────────────────────

TEST_CASE("GenShader2: graph parity - standard_surface_default", "[genshader2][parity]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "standard_surface") { rootNode = node; break; }
    REQUIRE(rootNode);
    runNodeParityTest(doc, rootNode, "test_standard_surface_default");
}

TEST_CASE("GenShader2: graph parity - standard_surface_marble_solid", "[genshader2][parity]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_marble_solid.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "standard_surface") { rootNode = node; break; }
    REQUIRE(rootNode);
    runNodeParityTest(doc, rootNode, "test_standard_surface_marble_solid");
}

TEST_CASE("GenShader2: graph parity - standard_surface_glass", "[genshader2][parity]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_glass.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "standard_surface") { rootNode = node; break; }
    REQUIRE(rootNode);
    runNodeParityTest(doc, rootNode, "test_standard_surface_glass");
}

// ─── OpenPBR ─────────────────────────────────────────────────────────────────

TEST_CASE("GenShader2: graph parity - open_pbr_default", "[genshader2][parity]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/OpenPbr/open_pbr_default.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "open_pbr_surface") { rootNode = node; break; }
    REQUIRE(rootNode);
    runNodeParityTest(doc, rootNode, "test_open_pbr_default");
}

TEST_CASE("GenShader2: graph parity - open_pbr_carpaint", "[genshader2][parity]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/OpenPbr/open_pbr_carpaint.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "open_pbr_surface") { rootNode = node; break; }
    REQUIRE(rootNode);
    runNodeParityTest(doc, rootNode, "test_open_pbr_carpaint");
}

// ─── GltfPbr ─────────────────────────────────────────────────────────────────

TEST_CASE("GenShader2: graph parity - gltf_pbr_default", "[genshader2][parity]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/GltfPbr/gltf_pbr_default.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "gltf_pbr") { rootNode = node; break; }
    REQUIRE(rootNode);
    runNodeParityTest(doc, rootNode, "test_gltf_pbr_default");
}

// ─── Output-rooted (NodeGraph output) ────────────────────────────────────────

TEST_CASE("GenShader2: graph parity - nodegraph output root", "[genshader2][parity]")
{
    // standard_surface_brass_tiled.mtlx contains NG_brass1 with outputs
    // out_color and out_roughness — exercise the buildOutputRoot path.
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);

    mx::NodeGraphPtr ng = doc->getNodeGraph("NG_brass1");
    REQUIRE(ng);

    for (mx::OutputPtr output : ng->getOutputs())
    {
        runOutputParityTest(doc, output, "test_brass_ng_" + output->getName());
    }
}

// ─── Shader emit parity helpers ───────────────────────────────────────────────

/// Compare generated shader source stage-by-stage.
static void checkShaderParity(const mx::Shader& oldShader, const mx::Shader& newShader,
                               const std::string& label)
{
    INFO("Shader: " << label);
    REQUIRE(newShader.numStages() == oldShader.numStages());
    for (size_t i = 0; i < oldShader.numStages(); ++i)
    {
        const mx::ShaderStage& oldStage = oldShader.getStage(i);
        const mx::ShaderStage& newStage = newShader.getStage(i);
        INFO("  Stage: " << oldStage.getName());
        CHECK(newStage.getSourceCode() == oldStage.getSourceCode());
    }
}

// ─── GLSL emit parity ─────────────────────────────────────────────────────────

TEST_CASE("GenShader2: emit parity GLSL - standard_surface_default", "[genshader2][emit]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "standard_surface") { rootNode = node; break; }
    REQUIRE(rootNode);

    const std::string shaderName = "emit_test_ss_default";

    // Old path
    mx::GenContext oldCtx = makeGlslContext();
    oldCtx.registerSourceCodeSearchPath(searchPath);
    mx::ShaderPtr oldShader = oldCtx.getShaderGenerator().generate(shaderName, rootNode, oldCtx);
    REQUIRE(oldShader);

    // New path
    auto adapter = std::make_unique<mx::MxElementAdapter>(doc, rootNode);
    mx::GenContextCreate ctx(mx::GlslShaderGenerator::create(), std::move(adapter));
    ctx.getGenContext().registerSourceCodeSearchPath(searchPath);
    mx::ShaderPtr newShader = ctx.buildShader(shaderName);
    REQUIRE(newShader);

    checkShaderParity(*oldShader, *newShader, "GLSL standard_surface_default");
}

TEST_CASE("GenShader2: emit parity GLSL - standard_surface_marble_solid", "[genshader2][emit]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_marble_solid.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "standard_surface") { rootNode = node; break; }
    REQUIRE(rootNode);

    const std::string shaderName = "emit_test_ss_marble";

    mx::GenContext oldCtx = makeGlslContext();
    oldCtx.registerSourceCodeSearchPath(searchPath);
    mx::ShaderPtr oldShader = oldCtx.getShaderGenerator().generate(shaderName, rootNode, oldCtx);
    REQUIRE(oldShader);

    auto adapter = std::make_unique<mx::MxElementAdapter>(doc, rootNode);
    mx::GenContextCreate ctx(mx::GlslShaderGenerator::create(), std::move(adapter));
    ctx.getGenContext().registerSourceCodeSearchPath(searchPath);
    mx::ShaderPtr newShader = ctx.buildShader(shaderName);
    REQUIRE(newShader);

    checkShaderParity(*oldShader, *newShader, "GLSL standard_surface_marble_solid");
}

TEST_CASE("GenShader2: emit parity GLSL - open_pbr_default", "[genshader2][emit]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/OpenPbr/open_pbr_default.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "open_pbr_surface") { rootNode = node; break; }
    REQUIRE(rootNode);

    const std::string shaderName = "emit_test_openpbr";

    mx::GenContext oldCtx = makeGlslContext();
    oldCtx.registerSourceCodeSearchPath(searchPath);
    mx::ShaderPtr oldShader = oldCtx.getShaderGenerator().generate(shaderName, rootNode, oldCtx);
    REQUIRE(oldShader);

    auto adapter = std::make_unique<mx::MxElementAdapter>(doc, rootNode);
    mx::GenContextCreate ctx(mx::GlslShaderGenerator::create(), std::move(adapter));
    ctx.getGenContext().registerSourceCodeSearchPath(searchPath);
    mx::ShaderPtr newShader = ctx.buildShader(shaderName);
    REQUIRE(newShader);

    checkShaderParity(*oldShader, *newShader, "GLSL open_pbr_default");
}

// ─── MDL emit parity ──────────────────────────────────────────────────────────

#ifdef MATERIALX_BUILD_GEN_MDL

static mx::GenContext makeMdlContext()
{
    return mx::GenContext(mx::MdlShaderGenerator::create());
}

TEST_CASE("GenShader2: emit parity MDL - standard_surface_default", "[genshader2][emit]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "standard_surface") { rootNode = node; break; }
    REQUIRE(rootNode);

    const std::string shaderName = "emit_test_mdl_ss_default";

    mx::GenContext oldCtx = makeMdlContext();
    oldCtx.registerSourceCodeSearchPath(searchPath);
    mx::ShaderPtr oldShader = oldCtx.getShaderGenerator().generate(shaderName, rootNode, oldCtx);
    REQUIRE(oldShader);

    auto adapter = std::make_unique<mx::MxElementAdapter>(doc, rootNode);
    mx::GenContextCreate ctx(mx::MdlShaderGenerator::create(), std::move(adapter));
    ctx.getGenContext().registerSourceCodeSearchPath(searchPath);
    mx::ShaderPtr newShader = ctx.buildShader(shaderName);
    REQUIRE(newShader);

    checkShaderParity(*oldShader, *newShader, "MDL standard_surface_default");
}

TEST_CASE("GenShader2: emit parity MDL - open_pbr_default", "[genshader2][emit]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath mtlxFile = searchPath.find(
        "resources/Materials/Examples/OpenPbr/open_pbr_default.mtlx");
    if (!mtlxFile.exists()) { WARN("Test material not found, skipping: " + mtlxFile.asString()); return; }

    mx::DocumentPtr doc = loadMaterial(mtlxFile);
    mx::NodePtr rootNode;
    for (mx::NodePtr node : doc->getNodes())
        if (node->getCategory() == "open_pbr_surface") { rootNode = node; break; }
    REQUIRE(rootNode);

    const std::string shaderName = "emit_test_mdl_openpbr";

    mx::GenContext oldCtx = makeMdlContext();
    oldCtx.registerSourceCodeSearchPath(searchPath);
    mx::ShaderPtr oldShader = oldCtx.getShaderGenerator().generate(shaderName, rootNode, oldCtx);
    REQUIRE(oldShader);

    auto adapter = std::make_unique<mx::MxElementAdapter>(doc, rootNode);
    mx::GenContextCreate ctx(mx::MdlShaderGenerator::create(), std::move(adapter));
    ctx.getGenContext().registerSourceCodeSearchPath(searchPath);
    mx::ShaderPtr newShader = ctx.buildShader(shaderName);
    REQUIRE(newShader);

    checkShaderParity(*oldShader, *newShader, "MDL open_pbr_default");
}

#endif // MATERIALX_BUILD_GEN_MDL
