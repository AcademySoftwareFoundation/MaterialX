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


TEST_CASE("GenShader: MDL Implementation Check", "[genmdl]")
{
    mx::GenContext context(mx::MdlShaderGenerator::create());

    mx::StringSet generatorSkipNodeTypes;
    generatorSkipNodeTypes.insert("light");
    mx::StringSet generatorSkipNodeDefs;

    GenShaderUtil::checkImplementations(context, generatorSkipNodeTypes, generatorSkipNodeDefs, 55);
}

void MdlShaderGeneratorTester::compileSource(const std::vector<mx::FilePath>& sourceCodePaths)
{
    if (sourceCodePaths.empty() || sourceCodePaths[0].isEmpty())
        return;
    
    mx::FilePath moduleToTestPath = sourceCodePaths[0].getParentPath();
    mx::FilePath module = sourceCodePaths[0];
    std::string moduleToTest = module[module.size()-1];
    moduleToTest = moduleToTest.substr(0, moduleToTest.size() - sourceCodePaths[0].getExtension().length() - 1);

    mx::StringVec extraModulePaths = mx::splitString(MATERIALX_MDL_MODULE_PATHS, ",");

    std::string renderExec(MATERIALX_MDL_RENDER_EXECUTABLE);
    bool testMDLC = renderExec.empty();
    if (testMDLC)
    {
        std::string mdlcExec(MATERIALX_MDLC_EXECUTABLE);
        if (mdlcExec.empty())
        {
            return;
        }

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
        mx::FilePath errorFile = moduleToTestPath / (moduleToTest + ".mdl_compile_errors.txt");
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

        CHECK(returnValue == 0);
    }
    else
    {
        std::string renderCommand = renderExec;
        for (const std::string& extraPath : extraModulePaths)
        {
            renderCommand += " --mdl_path\"" + extraPath + "\"";
        }

        // Note: These paths are based on
        mx::FilePath currentPath = mx::FilePath::getCurrentPath();
        mx::FilePath coreModulePath = currentPath / std::string(MATERIALX_INSTALL_MDL_MODULE_PATH) / "mdl";
        mx::FilePath coreModulePath2 = coreModulePath / mx::FilePath("materialx");
        renderCommand += " --mdl_path \"" + currentPath.asString() + "\"";
        renderCommand += " --mdl_path \"" + coreModulePath.asString() + "\"";
        renderCommand += " --mdl_path \"" + coreModulePath2.asString() + "\"";
        renderCommand += " --mdl_path \"" + moduleToTestPath.asString() + "\"";
        renderCommand += " --mdl_path \"" + moduleToTestPath.getParentPath().asString() + "\"";
        renderCommand += " --mdl_path \"" + _libSearchPath.asString() + "\"";
        // This must be a render args option. Rest are consistent between dxr and cuda example renderers.
        std::string renderArgs(MATERIALX_MDL_RENDER_ARGUMENTS);
        if (renderArgs.empty())
        {
            // Assume df_cuda is being used and set reasonable arguments automatically
            renderCommand += " --nogl --res 512 512 -p 2.0 0 0.5 -f 70 --spi 1 --spp 16";
        }
        else
        {
            renderCommand += " " + renderArgs;
        }
        renderCommand += " --noaux";
        std::string iblFile = (currentPath / "resources/lights/san_giuseppe_bridge.hdr").asString();
        renderCommand += " --hdr \"" + iblFile + "\"";
        renderCommand += " ::" + moduleToTest + "::*";

        std::string extension("_mdl.png");
#if defined(MATERIALX_BUILD_OIIO)
        extension = "_mdl.exr";
#endif
        mx::FilePath outputImageName = moduleToTestPath / (moduleToTest + extension);

        renderCommand += " -o " + outputImageName.asString();
        mx::FilePath logFile = moduleToTestPath / (moduleToTest + ".mdl_render_errors.txt");
        renderCommand += " > " + logFile.asString() + " 2>&1";

        int returnValue = std::system(renderCommand.c_str());
        std::ifstream logStream(logFile);
        mx::StringVec result;
        std::string line;
        bool writeLogCode = false;
        while (std::getline(logStream, line))
        {
            if (!writeLogCode)
            {
                _logFile << renderCommand << std::endl;
                _logFile << "\tReturn code: " << std::to_string(returnValue) << std::endl;
                writeLogCode = true;
            }
            _logFile << "\tLog: " << line << std::endl;
        }

        CHECK(returnValue == 0);
    }
}

TEST_CASE("GenShader: MDL Shader Generation", "[genmdl]")
{
    mx::FilePathVec testRootPaths;
    testRootPaths.push_back("resources/Materials/TestSuite");
    testRootPaths.push_back("resources/Materials/Examples");

    const mx::FilePath libSearchPath = mx::FilePath::getCurrentPath();
    mx::FileSearchPath srcSearchPath(libSearchPath.asString());
    srcSearchPath.append(libSearchPath / mx::FilePath("libraries/stdlib/genmdl"));

    const mx::FilePath logPath("genmdl_mdl_generate_test.txt");

    // Write shaders and try to compile only if mdlc exe specified.
    std::string mdlcExec(MATERIALX_MDLC_EXECUTABLE);
    bool writeShadersToDisk = !mdlcExec.empty();
    MdlShaderGeneratorTester tester(mx::MdlShaderGenerator::create(), testRootPaths, libSearchPath, srcSearchPath, logPath, writeShadersToDisk);
    tester.addSkipLibraryFiles();

    mx::GenOptions genOptions;
    genOptions.targetColorSpaceOverride = "lin_rec709";
    mx::FilePath optionsFilePath("resources/Materials/TestSuite/_options.mtlx");
    tester.validate(genOptions, optionsFilePath);
}
