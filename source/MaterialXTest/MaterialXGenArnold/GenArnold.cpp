//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/MaterialXGenOsl/GenOsl.h>
#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/XmlIo.h>

#include <MaterialXGenArnold/ArnoldShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>

namespace mx = MaterialX;

#ifdef SUPPORT_ARNOLD_CONTEXT_STRING
TEST_CASE("Arnold context semantic test", "[arnold_context]")
{
    const std::string testMaterial = {
    "<?xml version=\"1.0\"?>\n"
    "<materialx version=\"1.36\">\n"
    "<material name = \"test_material\">\n"
        "<shaderref name = \"simple_srf1\" node = \"noise\" context = \"surfaceshader\">\n"
        "<bindinput name = \"octaves\" type = \"int\" value = \"12\" />\n"
        "<bindinput name = \"distortion\" type = \"float\" value = \"6\" />\n"
        "<bindinput name = \"lacunarity\" type = \"float\" value = \"2\" />\n"
        "<bindinput name = \"amplitude\" type = \"float\" value = \"1\" />\n"
        "<bindinput name = \"color1\" type = \"color3\" value = \"1, 1, 0\" />\n"
        "<bindinput name = \"color2\" type = \"color3\" value = \"0.9, 0.1, 0\" />\n"
        "</shaderref>\n"
        "<shaderref name = \"simple_disp\" node = \"noise\" context = \"displacementshader\">\n"
        "<bindinput name = \"octaves\" type = \"int\" value = \"8\" />\n"
        "<bindinput name = \"distortion\" type = \"float\" value = \"3\" />\n"
        "<bindinput name = \"lacunarity\" type = \"float\" value = \"4\" />\n"
        "<bindinput name = \"amplitude\" type = \"float\" value = \"1\" />\n"
        "<bindinput name = \"color1\" type = \"color3\" value = \"0, 0, 0\" />\n"
        "<bindinput name = \"color2\" type = \"color3\" value = \"1, 1, 1\" />\n"
        "</shaderref>\n"
    "</material>\n"
     "</materialx>\n"
    };

    // Do the upgrade
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlString(doc, testMaterial);

    // Check that the "context" attribute was recognized and the appropriate
    // shader nodes created
    mx::NodePtr shaderNode= doc->getNode("simple_srf1");
    REQUIRE((shaderNode->getAttribute(mx::TypedElement::TYPE_ATTRIBUTE) == mx::SURFACE_SHADER_TYPE_STRING));
    shaderNode = doc->getNode("simple_disp");
    REQUIRE((shaderNode->getAttribute(mx::TypedElement::TYPE_ATTRIBUTE) == mx::DISPLACEMENT_SHADER_TYPE_STRING));

    mx::NodePtr materialNode = doc->getNode("test_material");
    mx::InputPtr input = materialNode->getInput(mx::DISPLACEMENT_SHADER_TYPE_STRING);
    REQUIRE((input && input->getAttribute(mx::PortElement::NODE_NAME_ATTRIBUTE) == "simple_disp"));
    REQUIRE((input && input->getAttribute(mx::TypedElement::TYPE_ATTRIBUTE) == mx::DISPLACEMENT_SHADER_TYPE_STRING));
}
#endif

TEST_CASE("GenShader: Arnold Implementation Check", "[genosl]")
{
    mx::GenContext context(mx::ArnoldShaderGenerator::create());

    mx::StringSet generatorSkipNodeTypes;
    generatorSkipNodeTypes.insert("light");
    mx::StringSet generatorSkipNodeDefs;

    GenShaderUtil::checkImplementations(context, generatorSkipNodeTypes, generatorSkipNodeDefs, 63);
}

TEST_CASE("GenShader: Arnold Unique Names", "[genosl]")
{
    mx::GenContext context(mx::ArnoldShaderGenerator::create());

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    context.registerSourceCodeSearchPath(searchPath);
    // Add path to find OSL include files
    context.registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

    GenShaderUtil::testUniqueNames(context, mx::Stage::PIXEL);
}

static void generateArnoldOslCode()
{
    const mx::FilePath testRootPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite");
    const mx::FilePath testRootPath2 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/StandardSurface");
    const mx::FilePath testRootPath3 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/UsdPreviewSurface");
    mx::FilePathVec testRootPaths;
    testRootPaths.push_back(testRootPath);
    testRootPaths.push_back(testRootPath2);
    testRootPaths.push_back(testRootPath3);
    const mx::FilePath libSearchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    mx::FileSearchPath srcSearchPath(libSearchPath.asString());
    srcSearchPath.append(libSearchPath / mx::FilePath("stdlib/osl"));
    srcSearchPath.append(mx::FilePath::getCurrentPath());
    const mx::FilePath logPath("genosl_arnold_generate_test.txt");

    bool writeShadersToDisk = false;
    OslShaderGeneratorTester tester(mx::ArnoldShaderGenerator::create(), testRootPaths, libSearchPath, srcSearchPath, logPath, writeShadersToDisk);

    const mx::GenOptions genOptions;
    mx::FilePath optionsFilePath = testRootPath / mx::FilePath("_options.mtlx");
    tester.validate(genOptions, optionsFilePath);
}

TEST_CASE("GenShader: Arnold Shader Generation", "[genosl]")
{
    generateArnoldOslCode();
}
