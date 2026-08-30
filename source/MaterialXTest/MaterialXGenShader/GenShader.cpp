//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <MaterialXGenHw/HwConstants.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/ShaderTranslator.h>
#include <MaterialXGenShader/Util.h>

#ifdef MATERIALX_BUILD_GEN_GLSL
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
#include <MaterialXGenOsl/OslShaderGenerator.h>
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
#include <MaterialXGenMdl/MdlShaderGenerator.h>
#endif
#ifdef MATERIALX_BUILD_GEN_MSL
#include <MaterialXGenMsl/MslShaderGenerator.h>
#endif
#ifdef MATERIALX_BUILD_GEN_SLANG
#include <MaterialXGenSlang/SlangShaderGenerator.h>
#endif

#include <cstdlib>
#include <iostream>
#include <vector>
#include <set>

namespace mx = MaterialX;

//
// Base tests
//

TEST_CASE("GenShader: Utilities", "[genshader]")
{
    // Test simple text substitution
    std::string test1 = "Look behind you, a $threeheaded $monkey!";
    std::string result1 = "Look behind you, a mighty pirate!";
    mx::StringMap subst1 = { {"$threeheaded","mighty"}, {"$monkey","pirate"} };
    mx::tokenSubstitution(subst1, test1);
    REQUIRE(test1 == result1);

    // Test uniform name substitution
    std::string test2 = "uniform vec3 " + mx::HW::T_ENV_RADIANCE + ";";
    std::string result2 = "uniform vec3 " + mx::HW::ENV_RADIANCE + ";";
    mx::StringMap subst2 = { {mx::HW::T_ENV_RADIANCE, mx::HW::ENV_RADIANCE} };
    mx::tokenSubstitution(subst2, test2);
    REQUIRE(test2 == result2);
}

TEST_CASE("GenShader: Valid Libraries", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr doc = mx::createDocument();
    loadLibraries({ "libraries" }, searchPath, doc);

    std::string validationErrors;
    bool valid = doc->validate(&validationErrors);
    if (!valid)
    {
        std::cout << validationErrors << std::endl;
    }
    REQUIRE(valid);
}

TEST_CASE("GenShader: TypeDesc Check", "[genshader]")
{
    mx::TypeSystemPtr ts = mx::TypeSystem::create();
    mx::GenContext context(mx::GlslShaderGenerator::create(ts));

    // Make sure the standard types are registered
    const mx::TypeDesc floatType = ts->getType("float");
    REQUIRE(floatType != mx::Type::NONE);
    REQUIRE(floatType.getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    const mx::TypeDesc integerType = ts->getType("integer");
    REQUIRE(integerType != mx::Type::NONE);
    REQUIRE(integerType.getBaseType() == mx::TypeDesc::BASETYPE_INTEGER);
    const mx::TypeDesc booleanType = ts->getType("boolean");
    REQUIRE(booleanType != mx::Type::NONE);
    REQUIRE(booleanType.getBaseType() == mx::TypeDesc::BASETYPE_BOOLEAN);
    const mx::TypeDesc color3Type = ts->getType("color3");
    REQUIRE(color3Type != mx::Type::NONE);
    REQUIRE(color3Type.getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color3Type.getSemantic() == mx::TypeDesc::SEMANTIC_COLOR);
    REQUIRE(color3Type.isFloat3());
    const mx::TypeDesc color4Type = ts->getType("color4");
    REQUIRE(color4Type != mx::Type::NONE);
    REQUIRE(color4Type.getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color4Type.getSemantic() == mx::TypeDesc::SEMANTIC_COLOR);
    REQUIRE(color4Type.isFloat4());

    // Make sure we can register a new custom type
    const std::string fooTypeName = "foo";
    ts->registerType(fooTypeName, mx::TypeDesc::BASETYPE_FLOAT, mx::TypeDesc::SEMANTIC_COLOR, 5);
    mx::TypeDesc fooType = ts->getType(fooTypeName);
    REQUIRE(fooType != mx::Type::NONE);
    REQUIRE(fooType.getSemantic() == mx::TypeDesc::SEMANTIC_COLOR);

    // Make sure we can register a new type replacing an old type
    ts->registerType(fooTypeName, mx::TypeDesc::BASETYPE_INTEGER, mx::TypeDesc::SEMANTIC_VECTOR, 3);
    fooType = ts->getType(fooTypeName);
    REQUIRE(fooType != mx::Type::NONE);
    REQUIRE(fooType.getSemantic() == mx::TypeDesc::SEMANTIC_VECTOR);

    // Make sure we can't request an unknown type
    REQUIRE(ts->getType("bar") == mx::Type::NONE);
}

TEST_CASE("GenShader: Shader Translation", "[translate]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::ShaderTranslatorPtr shaderTranslator = mx::ShaderTranslator::create();

    mx::FilePath testPath = searchPath.find("resources/Materials/Examples/StandardSurface");
    for (mx::FilePath& mtlxFile : testPath.getFilesInDirectory(mx::MTLX_EXTENSION))
    {
        mx::DocumentPtr doc = mx::createDocument();
        loadLibraries({ "libraries/targets", "libraries/stdlib", "libraries/pbrlib", "libraries/bxdf" }, searchPath, doc);

        mx::readFromXmlFile(doc, testPath / mtlxFile, searchPath);
        mtlxFile.removeExtension();

        bool translated = false;
        try
        {
            shaderTranslator->translateAllMaterials(doc, "UsdPreviewSurface");
            translated = true;
        }
        catch (mx::Exception &e)
        {
            std::cout << "Failed translating: " << (testPath / mtlxFile).asString() << ": " << e.what() << std::endl;
        }
        REQUIRE(translated);

        std::string validationErrors;
        bool valid = doc->validate(&validationErrors);
        if (!doc->validate(&validationErrors))
        {
            std::cout << "Shader translation of " << (testPath / mtlxFile).asString() << " failed" << std::endl;
            std::cout << "Validation errors: " << validationErrors << std::endl;
        }
        REQUIRE(valid);
    }
}

TEST_CASE("GenShader: Transparency Regression Check", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    const mx::FilePath resourcePath = searchPath.find("resources");
    mx::StringVec failedTests;
    mx::FilePathVec testFiles = { 
        "Materials/Examples/StandardSurface/standard_surface_default.mtlx", 
        "Materials/Examples/StandardSurface/standard_surface_glass.mtlx",
        "Materials/TestSuite/libraries/metal/brass_wire_mesh.mtlx"
    };
    std::vector<bool> transparencyTest = { false, true, true };
    for (size_t i=0; i<testFiles.size(); i++)
    {
        const mx::FilePath& testFile = resourcePath / testFiles[i];
        bool testValue = transparencyTest[i];

        mx::DocumentPtr testDoc = mx::createDocument();
        testDoc->setDataLibrary(libraries);

        try
        {
            mx::readFromXmlFile(testDoc, testFile, searchPath);
            std::vector<mx::TypedElementPtr> renderables = mx::findRenderableElements(testDoc);
            for (auto renderable : renderables)
            {
                mx::NodePtr node = renderable->asA<mx::Node>();
                if (!node)
                {
                    continue;
                }
                if (testValue != mx::isTransparentSurface(node))
                {
                    failedTests.push_back(std::string("File: ") + testFile.asString() + std::string(". Element: ")
                        + renderable->getNamePath() + std::string(" should be:" + std::to_string(testValue)));
                }
            }
        }
        catch (mx::Exception& e)
        {
            INFO(std::string("Test failed: ") + std::string(e.what()));
        }
    }
    for (auto failedTest : failedTests)
    {
        INFO(failedTest);
    }
    CHECK(failedTests.empty());
}

void testDeterministicGeneration(mx::DocumentPtr libraries, mx::GenContext& context)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath testFile = searchPath.find("resources/Materials/Examples/StandardSurface/standard_surface_marble_solid.mtlx");
    mx::string testElement = "SR_marble1";

    const size_t numRuns = 10;
    mx::vector<mx::DocumentPtr> testDocs(numRuns);
    mx::StringVec sourceCode(numRuns);

    for (size_t i = 0; i < numRuns; ++i)
    {
        mx::DocumentPtr testDoc = mx::createDocument();
        mx::readFromXmlFile(testDoc, testFile);
        testDoc->setDataLibrary(libraries);

        // Keep the document alive to make sure
        // new memory is allocated for each run
        testDocs[i] = testDoc;

        mx::ElementPtr element = testDoc->getChild(testElement);
        CHECK(element);

        mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
        sourceCode[i] = shader->getSourceCode();

        if (i > 0)
        {
            // Check if the generated source code is the same
            // for each successive run.
            CHECK(sourceCode[i] == sourceCode[0]);
        }
    }
}

TEST_CASE("GenShader: Deterministic Generation", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testDeterministicGeneration(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testDeterministicGeneration(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testDeterministicGeneration(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MSL
    {
        mx::GenContext context(mx::MslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testDeterministicGeneration(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_SLANG
    {
        mx::GenContext context(mx::SlangShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        testDeterministicGeneration(libraries, context);
    }
#endif
}

void checkPixelDependencies(mx::DocumentPtr libraries, mx::GenContext& context)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath testFile = searchPath.find("resources/Materials/Examples/GltfPbr/gltf_pbr_boombox.mtlx");
    mx::string testElement = "Material_boombox";

    mx::DocumentPtr testDoc = mx::createDocument();
    mx::readFromXmlFile(testDoc, testFile);
    testDoc->setDataLibrary(libraries);

    mx::ElementPtr element = testDoc->getChild(testElement);
    CHECK(element);

    mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
    std::set<std::string> dependencies = shader->getStage("pixel").getSourceDependencies();
    for (auto dependency : dependencies) {
        mx::FilePath path(dependency);
        REQUIRE(path.exists() == true);
    }
}

TEST_CASE("GenShader: Track Dependencies", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkPixelDependencies(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkPixelDependencies(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkPixelDependencies(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_SLANG
    {
        mx::GenContext context(mx::SlangShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkPixelDependencies(libraries, context);
    }
#endif
}

void variableTracker(mx::ShaderNode* node, mx::GenContext& /*context*/)
{
    static mx::StringMap results;
    results["primvar_one"] = "geompropvalue1/geomprop";
    results["primvar_two"] = "geompropvalue2/geomprop";
    results["0"] = "Tworld";
    results["upstream_primvar"] = "constant/value";

    if (node->hasClassification(mx::ShaderNode::Classification::GEOMETRIC))
    {
        const mx::ShaderInput* geomPropInput = node->getInput("geomprop");
        if (geomPropInput && geomPropInput->getValue())
        {
            std::string prop = geomPropInput->getValue()->getValueString();
            REQUIRE(results.count(prop));
            REQUIRE(results[prop] == geomPropInput->getPath());
        }
        else
        {
            const mx::ShaderInput* indexIput = node->getInput("index");
            if (indexIput && indexIput->getValue())
            {
                std::string prop = indexIput->getValue()->getValueString();
                REQUIRE(results.count(prop));
                REQUIRE(results[prop] == indexIput->getPath());
            }
        }
    }
}

TEST_CASE("GenShader: Track Application Variables", "[genshader]")
{
    std::string testDocumentString = 
    "<?xml version=\"1.0\"?> \
      <materialx version=\"1.38\"> \
      <geompropvalue name=\"geompropvalue\" type=\"color3\" >  \
        <input name=\"geomprop\" type=\"string\" uniform=\"true\" nodename=\"constant\" /> \
      </geompropvalue> \
      <geompropvalue name=\"geompropvalue1\" type=\"color3\" > \
        <input name=\"geomprop\" type=\"string\" uniform=\"true\" value=\"primvar_one\" /> \
      </geompropvalue> \
      <geompropvalue name=\"geompropvalue2\" type=\"color3\" > \
        <input name=\"geomprop\" type=\"string\" uniform=\"true\" value=\"primvar_two\" /> \
      </geompropvalue> \
      <multiply name=\"multiply\" type=\"color3\" > \
        <input name=\"in1\" type=\"color3\" nodename=\"geompropvalue\" /> \
        <input name=\"in2\" type=\"color3\" nodename=\"geompropvalue1\" /> \
      </multiply> \
      <add name=\"add\" type=\"color3\"  > \
        <input name=\"in1\" type=\"color3\" nodename=\"multiply\" /> \
        <input name=\"in2\" type=\"color3\" nodename=\"geompropvalue2\" /> \
      </add> \
      <standard_surface name=\"standard_surface\" type=\"surfaceshader\" > \
        <input name=\"base_color\" type=\"color3\" nodename=\"add\" /> \
      </standard_surface> \
      <constant name=\"constant\" type=\"string\" > \
        <input name=\"value\" type=\"string\" uniform=\"true\" value=\"upstream_primvar\" /> \
      </constant> \
      <surfacematerial name=\"surfacematerial\" type=\"material\" > \
        <input name=\"surfaceshader\" type=\"surfaceshader\" nodename=\"standard_surface\" /> \
      </surfacematerial> \
    </materialx>";

    const mx::string testElement = "surfacematerial";

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::DocumentPtr testDoc = mx::createDocument();
    mx::readFromXmlString(testDoc, testDocumentString);
    testDoc->setDataLibrary(libraries);

    mx::ElementPtr element = testDoc->getChild(testElement);
    CHECK(element);

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        context.setApplicationVariableHandler(variableTracker);
        mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        context.setApplicationVariableHandler(variableTracker);
        mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        context.setApplicationVariableHandler(variableTracker);
        mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_SLANG
    {
        mx::GenContext context(mx::SlangShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        context.setApplicationVariableHandler(variableTracker);
        mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
    }
#endif
}

// Check that the 'implname' attributes declared on an implementation element are
// respected when naming the variables of the corresponding node in generated code.
// The standard libraries declare that the 'default' input of the <image> node is
// called 'default_value' in the implementations of all our source code targets.
void checkImplementationNames(mx::DocumentPtr libraries, mx::GenContext& context)
{
    const std::string testDocumentString =
    "<?xml version=\"1.0\"?> \
      <materialx version=\"1.39\"> \
      <image name=\"image1\" type=\"color3\" > \
        <input name=\"default\" type=\"color3\" value=\"0.1, 0.2, 0.3\" /> \
      </image> \
      <standard_surface name=\"standard_surface1\" type=\"surfaceshader\" > \
        <input name=\"base_color\" type=\"color3\" nodename=\"image1\" /> \
      </standard_surface> \
      <surfacematerial name=\"surfacematerial1\" type=\"material\" > \
        <input name=\"surfaceshader\" type=\"surfaceshader\" nodename=\"standard_surface1\" /> \
      </surfacematerial> \
    </materialx>";

    const mx::string testElement = "surfacematerial1";

    mx::DocumentPtr testDoc = mx::createDocument();
    mx::readFromXmlString(testDoc, testDocumentString);
    testDoc->setDataLibrary(libraries);

    mx::ElementPtr element = testDoc->getChild(testElement);
    REQUIRE(element);

    mx::ShaderPtr shader = context.getShaderGenerator().generate(testElement, element, context);
    REQUIRE(shader);

    mx::ShaderNode* imageNode = nullptr;
    for (mx::ShaderNode* node : shader->getGraph().getNodes())
    {
        if (node->getName() == "image1")
        {
            imageNode = node;
        }
    }
    REQUIRE(imageNode);

    // The remapped input is named by its 'implname', while an input with no
    // 'implname' keeps the name given in the nodedef.
    CHECK(imageNode->getPortName("default") == "default_value");
    CHECK(imageNode->getPortName("texcoord") == "texcoord");

    mx::ShaderInput* defaultInput = imageNode->getInput("default");
    REQUIRE(defaultInput);
    CHECK(defaultInput->getVariable() == "image1_default_value");

    mx::ShaderInput* texcoordInput = imageNode->getInput("texcoord");
    REQUIRE(texcoordInput);
    CHECK(texcoordInput->getVariable() == "image1_texcoord");
}

TEST_CASE("GenShader: Implementation Names", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkImplementationNames(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkImplementationNames(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkImplementationNames(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MSL
    {
        mx::GenContext context(mx::MslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkImplementationNames(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_SLANG
    {
        mx::GenContext context(mx::SlangShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkImplementationNames(libraries, context);
    }
#endif
}

// Create a shader graph directly from a nodedef, which is the path used by generators
// that emit node graphs rather than source code.
mx::ShaderGraphPtr createNodeDefGraph(mx::DocumentPtr libraries, const std::string& nodeDefName, mx::GenContext& context)
{
    mx::NodeDefPtr nodeDef = libraries->getNodeDef(nodeDefName);
    REQUIRE(nodeDef);

    mx::ShaderGraphPtr graph = mx::ShaderGraph::create(nullptr, nodeDefName, nodeDef, context);
    REQUIRE(graph);
    return graph;
}

// Check that a graph created from a nodedef publishes the interface of that nodedef,
// with every input and output socket wired to the matching port on the node that the
// nodedef declares.
void checkNodeDefInterface(mx::DocumentPtr libraries, mx::GenContext& context)
{
    // These nodedefs cover float, vector, filename, enum and uniform inputs, as well as
    // single and multi-output nodes, and a nodedef implemented by a nodegraph.
    const mx::StringVec nodeDefNames =
    {
        "ND_add_float",
        "ND_image_float",
        "ND_dielectric_bsdf",
        "ND_generalized_schlick_bsdf",
        "ND_separate3_vector3",
        "ND_UsdPreviewSurface_surfaceshader"
    };

    const std::string& target = context.getShaderGenerator().getTarget();
    for (const std::string& nodeDefName : nodeDefNames)
    {
        INFO(target + ": " + nodeDefName);

        mx::NodeDefPtr nodeDef = libraries->getNodeDef(nodeDefName);
        REQUIRE(nodeDef);

        mx::ShaderGraphPtr graph = createNodeDefGraph(libraries, nodeDefName, context);

        // The graph wraps the single node declared by the nodedef.
        REQUIRE(graph->getNodes().size() == 1);
        mx::ShaderNode* node = graph->getNodes().front();

        // Every input on the nodedef is published as an input socket, is bound to the
        // matching input on the node, and agrees with it on type and uniformity. The
        // sockets and the node ports are built by separate traversals of the nodedef,
        // so a divergence would give a type mismatched connection rather than an error.
        // Hardware generators publish inputs of their own as well, such as the uv scale
        // and offset of a texture node, so the nodedef's inputs are a subset here.
        CHECK(graph->numInputSockets() >= nodeDef->getActiveInputs().size());
        for (const mx::InputPtr& nodeDefInput : nodeDef->getActiveInputs())
        {
            INFO(target + ": " + nodeDefName + "." + nodeDefInput->getName());

            mx::ShaderGraphInputSocket* inputSocket = graph->getInputSocket(nodeDefInput->getName());
            REQUIRE(inputSocket);
            mx::ShaderInput* input = node->getInput(nodeDefInput->getName());
            REQUIRE(input);

            CHECK(input->getConnection() == inputSocket);
            CHECK(input->isBindInput());
            CHECK(inputSocket->getType() == input->getType());
            CHECK(inputSocket->isUniform() == input->isUniform());
        }

        // Every output on the nodedef is published as an output socket, connected to the
        // node output of the same name.
        CHECK(graph->numOutputSockets() == nodeDef->getActiveOutputs().size());
        for (const mx::OutputPtr& nodeDefOutput : nodeDef->getActiveOutputs())
        {
            INFO(target + ": " + nodeDefName + "." + nodeDefOutput->getName());

            mx::ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket(nodeDefOutput->getName());
            REQUIRE(outputSocket);
            mx::ShaderOutput* output = node->getOutput(nodeDefOutput->getName());
            REQUIRE(output);

            CHECK(outputSocket->getConnection() == output);
            CHECK(outputSocket->getType() == output->getType());
        }
    }
}

// Check that generating a shader from a nodedef emits code, with the interface of the
// nodedef published on the graph of the generated shader.
void checkNodeDefGeneration(mx::DocumentPtr libraries, mx::GenContext& context)
{
    // A nodedef implemented by source code, and one implemented by a nodegraph, which is
    // wrapped in a compound node.
    const mx::StringVec nodeDefNames =
    {
        "ND_add_float",
        "ND_UsdPreviewSurface_surfaceshader"
    };

    const std::string& target = context.getShaderGenerator().getTarget();
    for (const std::string& nodeDefName : nodeDefNames)
    {
        INFO(target + ": " + nodeDefName);

        mx::NodeDefPtr nodeDef = libraries->getNodeDef(nodeDefName);
        REQUIRE(nodeDef);

        mx::ShaderPtr shader = context.getShaderGenerator().generate(nodeDefName, nodeDef, context);
        REQUIRE(shader);

        mx::ShaderGraph& graph = shader->getGraph();
        CHECK(graph.numInputSockets() >= nodeDef->getActiveInputs().size());
        CHECK(graph.numOutputSockets() == nodeDef->getActiveOutputs().size());

        // The published interface has to reach the emitted shader, which is the contract
        // that generating from a nodedef exists to provide.
        const std::string& sourceCode = shader->getSourceCode();
        REQUIRE(!sourceCode.empty());
        for (size_t i = 0; i < graph.numInputSockets(); ++i)
        {
            const mx::ShaderGraphInputSocket* inputSocket = graph.getInputSocket(i);
            INFO(target + ": " + nodeDefName + "." + inputSocket->getName());
            CHECK(sourceCode.find(inputSocket->getVariable()) != std::string::npos);
        }
    }
}

// Check that a nodedef whose only node is elided by the graph refactoring passes still
// generates a shader carrying the value that the elision pushed downstream.
void checkNodeDefNodeElision(mx::DocumentPtr libraries, mx::GenContext& context)
{
    // Constant nodes and filename typed dot nodes are bypassed when the graph is
    // finalized, leaving an empty graph whose output value comes from the elision.
    const mx::StringVec nodeDefNames =
    {
        "ND_constant_float",
        "ND_constant_color3",
        "ND_dot_filename"
    };

    const std::string& target = context.getShaderGenerator().getTarget();
    for (const std::string& nodeDefName : nodeDefNames)
    {
        INFO(target + ": " + nodeDefName);

        mx::NodeDefPtr nodeDef = libraries->getNodeDef(nodeDefName);
        REQUIRE(nodeDef);

        mx::ShaderGraphPtr graph = createNodeDefGraph(libraries, nodeDefName, context);

        // The node really is gone, but the interface survives and the output socket still
        // resolves to a source.
        CHECK(graph->getNodes().empty());
        CHECK(graph->numInputSockets() >= 1);
        REQUIRE(graph->numOutputSockets() == 1);
        mx::ShaderGraphOutputSocket* outputSocket = graph->getOutputSocket(0);
        REQUIRE(outputSocket);
        CHECK((outputSocket->getConnection() || outputSocket->getValue()));

        // The value that the elision pushed downstream has to reach the emitted shader.
        // Without this the elision would silently give a shader with nothing in it.
        mx::ShaderPtr shader = context.getShaderGenerator().generate(nodeDefName, nodeDef, context);
        REQUIRE(shader);
        const std::string& sourceCode = shader->getSourceCode();
        REQUIRE(!sourceCode.empty());
        CHECK(sourceCode.find(shader->getGraph().getInputSocket(0)->getVariable()) != std::string::npos);
    }
}

// Check that a nodedef with no implementation for the target reports an error, rather
// than producing an incomplete graph.
void checkNodeDefWithoutImplementation(mx::DocumentPtr libraries, mx::GenContext& context)
{
    mx::DocumentPtr testDoc = mx::createDocument();
    testDoc->setDataLibrary(libraries);

    mx::NodeDefPtr nodeDef = testDoc->addNodeDef("ND_no_such_implementation", "float", "no_such_implementation");
    nodeDef->addInput("in", "float");

    CHECK_THROWS_AS(mx::ShaderGraph::create(nullptr, "no_such_implementation", nodeDef, context),
                    mx::ExceptionShaderGenError);
}

void checkNodeDefShaderGeneration(mx::DocumentPtr libraries, mx::GenContext& context)
{
    checkNodeDefInterface(libraries, context);
    checkNodeDefGeneration(libraries, context);
    checkNodeDefNodeElision(libraries, context);
    checkNodeDefWithoutImplementation(libraries, context);
}

TEST_CASE("GenShader: NodeDef Shader Generation", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkNodeDefShaderGeneration(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkNodeDefShaderGeneration(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkNodeDefShaderGeneration(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MSL
    {
        mx::GenContext context(mx::MslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkNodeDefShaderGeneration(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_SLANG
    {
        mx::GenContext context(mx::SlangShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkNodeDefShaderGeneration(libraries, context);
    }
#endif
}

// Check that the metadata declared on a nodedef and its inputs is exported to the graph
// created from that nodedef, and to the published input sockets of that graph.
void checkNodeDefMetadata(mx::DocumentPtr libraries, mx::GenContext& context)
{
    // Metadata is only created when the context carries a populated metadata registry,
    // without which the metadata is simply null and any check on it passes for the
    // wrong reason.
    context.getShaderGenerator().registerShaderMetadata(libraries, context);

    mx::ShaderGraphPtr graph = createNodeDefGraph(libraries, "ND_add_float", context);
    REQUIRE(graph->getNodes().size() == 1);
    mx::ShaderNode* node = graph->getNodes().front();

    // The node picked up the metadata of the nodedef, and the graph shares it since the
    // graph represents that node.
    REQUIRE(node->getMetadata());
    CHECK(graph->getMetadata() == node->getMetadata());

    bool foundNodeMetadata = false;
    for (const mx::ShaderMetadata& metadata : *graph->getMetadata())
    {
        if (metadata.name == "nodedef_test_name")
        {
            REQUIRE(metadata.value);
            CHECK(metadata.value->getValueString() == "AddFloat");
            foundNodeMetadata = true;
        }
    }
    CHECK(foundNodeMetadata);

    // Input metadata reaches the published socket as well.
    mx::ShaderGraphInputSocket* inputSocket = graph->getInputSocket("in1");
    REQUIRE(inputSocket);
    REQUIRE(inputSocket->getMetadata());

    bool foundInputMetadata = false;
    for (const mx::ShaderMetadata& metadata : *inputSocket->getMetadata())
    {
        if (metadata.name == "nodedef_test_input_name")
        {
            REQUIRE(metadata.value);
            CHECK(metadata.value->getValueString() == "FirstAddend");
            foundInputMetadata = true;
        }
    }
    CHECK(foundInputMetadata);
}

TEST_CASE("GenShader: NodeDef Metadata", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    // Define custom attributes to be exported as shader metadata, and assign them on the
    // nodedef under test.
    mx::AttributeDefPtr adNodeName = libraries->addAttributeDef("AD_nodedef_test_name");
    adNodeName->setType("string");
    adNodeName->setAttrName("nodedef_test_name");
    adNodeName->setExportable(true);

    mx::AttributeDefPtr adInputName = libraries->addAttributeDef("AD_nodedef_test_input_name");
    adInputName->setType("string");
    adInputName->setAttrName("nodedef_test_input_name");
    adInputName->setExportable(true);

    mx::NodeDefPtr nodeDef = libraries->getNodeDef("ND_add_float");
    REQUIRE(nodeDef);
    nodeDef->setAttribute("nodedef_test_name", "AddFloat");

    mx::InputPtr in1 = nodeDef->getActiveInput("in1");
    REQUIRE(in1);
    in1->setAttribute("nodedef_test_input_name", "FirstAddend");

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context(mx::GlslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkNodeDefMetadata(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    {
        mx::GenContext context(mx::OslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkNodeDefMetadata(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MDL
    {
        mx::GenContext context(mx::MdlShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkNodeDefMetadata(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_MSL
    {
        mx::GenContext context(mx::MslShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkNodeDefMetadata(libraries, context);
    }
#endif
#ifdef MATERIALX_BUILD_GEN_SLANG
    {
        mx::GenContext context(mx::SlangShaderGenerator::create());
        context.registerSourceCodeSearchPath(searchPath);
        checkNodeDefMetadata(libraries, context);
    }
#endif
}
