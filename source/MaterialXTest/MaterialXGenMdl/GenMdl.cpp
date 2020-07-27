//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/MaterialXGenMdl/GenMdl.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>

#include <MaterialXGenMdl/MdlShaderGenerator.h>
#include <MaterialXGenMdl/MdlSyntax.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Util.h>


namespace mx = MaterialX;

TEST_CASE("GenShader: MDL Syntax", "[genmdl]")
{
    mx::SyntaxPtr syntax = mx::MdlSyntax::create();

    REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "float");
    REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "color");
    REQUIRE(syntax->getTypeName(mx::Type::VECTOR3) == "float3");
    REQUIRE(syntax->getTypeName(mx::Type::FLOATARRAY) == "float");
    REQUIRE(syntax->getTypeName(mx::Type::INTEGERARRAY) == "int");
    REQUIRE(mx::Type::FLOATARRAY->isArray());
    REQUIRE(mx::Type::INTEGERARRAY->isArray());

    REQUIRE(syntax->getTypeName(mx::Type::BSDF) == "material");
    REQUIRE(syntax->getOutputTypeName(mx::Type::BSDF) == "material");

    // Set fixed precision with one digit
    mx::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

    std::string value;
    value = syntax->getDefaultValue(mx::Type::FLOAT);
    REQUIRE(value == "0.0");
    value = syntax->getDefaultValue(mx::Type::COLOR3);
    REQUIRE(value == "color(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR3, true);
    REQUIRE(value == "color(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4);
    REQUIRE(value == "mk_color4(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4, true);
    REQUIRE(value == "mk_color4(0.0)");
    value = syntax->getDefaultValue(mx::Type::FLOATARRAY, true);
    REQUIRE(value.empty());
    value = syntax->getDefaultValue(mx::Type::INTEGERARRAY, true);
    REQUIRE(value.empty());

    mx::ValuePtr floatValue = mx::Value::createValue<float>(42.0f);
    value = syntax->getValue(mx::Type::FLOAT, *floatValue);
    REQUIRE(value == "42.0");
    value = syntax->getValue(mx::Type::FLOAT, *floatValue, true);
    REQUIRE(value == "42.0");

    mx::ValuePtr color3Value = mx::Value::createValue<mx::Color3>(mx::Color3(1.0f, 2.0f, 3.0f));
    value = syntax->getValue(mx::Type::COLOR3, *color3Value);
    REQUIRE(value == "color(1.0, 2.0, 3.0)");
    value = syntax->getValue(mx::Type::COLOR3, *color3Value, true);
    REQUIRE(value == "color(1.0, 2.0, 3.0)");

    mx::ValuePtr color4Value = mx::Value::createValue<mx::Color4>(mx::Color4(1.0f, 2.0f, 3.0f, 4.0f));
    value = syntax->getValue(mx::Type::COLOR4, *color4Value);
    REQUIRE(value == "mk_color4(1.0, 2.0, 3.0, 4.0)");
    value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
    REQUIRE(value == "mk_color4(1.0, 2.0, 3.0, 4.0)");

    std::vector<float> floatArray = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f };
    mx::ValuePtr floatArrayValue = mx::Value::createValue<std::vector<float>>(floatArray);
    value = syntax->getValue(mx::Type::FLOATARRAY, *floatArrayValue);
    REQUIRE(value == "float[](0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7)");

    std::vector<int> intArray = { 1, 2, 3, 4, 5, 6, 7 };
    mx::ValuePtr intArrayValue = mx::Value::createValue<std::vector<int>>(intArray);
    value = syntax->getValue(mx::Type::INTEGERARRAY, *intArrayValue);
    REQUIRE(value == "int[](1, 2, 3, 4, 5, 6, 7)");
}

/*
TEST_CASE("GenShader: MDL Implementation Check", "[genmdl]")
{
    mx::GenContext context(mx::MdlShaderGenerator::create());

    mx::StringSet generatorSkipNodeTypes;
    generatorSkipNodeTypes.insert("light");
    mx::StringSet generatorSkipNodeDefs;

    GenShaderUtil::checkImplementations(context, generatorSkipNodeTypes, generatorSkipNodeDefs, 65);
}


TEST_CASE("GenShader: MDL Unique Names", "[genmdl]")
{
    mx::GenContext context(mx::MdlShaderGenerator::create());

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    context.registerSourceCodeSearchPath(searchPath);
    // TODO: Add path to find MDL module files
    // context.registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

    GenShaderUtil::testUniqueNames(context, mx::Stage::PIXEL);
}
*/

void MdlShaderGeneratorTester::compileSource(const std::vector<mx::FilePath>& sourceCodePaths)
{
    if (sourceCodePaths.empty() || sourceCodePaths[0].isEmpty())
        return;
    
    std::string mdlcExec(MATERIALX_MDLC_EXECUTABLE);
    if (mdlcExec.empty())
    {
        return;
    }

    mx::FilePath moduleToTestPath = sourceCodePaths[0].getParentPath();
    mx::FilePath module = sourceCodePaths[0];
    std::string moduleToTest = module[module.size()-1];
    moduleToTest = moduleToTest.substr(0, moduleToTest.size() - sourceCodePaths[0].getExtension().length() - 1);

    mx::StringVec extraModulePaths = mx::splitString(MATERIALX_MDL_MODULE_PATHS, ",");

    std::string mdlcCommand = mdlcExec;
    for (const std::string& extraPath : extraModulePaths)
    {
        mdlcCommand += " -p\"" + extraPath + "\"";
    }
    // Note: These paths are based on
    mx::FilePath currentPath = mx::FilePath::getCurrentPath();
    mx::FilePath coreModulePath = currentPath / std::string(MATERIALX_INSTALL_MDL_MODULE_PATH) / "mdl";
    mx::FilePath coreModulePath2 = coreModulePath / mx::FilePath("materialx");
    mdlcCommand += " -p \"" + currentPath.asString() + "\"";
    mdlcCommand += " -p \"" + coreModulePath.asString() + "\"";
    mdlcCommand += " -p \"" + coreModulePath2.asString() + "\"";
    mdlcCommand += " -p \"" + moduleToTestPath.asString() + "\"";
    mdlcCommand += " -p \"" + moduleToTestPath.getParentPath().asString() + "\"";
    mdlcCommand += " -p \"" + _libSearchPath.asString() + "\"";
    mdlcCommand += " -W \"181=off\" -W \"183=off\"  -W \"225=off\"";
    mdlcCommand += " " + moduleToTest;
    mx::FilePath errorFile = moduleToTestPath / (moduleToTest + ".mdl_errors.txt");
    mdlcCommand += " > " + errorFile.asString() + " 2>&1";

    int returnValue = std::system(mdlcCommand.c_str());
    std::ifstream errorStream(errorFile);
    mx::StringVec result;
    std::string line;
    bool writeErrorCode = false;
    while (std::getline(errorStream, line))
    {
        if (!writeErrorCode)
        {
            _logFile << mdlcCommand << std::endl;
            _logFile << "\tReturn code: " << std::to_string(returnValue) << std::endl;
            writeErrorCode = true;
        }
        _logFile << "\tError: " << line << std::endl;
    }
}

TEST_CASE("GenShader: MDL Shader Generation", "[genmdl]")
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
    srcSearchPath.append(libSearchPath / mx::FilePath("stdlib/genmdl"));

    const mx::FilePath logPath("genmdl_mdl_generate_test.txt");

    // Write shaders and try to compile only if mdlc exe specified.
    std::string mdlcExec(MATERIALX_MDLC_EXECUTABLE);
    bool writeShadersToDisk = !mdlcExec.empty();
    MdlShaderGeneratorTester tester(mx::MdlShaderGenerator::create(), testRootPaths, libSearchPath, srcSearchPath, logPath, writeShadersToDisk);
    tester.addSkipLibraryFiles();

    mx::GenOptions genOptions;
    genOptions.targetColorSpaceOverride = "lin_rec709";
    mx::FilePath optionsFilePath = testRootPath / mx::FilePath("_options.mtlx");
    tester.validate(genOptions, optionsFilePath);
}
