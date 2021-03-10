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
    searchPath.append(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    loadLibraries({ "targets", "stdlib", "pbrlib" }, searchPath, doc);

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

TEST_CASE("GenShader: Translation Check", "[genshader]")
{
    mx::FileSearchPath searchPath;
    const mx::FilePath currentPath = mx::FilePath::getCurrentPath();
    searchPath.append(currentPath / mx::FilePath("libraries"));
    searchPath.append(currentPath / mx::FilePath("resources/Materials/TestSuite"));

    mx::DocumentPtr doc = mx::createDocument();
    loadLibraries({ "targets", "stdlib", "pbrlib", "bxdf",  }, searchPath, doc);
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
    mx::FileSearchPath searchPath;
    const mx::FilePath currentPath = mx::FilePath::getCurrentPath();
    searchPath.append(currentPath / mx::FilePath("libraries"));
    searchPath.append(currentPath / mx::FilePath("resources/Materials/TestSuite"));

    mx::ShaderTranslatorPtr shaderTranslator = mx::ShaderTranslator::create();

    const std::string USD_PREVIEW_SURFACE_NAME("UsdPreviewSurface");

    mx::FilePath testPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/StandardSurface");
    for (mx::FilePath& mtlxFile : testPath.getFilesInDirectory("mtlx"))
    {
        mx::DocumentPtr doc = mx::createDocument();
        loadLibraries({ "targets", "stdlib", "pbrlib", "bxdf", "translation" }, searchPath, doc);

        mx::readFromXmlFile(doc, testPath / mtlxFile, searchPath);
        mtlxFile.removeExtension();
        mx::writeToXmlFile(doc, mtlxFile.asString() + "_untranslated.mtlx");

        try {
            shaderTranslator->translateAllMaterials(doc, USD_PREVIEW_SURFACE_NAME);
        }
        catch (mx::Exception &e)
        {
            std::cout << "Failed translating: " << (testPath / mtlxFile).asString() << ": " << e.what() << std::endl;
        }

        mx::writeToXmlFile(doc, mtlxFile.asString() + "_translated.mtlx");
        std::string validationErrors;
        bool valid = doc->validate(&validationErrors);
        std::cout << "Shader translation of : " << (testPath / mtlxFile).asString() << (valid ?  ": passed"  : ": failed") << std::endl;
        if (!valid)
        {
            std::cout << "Validation errors: " << validationErrors << std::endl;
        }
        CHECK(valid);
    }
}

