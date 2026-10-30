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

#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/OcioColorManagementSystem.h>
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

#include <algorithm>
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

#ifdef MATERIALX_BUILD_GEN_GLSL
namespace
{

// A material referencing a surface shader node named "surface".
const std::string SURFACE_MATERIAL = R"(
  <surfacematerial name="material" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="surface" />
  </surfacematerial>)";

using ColorTransformSet = std::set<std::pair<std::string, std::string>>;

// Return the (node, transform) pairs for all color transform nodes in a graph and its compounds.
ColorTransformSet collectColorTransforms(const mx::ShaderGraph& graph)
{
    ColorTransformSet transforms;
    for (const mx::ShaderNode* node : graph.getNodes())
    {
        if (mx::stringEndsWith(node->getName(), "_cm"))
        {
            transforms.insert({ node->getName(), node->getImplementation().getName() });
        }
        if (const mx::ShaderGraph* inner = node->getImplementation().getGraph())
        {
            for (const auto& transform : collectColorTransforms(*inner))
            {
                transforms.insert(transform);
            }
        }
    }
    return transforms;
}

// Return the number of function definitions whose names begin with the given prefix.
size_t functionCount(const std::string& code, const std::string& prefix)
{
    size_t count = 0;
    const std::string definition = "void " + prefix;
    for (size_t at = code.find(definition); at != std::string::npos; at = code.find(definition, at + 1))
    {
        ++count;
    }
    return count;
}

} // anonymous namespace

TEST_CASE("GenShader: Compound Filename Color Spaces", "[genshader]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
    mx::ColorManagementSystemPtr colorManagementSystem =
        mx::DefaultColorManagementSystem::create(shaderGenerator->getTarget());
    colorManagementSystem->loadLibrary(libraries);
    shaderGenerator->setColorManagementSystem(colorManagementSystem);

    mx::GenContext context(shaderGenerator);
    context.registerSourceCodeSearchPath(searchPath);
    context.getOptions().targetColorSpaceOverride = "lin_rec709";

    // Create a document with the given working color space and body.
    auto createTestDocument = [&](const std::string& workingSpace, const std::string& body)
    {
        const std::string colorSpaceAttribute = workingSpace.empty() ? "" : " colorspace=\"" + workingSpace + "\"";
        mx::DocumentPtr doc = mx::createDocument();
        mx::readFromXmlString(doc, "<?xml version=\"1.0\"?><materialx version=\"1.39\"" + colorSpaceAttribute + ">" +
                                   body + "</materialx>");
        doc->setDataLibrary(libraries);
        REQUIRE(doc->validate());
        return doc;
    };

    // Generate a shader from the element at the given path.
    auto generate = [&](mx::DocumentPtr doc, const std::string& elementPath)
    {
        mx::ElementPtr element = doc->getDescendant(elementPath);
        REQUIRE(element);
        mx::ShaderPtr shader;
        REQUIRE_NOTHROW(shader = shaderGenerator->generate(element->getName(), element, context));
        REQUIRE(shader);
        return shader;
    };

    SECTION("Nested node graphs")
    {
        // The transform is applied once, within the inner node that reads the file (#2181).
        mx::ShaderPtr shader = generate(createTestDocument("", R"(
          <nodedef name="ND_test_image_inner" node="test_image_inner">
            <input name="tex" type="filename" uniform="true" value="" />
            <output name="output1" type="color3" />
          </nodedef>
          <nodegraph name="NG_test_image_inner" nodedef="ND_test_image_inner">
            <image name="read" type="color3">
              <input name="file" type="filename" interfacename="tex" />
            </image>
            <output name="output1" type="color3" nodename="read" />
          </nodegraph>
          <nodedef name="ND_test_image_outer" node="test_image_outer">
            <input name="file" type="filename" uniform="true" value="" />
            <output name="output1" type="color3" />
            <output name="output2" type="float" />
          </nodedef>
          <nodegraph name="NG_test_image_outer" nodedef="ND_test_image_outer">
            <test_image_inner name="inner" type="color3">
              <input name="tex" type="filename" interfacename="file" />
            </test_image_inner>
            <constant name="half" type="float">
              <input name="value" type="float" value="0.5" />
            </constant>
            <output name="output1" type="color3" nodename="inner" />
            <output name="output2" type="float" nodename="half" />
          </nodegraph>
          <test_image_outer name="outer" type="multioutput">
            <input name="file" type="filename" value="color.png" colorspace="srgb_texture" />
          </test_image_outer>
          <surface_unlit name="surface" type="surfaceshader">
            <input name="emission_color" type="color3" nodename="outer" output="output1" />
          </surface_unlit>)" + SURFACE_MATERIAL), "material");
        const ColorTransformSet expected = { { "read_out_cm", "NG_srgb_texture_to_lin_rec709_color3" } };
        CHECK(collectColorTransforms(shader->getGraph()) == expected);
    }

    SECTION("Multiple filename inputs")
    {
        // Each filename input of triplanarprojection is transformed at its own image node (#2182).
        for (int differing = 0; differing < 2; ++differing)
        {
            const std::string yzColorSpace = differing ? "acescg" : "srgb_texture";
            mx::ShaderPtr shader = generate(createTestDocument("lin_rec709", R"(
              <triplanarprojection name="triplanar" type="color3">
                <input name="filex" type="filename" value="" colorspace="srgb_texture" />
                <input name="filey" type="filename" value="" colorspace=")" + yzColorSpace + R"(" />
                <input name="filez" type="filename" value="" colorspace=")" + yzColorSpace + R"(" />
              </triplanarprojection>
              <surface_unlit name="surface" type="surfaceshader">
                <input name="emission_color" type="color3" nodename="triplanar" />
              </surface_unlit>)" + SURFACE_MATERIAL), "material");
            const std::string yzTransform = "NG_" + yzColorSpace + "_to_lin_rec709_color3";
            const ColorTransformSet expected = {
                { "N_imgX_color3_out_cm", "NG_srgb_texture_to_lin_rec709_color3" },
                { "N_imgY_color3_out_cm", yzTransform },
                { "N_imgZ_color3_out_cm", yzTransform },
            };
            CHECK(collectColorTransforms(shader->getGraph()) == expected);
        }
    }

    SECTION("Transform before downstream operations")
    {
        // The color space declared on a compound nodegraph input is applied ahead of a multiply (#2353).
        mx::DocumentPtr doc = createTestDocument("lin_rec709", R"(
          <nodegraph name="imageGraph">
            <input name="myFile" type="filename" value="color.png" colorspace="srgb_texture" />
            <input name="myScale" type="color3" value="0.5, 0.5, 0.5" />
            <image name="read" type="color3">
              <input name="file" type="filename" interfacename="myFile" />
            </image>
            <multiply name="scaled" type="color3">
              <input name="in1" type="color3" nodename="read" />
              <input name="in2" type="color3" interfacename="myScale" />
            </multiply>
            <output name="out" type="color3" nodename="scaled" />
          </nodegraph>
          <surface_unlit name="surface" type="surfaceshader">
            <input name="emission_color" type="color3" nodegraph="imageGraph" output="out" />
          </surface_unlit>)" + SURFACE_MATERIAL);
        const ColorTransformSet expected = { { "read_out_cm", "NG_srgb_texture_to_lin_rec709_color3" } };
        for (const char* elementPath : { "material", "imageGraph/out" })
        {
            mx::ShaderPtr shader = generate(doc, elementPath);
            CHECK(collectColorTransforms(shader->getGraph()) == expected);
        }
    }

    SECTION("Instances with different color spaces")
    {
        // Instances requiring different transforms each emit their own function.
        mx::DocumentPtr doc = createTestDocument("", R"(
          <nodedef name="ND_test_wrap" node="test_wrap">
            <input name="tex" type="filename" uniform="true" value="" />
            <output name="out" type="color3" />
          </nodedef>
          <nodegraph name="NG_test_wrap" nodedef="ND_test_wrap">
            <tiledimage name="inner" type="color3">
              <input name="file" type="filename" interfacename="tex" />
            </tiledimage>
            <output name="out" type="color3" nodename="inner" />
          </nodegraph>
          <test_wrap name="wrapA" type="color3">
            <input name="tex" type="filename" value="a.png" colorspace="srgb_texture" />
          </test_wrap>
          <test_wrap name="wrapB" type="color3">
            <input name="tex" type="filename" value="b.png" colorspace="g22_rec709" />
          </test_wrap>
          <surface_unlit name="surface" type="surfaceshader">
            <input name="emission_color" type="color3" nodename="wrapA" />
            <input name="transmission_color" type="color3" nodename="wrapB" />
          </surface_unlit>)" + SURFACE_MATERIAL);
        mx::ShaderPtr shader = generate(doc, "material");
        const std::string code = shader->getSourceCode(mx::Stage::PIXEL);
        CHECK(functionCount(code, "NG_test_wrap") == 2);
        CHECK(functionCount(code, "NG_tiledimage_color3") == 2);
        const ColorTransformSet expected = {
            { "N_img_color3_out_cm", "NG_srgb_texture_to_lin_rec709_color3" },
            { "N_img_color3_out_cm", "NG_g22_rec709_to_lin_rec709_color3" },
        };
        CHECK(collectColorTransforms(shader->getGraph()) == expected);

        // Generation from a node applies the color spaces declared on that node.
        shader = generate(doc, "wrapB");
        const ColorTransformSet expectedB = { { "N_img_color3_out_cm", "NG_g22_rec709_to_lin_rec709_color3" } };
        CHECK(collectColorTransforms(shader->getGraph()) == expectedB);
    }

    SECTION("Context reuse across working spaces")
    {
        // The target color space is taken from the document being generated, and distinguishes cached implementations.
        context.getOptions().targetColorSpaceOverride.clear();
        const std::string body = R"(
          <nodedef name="ND_test_tinted" node="test_tinted">
            <output name="out" type="color3" />
          </nodedef>
          <nodegraph name="NG_test_tinted" nodedef="ND_test_tinted">
            <constant name="tint" type="color3">
              <input name="value" type="color3" value="0.5, 0.5, 0.5" colorspace="srgb_texture" />
            </constant>
            <output name="out" type="color3" nodename="tint" />
          </nodegraph>
          <test_tinted name="tinted" type="color3" />
          <surface_unlit name="surface" type="surfaceshader">
            <input name="emission_color" type="color3" nodename="tinted" />
          </surface_unlit>)" + SURFACE_MATERIAL;
        const ColorTransformSet expected = { { "tint_value_cm", "NG_srgb_texture_to_lin_rec709_color3" } };
        for (int linearFirst = 0; linearFirst < 2; ++linearFirst)
        {
            context.clearNodeImplementations();
            mx::ShaderPtr first = generate(createTestDocument(linearFirst ? "lin_rec709" : "srgb_texture", body), "material");
            mx::ShaderPtr second = generate(createTestDocument(linearFirst ? "srgb_texture" : "lin_rec709", body), "material");
            const mx::Shader& linearShader = linearFirst ? *first : *second;
            const mx::Shader& srgbShader = linearFirst ? *second : *first;
            CHECK(collectColorTransforms(linearShader.getGraph()) == expected);
            CHECK(collectColorTransforms(srgbShader.getGraph()).empty());
        }
    }
}
#endif

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

TEST_CASE("GenShader: No-op Color Transforms", "[genshader]")
{
    // DefaultColorManagementSystem treats "none" and "data" as requiring no
    // color transformation, regardless of the source/target working space.
    mx::DefaultColorManagementSystemPtr colorManagementSystem =
        mx::DefaultColorManagementSystem::create("genglsl");
    CHECK(colorManagementSystem->isNoOpTransform("none", "lin_rec709_scene"));
    CHECK(colorManagementSystem->isNoOpTransform("data", "lin_rec709_scene"));
    CHECK(colorManagementSystem->isNoOpTransform("lin_rec709_scene", "none"));
    CHECK(colorManagementSystem->isNoOpTransform("lin_rec709_scene", "data"));
    CHECK(!colorManagementSystem->isNoOpTransform("srgb_texture", "lin_rec709_scene"));
    // Any unrecognized color space is not considered a no-op.
    CHECK(!colorManagementSystem->isNoOpTransform("Raw", "lin_rec709_scene"));

#ifdef MATERIALX_BUILD_OCIO
    // OcioColorManagementSystem inherits the "none"/"data" no-op behavior and the
    // legacy name equivalences from DefaultColorManagementSystem, and additionally
    // treats any OCIO color space flagged isData() (e.g. "Raw" in the ACES/studio
    // configs) as a no-op, even though that name means nothing to the default system.
    // It also recognizes names that resolve to the same color space in the active
    // config, such as two aliases of ACES2065-1 that are unknown to the default system.
    try
    {
        mx::OcioColorManagementSystemPtr ocioColorManagementSystem =
            mx::OcioColorManagementSystem::createFromBuiltinConfig("ocio://cg-config-latest", "genglsl");
        CHECK(ocioColorManagementSystem->isNoOpTransform("none", "lin_rec709_scene"));
        CHECK(ocioColorManagementSystem->isNoOpTransform("data", "lin_rec709_scene"));
        CHECK(ocioColorManagementSystem->isNoOpTransform("Raw", "lin_rec709_scene"));
        CHECK(ocioColorManagementSystem->isNoOpTransform("lin_rec709", "lin_rec709_scene"));
        CHECK(ocioColorManagementSystem->isNoOpTransform("ACES - ACES2065-1", "lin_ap0"));
        CHECK(!ocioColorManagementSystem->isNoOpTransform("ACEScg", "lin_rec709_scene"));
    }
    catch (const std::exception& e)
    {
        WARN(std::string("Could not create OcioColorManagementSystem from builtin config: ") + e.what());
    }
#endif

#ifdef MATERIALX_BUILD_GEN_GLSL
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::GenContext context(mx::GlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);
    colorManagementSystem = mx::DefaultColorManagementSystem::create(context.getShaderGenerator().getTarget());
    colorManagementSystem->loadLibrary(libraries);
    context.getShaderGenerator().setColorManagementSystem(colorManagementSystem);

    // A color3 tagged "data" is not the working color space, and has no
    // corresponding transform nodedef (e.g. no "data_to_lin_rec709_scene"
    // node exists). If isNoOpTransform() were not consulted before looking
    // up a transform, ShaderGraph::populateColorTransformMap would throw
    // ExceptionShaderGenError("Unsupported color space transform ...").
    const std::string noOpDocString = R"(<?xml version="1.0"?>
      <materialx version="1.39" colorspace="lin_rec709_scene">
        <constant name="data_constant" type="color3">
          <input name="value" type="color3" value="0.5, 0.5, 0.5" colorspace="data" />
        </constant>
        <surface_unlit name="data_surface" type="surfaceshader">
          <input name="emission_color" type="color3" nodename="data_constant" />
        </surface_unlit>
      </materialx>)";
    mx::DocumentPtr noOpDoc = mx::createDocument();
    mx::readFromXmlString(noOpDoc, noOpDocString);
    noOpDoc->setDataLibrary(libraries);
    mx::ElementPtr noOpElement = noOpDoc->getChild("data_surface");
    REQUIRE(noOpElement);
    REQUIRE_NOTHROW(context.getShaderGenerator().generate("data_surface", noOpElement, context));

    // Sanity check that the harness above is capable of catching a real
    // mismatch: an unrecognized, non-no-op color space with no transform
    // nodedef must still throw.
    const std::string unsupportedDocString = R"(<?xml version="1.0"?>
      <materialx version="1.39" colorspace="lin_rec709_scene">
        <constant name="bogus_constant" type="color3">
          <input name="value" type="color3" value="0.5, 0.5, 0.5" colorspace="bogus_colorspace" />
        </constant>
        <surface_unlit name="bogus_surface" type="surfaceshader">
          <input name="emission_color" type="color3" nodename="bogus_constant" />
        </surface_unlit>
      </materialx>)";
    mx::DocumentPtr unsupportedDoc = mx::createDocument();
    mx::readFromXmlString(unsupportedDoc, unsupportedDocString);
    unsupportedDoc->setDataLibrary(libraries);
    mx::ElementPtr unsupportedElement = unsupportedDoc->getChild("bogus_surface");
    REQUIRE(unsupportedElement);
    REQUIRE_THROWS_AS(context.getShaderGenerator().generate("bogus_surface", unsupportedElement, context),
                      mx::ExceptionShaderGenError);
#endif
}

TEST_CASE("GenShader: Equivalent Color Spaces", "[genshader]")
{
    // A legacy color space name and its color interop equivalent refer to the same
    // color space, so DefaultColorManagementSystem reports no transform between them.
    mx::DefaultColorManagementSystemPtr colorManagementSystem =
        mx::DefaultColorManagementSystem::create("genglsl");
    CHECK(colorManagementSystem->isNoOpTransform("lin_rec709", "lin_rec709"));
    CHECK(colorManagementSystem->isNoOpTransform("lin_rec709", "lin_rec709_scene"));
    CHECK(colorManagementSystem->isNoOpTransform("lin_rec709_scene", "lin_rec709"));
    CHECK(colorManagementSystem->isNoOpTransform("srgb_texture", "srgb_rec709_scene"));
    CHECK(colorManagementSystem->isNoOpTransform("acescg", "lin_ap1_scene"));
    CHECK(!colorManagementSystem->isNoOpTransform("lin_rec709", "acescg"));
    CHECK(!colorManagementSystem->isNoOpTransform("srgb_texture", "lin_rec709_scene"));
    CHECK(!colorManagementSystem->isNoOpTransform("lin_rec709", "bogus_colorspace"));

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    // A document authored in the legacy "lin_rec709" working space, generated for the
    // equivalent "lin_rec709_scene" target, must produce the same graph as a document
    // authored in the target space: no color transform nodes are inserted. The uniform
    // attenuation_color input of glTF PBR is included so that the MDL checks below can
    // confirm that its published value remains uniform.
    const std::string legacyDocString = R"(<?xml version="1.0"?>
      <materialx version="1.39" colorspace="lin_rec709">
        <gltf_pbr name="legacy_shader" type="surfaceshader">
          <input name="base_color" type="color3" value="0.5, 0.5, 0.5" />
          <input name="attenuation_color" type="color3" value="0, 0, 0" />
        </gltf_pbr>
        <surfacematerial name="legacy_material" type="material">
          <input name="surfaceshader" type="surfaceshader" nodename="legacy_shader" />
        </surfacematerial>
      </materialx>)";
    mx::DocumentPtr legacyDoc = mx::createDocument();
    mx::readFromXmlString(legacyDoc, legacyDocString);
    legacyDoc->setDataLibrary(libraries);
    mx::ElementPtr legacyElement = legacyDoc->getChild("legacy_material");
    REQUIRE(legacyElement);

    // When the source and target color spaces differ, a color transform node is inserted
    // between the published value and the shader input. The transform node's input must
    // carry the same uniform flag as the shader input that it feeds.
    const std::string transformDocString = R"(<?xml version="1.0"?>
      <materialx version="1.39" colorspace="lin_rec709_scene">
        <gltf_pbr name="transform_shader" type="surfaceshader">
          <input name="base_color" type="color3" value="0.5, 0.5, 0.5" colorspace="srgb_texture" />
          <input name="attenuation_color" type="color3" value="0, 0, 0" colorspace="srgb_texture" />
        </gltf_pbr>
        <surfacematerial name="transform_material" type="material">
          <input name="surfaceshader" type="surfaceshader" nodename="transform_shader" />
        </surfacematerial>
      </materialx>)";
    mx::DocumentPtr transformDoc = mx::createDocument();
    mx::readFromXmlString(transformDoc, transformDocString);
    transformDoc->setDataLibrary(libraries);
    mx::ElementPtr transformElement = transformDoc->getChild("transform_material");
    REQUIRE(transformElement);

    auto createContext = [&](mx::ShaderGeneratorPtr generator)
    {
        mx::GenContext context(generator);
        context.registerSourceCodeSearchPath(searchPath);
        mx::DefaultColorManagementSystemPtr cms = mx::DefaultColorManagementSystem::create(generator->getTarget());
        cms->loadLibrary(libraries);
        generator->setColorManagementSystem(cms);
        context.getOptions().targetColorSpaceOverride = "lin_rec709_scene";
        return context;
    };

#ifdef MATERIALX_BUILD_GEN_GLSL
    {
        mx::GenContext context = createContext(mx::GlslShaderGenerator::create());

        mx::ShaderPtr legacyShader = context.getShaderGenerator().generate("legacy_material", legacyElement, context);
        REQUIRE(legacyShader);
        for (const mx::ShaderNode* node : legacyShader->getGraph().getNodes())
        {
            CHECK(!mx::stringEndsWith(node->getName(), "_cm"));
        }

        mx::ShaderPtr transformShader = context.getShaderGenerator().generate("transform_material", transformElement, context);
        REQUIRE(transformShader);
        const mx::ShaderNode* baseColorTransform = transformShader->getGraph().getNode("transform_shader_base_color_cm");
        REQUIRE(baseColorTransform);
        CHECK(!baseColorTransform->getInput(0)->isUniform());
        const mx::ShaderNode* attenuationColorTransform = transformShader->getGraph().getNode("transform_shader_attenuation_color_cm");
        REQUIRE(attenuationColorTransform);
        CHECK(attenuationColorTransform->getInput(0)->isUniform());
    }
#endif

#ifdef MATERIALX_BUILD_GEN_MDL
    {
        // MDL distinguishes uniform and varying values, so the published value for a uniform
        // input must be declared uniform whether or not a color transform is applied to it.
        mx::GenContext context = createContext(mx::MdlShaderGenerator::create());

        mx::ShaderPtr legacyShader = context.getShaderGenerator().generate("legacy_material", legacyElement, context);
        REQUIRE(legacyShader);
        const std::string& legacyCode = legacyShader->getSourceCode();
        CHECK(legacyCode.find("_cm") == std::string::npos);
        CHECK(legacyCode.find("uniform color legacy_shader_attenuation_color =") != std::string::npos);

        mx::ShaderPtr transformShader = context.getShaderGenerator().generate("transform_material", transformElement, context);
        REQUIRE(transformShader);
        const std::string& transformCode = transformShader->getSourceCode();
        CHECK(transformCode.find("uniform color transform_shader_attenuation_color_cm_in =") != std::string::npos);
        CHECK(transformCode.find("uniform color transform_shader_base_color_cm_in =") == std::string::npos);
        CHECK(transformCode.find("color transform_shader_base_color_cm_in =") != std::string::npos);
    }
#endif
}

TEST_CASE("GenShader: User-Facing Color Space Names", "[genshader]")
{
    // DefaultColorManagementSystem translates both color interop forum IDs and their
    // deprecated legacy equivalents to the same user-facing display name.
    mx::DefaultColorManagementSystemPtr colorManagementSystem =
        mx::DefaultColorManagementSystem::create("genglsl");
    CHECK(colorManagementSystem->getUserFacingName("lin_rec709_scene") == "Linear Rec.709 (sRGB)");
    CHECK(colorManagementSystem->getUserFacingName("lin_rec709") == "Linear Rec.709 (sRGB)");
    CHECK(colorManagementSystem->getUserFacingName("lin_ap1_scene") == "ACEScg");
    CHECK(colorManagementSystem->getUserFacingName("acescg") == "ACEScg");
    CHECK(colorManagementSystem->getUserFacingName("lin_ap0_scene") == "ACES2065-1");
    CHECK(colorManagementSystem->getUserFacingName("none") == "Data");
    CHECK(colorManagementSystem->getUserFacingName("data") == "Data");
    // An unrecognized color space is returned unchanged.
    CHECK(colorManagementSystem->getUserFacingName("bogus_colorspace") == "bogus_colorspace");

#ifdef MATERIALX_BUILD_OCIO
    try
    {
        mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
        mx::FilePath minimalConfigFile = searchPath.find(
            "resources/Materials/TestSuite/stdlib/color_management/minimal_config.ocio");
        REQUIRE(minimalConfigFile.exists());

        mx::OcioColorManagementSystemPtr ocioColorManagementSystem =
            mx::OcioColorManagementSystem::createFromFile(minimalConfigFile.asString(), "genglsl");

        // The OcioColorManagementSystem checks its own config first. This color space is defined
        // there but not in the built-in cg-config or the DefaultColorManagementSystem.
        CHECK(ocioColorManagementSystem->getUserFacingName("ocio:arrilogc4_awg4_scene") == "ARRI camera LogC4");

        // This color space is known to both its own config and the DefaultColorManagementSystem
        // but the config author's name gets first preference.
        CHECK(ocioColorManagementSystem->getUserFacingName("srgb_rec709_scene") == "sRGB (Scene-referred)");

        // This color space is not in its config, so the fallback will come from the DefaultColorManagementSystem.
        CHECK(ocioColorManagementSystem->getUserFacingName("lin_rec709_scene") == "Linear Rec.709 (sRGB)");

        // In this case, the color space string is not in its own config or the DefaultColorManagementSystem,
        // so the IdentifyBuiltinColorSpace fallback in getSupportedColorSpaceName is invoked. This will find
        // a color space for the string in the built-in cg-config. The transform math is then compared against
        // the color spaces in the minimal_config.ocio, where it finds a match.
        CHECK(ocioColorManagementSystem->getUserFacingName("srgb_encoded_ap1_tx") == "sRGB AP1");

        // A totally unrecognized color space name is returned unmodified.
        CHECK(ocioColorManagementSystem->getUserFacingName("bogus_colorspace") == "bogus_colorspace");
    }
    catch (const std::exception& e)
    {
        WARN(std::string("Could not create OcioColorManagementSystem from minimal_config.ocio: ") + e.what());
    }
#endif
}
