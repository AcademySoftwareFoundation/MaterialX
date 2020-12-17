//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/MaterialXGenOsl/GenOsl.h>
#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

#include <MaterialXFormat/File.h>

#include <MaterialXGenArnold/ArnoldShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>

namespace mx = MaterialX;

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
