//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXTest/MaterialXGenSlang/GenSlang.h>

#include <MaterialXGenSlang/SlangShaderGenerator.h>
#include <MaterialXGenSlang/SlangSyntax.h>

namespace mx = MaterialX;

TEST_CASE("GenShader: Slang Syntax Check", "[genslang]")
{
    mx::TypeSystemPtr ts = mx::TypeSystem::create();
    mx::SyntaxPtr syntax = mx::SlangSyntax::create(ts);

    REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "float");
    REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "float3");
    REQUIRE(syntax->getTypeName(mx::Type::VECTOR3) == "float3");

    REQUIRE(syntax->getTypeName(mx::Type::BSDF) == "BSDF");
    REQUIRE(syntax->getOutputTypeName(mx::Type::BSDF) == "out BSDF");

    // Set fixed precision with one digit
    mx::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

    auto convertUniform = [&syntax](const std::string& strDefault, mx::TypeDesc typeDesc) -> std::string
    {
        auto value = mx::Value::createValueFromStrings(strDefault, typeDesc.getName());
        return syntax->getValue(typeDesc, *value);
    };

    std::string value;
    value = syntax->getDefaultValue(mx::Type::FLOAT);
    REQUIRE(value == "0.0");
    value = syntax->getDefaultValue(mx::Type::COLOR3);
    REQUIRE(value == "float3(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR3, true);
    REQUIRE(value == "0.0, 0.0, 0.0");
    value = convertUniform(value, mx::Type::COLOR3);
    REQUIRE(value == "float3(0.0, 0.0, 0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4);
    REQUIRE(value == "float4(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4, true);
    REQUIRE(value == "0.0, 0.0, 0.0, 0.0");
    value = convertUniform(value, mx::Type::COLOR4);
    REQUIRE(value == "float4(0.0, 0.0, 0.0, 0.0)");
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
    REQUIRE(value == "float3(1.0, 2.0, 3.0)");
    value = syntax->getValue(mx::Type::COLOR3, *color3Value, true);
    REQUIRE(value == "1.0, 2.0, 3.0");
    value = convertUniform(value, mx::Type::COLOR3);
    REQUIRE(value == "float3(1.0, 2.0, 3.0)");

    mx::ValuePtr color4Value = mx::Value::createValue<mx::Color4>(mx::Color4(1.0f, 2.0f, 3.0f, 4.0f));
    value = syntax->getValue(mx::Type::COLOR4, *color4Value);
    REQUIRE(value == "float4(1.0, 2.0, 3.0, 4.0)");
    value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
    REQUIRE(value == "1.0, 2.0, 3.0, 4.0");
    value = convertUniform(value, mx::Type::COLOR4);
    REQUIRE(value == "float4(1.0, 2.0, 3.0, 4.0)");

    std::vector<float> floatArray = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f };
    mx::ValuePtr floatArrayValue = mx::Value::createValue<std::vector<float>>(floatArray);
    value = syntax->getValue(mx::Type::FLOATARRAY, *floatArrayValue);
    REQUIRE(value == "{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7}");

    std::vector<int> intArray = { 1, 2, 3, 4, 5, 6, 7 };
    mx::ValuePtr intArrayValue = mx::Value::createValue<std::vector<int>>(intArray);
    value = syntax->getValue(mx::Type::INTEGERARRAY, *intArrayValue);
    REQUIRE(value == "{1, 2, 3, 4, 5, 6, 7}");
}

TEST_CASE("GenShader: Slang Implementation Check", "[genslang]")
{
    mx::GenContext context(mx::SlangShaderGenerator::create());

    mx::StringSet generatorSkipNodeTypes;
    mx::StringSet generatorSkipNodeDefs;
    GenShaderUtil::checkImplementations(context, generatorSkipNodeTypes, generatorSkipNodeDefs);
}

TEST_CASE("GenShader: Slang Unique Names", "[genslang]")
{
    mx::GenContext context(mx::SlangShaderGenerator::create());
    context.registerSourceCodeSearchPath(mx::getDefaultDataSearchPath());
    GenShaderUtil::testUniqueNames(context, mx::Stage::PIXEL);
}

TEST_CASE("GenShader: Slang Light Shaders", "[genslang]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    loadLibraries({ "libraries" }, searchPath, doc);

    mx::NodeDefPtr pointLightShader = doc->getNodeDef("ND_point_light");
    mx::NodeDefPtr spotLightShader = doc->getNodeDef("ND_spot_light");
    REQUIRE(pointLightShader != nullptr);
    REQUIRE(spotLightShader != nullptr);

    mx::GenContext context(mx::SlangShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);
    context.getShaderGenerator().registerTypeDefs(doc);

    mx::HwShaderGenerator::bindLightShader(*pointLightShader, 42, context);
    REQUIRE_THROWS(mx::HwShaderGenerator::bindLightShader(*spotLightShader, 42, context));
    mx::HwShaderGenerator::unbindLightShader(42, context);
    REQUIRE_NOTHROW(mx::HwShaderGenerator::bindLightShader(*spotLightShader, 42, context));
    REQUIRE_NOTHROW(mx::HwShaderGenerator::bindLightShader(*pointLightShader, 66, context));
    mx::HwShaderGenerator::unbindLightShaders(context);
    REQUIRE_NOTHROW(mx::HwShaderGenerator::bindLightShader(*spotLightShader, 66, context));
}

#ifdef MATERIALX_BUILD_BENCHMARK_TESTS
TEST_CASE("GenShader: Slang Performance Test", "[genslang]")
{
    mx::GenContext context(mx::SlangShaderGenerator::create());
    BENCHMARK("Load documents, validate and generate shader") 
    {
        return GenShaderUtil::shaderGenPerformanceTest(context);
    };
}
#endif

TEST_CASE("GenShader: Slang Shader Generation", "[genslang]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    mx::FilePathVec testRootPaths;
    testRootPaths.push_back(searchPath.find("resources/Materials/TestSuite"));
    testRootPaths.push_back(searchPath.find("resources/Materials/Examples/StandardSurface"));

    // Create the requested shader generator.
    mx::ShaderGeneratorPtr generator;
    generator = mx::SlangShaderGenerator::create();

    const mx::FilePath logPath("genslang_generate_test.txt");
    SlangShaderGeneratorTester tester(generator, testRootPaths, searchPath, logPath, false);

    // Handle resource binding layouts if requested.
    const mx::GenOptions genOptions;
    mx::FilePath optionsFilePath = searchPath.find("resources/Materials/TestSuite/_options.mtlx");
    tester.validate(genOptions, optionsFilePath);
}
