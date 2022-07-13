//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <MaterialXGenShader/HwShaderGenerator.h>
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
    mx::DocumentPtr doc = mx::createDocument();

    mx::FileSearchPath searchPath;
    searchPath.append(mx::FilePath::getCurrentPath());
    loadLibraries({ "libraries/targets", "libraries/stdlib", "libraries/pbrlib" }, searchPath, doc);

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
    // Make sure the standard types are registered
    const mx::TypeDesc* floatType = mx::TypeDesc::get("float");
    REQUIRE(floatType != nullptr);
    REQUIRE(floatType->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    const mx::TypeDesc* integerType = mx::TypeDesc::get("integer");
    REQUIRE(integerType != nullptr);
    REQUIRE(integerType->getBaseType() == mx::TypeDesc::BASETYPE_INTEGER);
    const mx::TypeDesc* booleanType = mx::TypeDesc::get("boolean");
    REQUIRE(booleanType != nullptr);
    REQUIRE(booleanType->getBaseType() == mx::TypeDesc::BASETYPE_BOOLEAN);
    const mx::TypeDesc* color3Type = mx::TypeDesc::get("color3");
    REQUIRE(color3Type != nullptr);
    REQUIRE(color3Type->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color3Type->getSemantic() == mx::TypeDesc::SEMANTIC_COLOR);
    REQUIRE(color3Type->isFloat3());
    const mx::TypeDesc* color4Type = mx::TypeDesc::get("color4");
    REQUIRE(color4Type != nullptr);
    REQUIRE(color4Type->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color4Type->getSemantic() == mx::TypeDesc::SEMANTIC_COLOR);
    REQUIRE(color4Type->isFloat4());

    // Make sure we can register a new custom type
    const mx::TypeDesc* fooType = mx::TypeDesc::registerType("foo", mx::TypeDesc::BASETYPE_FLOAT, mx::TypeDesc::SEMANTIC_COLOR, 5);
    REQUIRE(fooType != nullptr);

    // Make sure we can't use a name that is already taken
    REQUIRE_THROWS(mx::TypeDesc::registerType("color3", mx::TypeDesc::BASETYPE_FLOAT));

    // Make sure we can't request an unknown type
    REQUIRE(mx::TypeDesc::get("bar") == nullptr);
}

TEST_CASE("GenShader: Graph + Nodedf Transparent Check", "[genshader]")
{
    mx::FileSearchPath searchPath;
    const mx::FilePath currentPath = mx::FilePath::getCurrentPath();
    searchPath.append(currentPath);
    searchPath.append(currentPath / mx::FilePath("resources/Materials/TestSuite"));

    mx::DocumentPtr doc = mx::createDocument();
    loadLibraries({ "libraries/targets", "libraries/stdlib", "libraries/pbrlib", "libraries/bxdf" }, searchPath, doc);
    mx::FilePath testPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/pbrlib/surfaceshader/transparency_test.mtlx");
    mx::readFromXmlFile(doc, testPath, searchPath);

    // Test against nodegraphs using surface shaders
    std::vector<mx::TypedElementPtr> testElements;
    mx::findRenderableElements(doc, testElements);
    std::string failedElements;
    std::set<mx::NodeGraphPtr> testGraphs;
    for (auto testElement : testElements)
    {
        if (testElement->isA<mx::Output>())
        {
            const mx::ElementPtr testParent = testElement->getParent();            
            mx::NodeGraphPtr graph = testParent ? testParent->asA<mx::NodeGraph>() : nullptr;
            if (graph)
            {
                testGraphs.insert(graph);
            }
        }
        if (!mx::isTransparentSurface(testElement))
        {
            failedElements += testElement->getNamePath() + " ";
        }
    }
    REQUIRE(failedElements == mx::EMPTY_STRING);

    // Create nodedefs from nodegraphs to test against instances using
    // definitions which have surface shaders.
    unsigned int counter = 0;
    for (auto testGraph : testGraphs)
    {
        std::string counterString = std::to_string(counter);
        std::string newCategory = doc->createValidChildName("transptype_" + counterString);
        const std::string graphName = testGraph->getName() + counterString;
        std::string newNodeDefName = doc->createValidChildName("ND_" + graphName);
        std::string newGraphName = doc->createValidChildName("NG_" + graphName);
        mx::NodeDefPtr nodeDef = doc->addNodeDefFromGraph(testGraph, newNodeDefName, newCategory, "1.0", true, mx::EMPTY_STRING, newGraphName);
        if (nodeDef)
        {
            mx::NodePtr newInstance = doc->addNode(newCategory, mx::EMPTY_STRING, nodeDef->getType());
            for (auto valueElement : nodeDef->getActiveValueElements())
            {
                mx::ValueElementPtr newValueElem = newInstance->addChildOfCategory(valueElement->getCategory(), valueElement->getName())->asA<mx::ValueElement>();
                newValueElem->setValue(valueElement->getValueString(), valueElement->getType());
            }
            bool defInstanceIsTransp = mx::isTransparentSurface(newInstance);
            CHECK(defInstanceIsTransp);
        }
    }
    mx::writeToXmlFile(doc, "transparency_test_nodedefs.mtlx");
}

TEST_CASE("GenShader: Shader Translation", "[translate]")
{
    const mx::FilePath currentPath = mx::FilePath::getCurrentPath();
    mx::FileSearchPath searchPath(currentPath);
    mx::ShaderTranslatorPtr shaderTranslator = mx::ShaderTranslator::create();

    mx::FilePath testPath = currentPath / mx::FilePath("resources/Materials/Examples/StandardSurface");
    for (mx::FilePath& mtlxFile : testPath.getFilesInDirectory(mx::MTLX_EXTENSION))
    {
        mx::DocumentPtr doc = mx::createDocument();
        loadLibraries({ "libraries/targets", "libraries/stdlib", "libraries/pbrlib", "libraries/bxdf" }, searchPath, doc);

        mx::readFromXmlFile(doc, testPath / mtlxFile, searchPath);
        mtlxFile.removeExtension();

        try
        {
            shaderTranslator->translateAllMaterials(doc, "UsdPreviewSurface");
        }
        catch (mx::Exception &e)
        {
            std::cout << "Failed translating: " << (testPath / mtlxFile).asString() << ": " << e.what() << std::endl;
        }

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
    mx::DocumentPtr libraries = mx::createDocument();
    mx::FilePath currentPath = mx::FilePath::getCurrentPath();
    mx::FileSearchPath searchPath(currentPath);
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    const mx::FilePath resourcePath(currentPath / "resources");
    mx::StringVec failedTests;
    mx::FilePathVec testFiles = { 
        "Materials/Examples/StandardSurface/standard_surface_default.mtlx", 
        "Materials/Examples/StandardSurface/standard_surface_glass.mtlx",
        "Materials/TestSuite/libraries/metal/brass_wire_mesh.mtlx",
        "Materials/TestSuite/pbrlib/surfaceshader/transparency_nodedef_test.mtlx",
        "Materials/TestSuite/pbrlib/surfaceshader/transparency_test.mtlx",
    };
    std::vector<bool> transparencyTest = { false, true, true, true, true };
    for (size_t i=0; i<testFiles.size(); i++)
    {
        const mx::FilePath& testFile = resourcePath / testFiles[i];
        bool testValue = transparencyTest[i];

        mx::DocumentPtr testDoc = mx::createDocument();
        testDoc->importLibrary(libraries);

        try
        {
            mx::readFromXmlFile(testDoc, testFile, searchPath);
            std::vector<mx::TypedElementPtr> renderables;
            mx::findRenderableElements(testDoc, renderables);
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
    const mx::FilePath testFile = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/StandardSurface/standard_surface_marble_solid.mtlx");
    const mx::string testElement = "SR_marble1";

    const size_t numRuns = 10;
    mx::vector<mx::DocumentPtr> testDocs(numRuns);
    mx::StringVec sourceCode(numRuns);

    for (size_t i = 0; i < numRuns; ++i)
    {
        mx::DocumentPtr testDoc = mx::createDocument();
        mx::readFromXmlFile(testDoc, testFile);
        testDoc->importLibrary(libraries);

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
    mx::DocumentPtr libraries = mx::createDocument();
    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath());
    mx::loadLibraries({ "libraries/targets", "libraries/stdlib", "libraries/pbrlib", "libraries/bxdf" }, searchPath, libraries);

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
}
