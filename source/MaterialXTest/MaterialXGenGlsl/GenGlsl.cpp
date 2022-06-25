//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>
#include <MaterialXTest/MaterialXGenGlsl/GenGlsl.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>

#include <MaterialXGenShader/TypeDesc.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenGlsl/GlslSyntax.h>
#include <MaterialXGenGlsl/GlslResourceBindingContext.h>
#include <MaterialXGenGlsl/VkShaderGenerator.h>

namespace mx = MaterialX;

TEST_CASE("GenShader: GLSL Syntax Check", "[genglsl]")
{
    mx::SyntaxPtr syntax = mx::GlslSyntax::create();

    REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "float");
    REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "vec3");
    REQUIRE(syntax->getTypeName(mx::Type::VECTOR3) == "vec3");

    REQUIRE(syntax->getTypeName(mx::Type::BSDF) == "BSDF");
    REQUIRE(syntax->getOutputTypeName(mx::Type::BSDF) == "out BSDF");

    // Set fixed precision with one digit
    mx::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

    std::string value;
    value = syntax->getDefaultValue(mx::Type::FLOAT);
    REQUIRE(value == "0.0");
    value = syntax->getDefaultValue(mx::Type::COLOR3);
    REQUIRE(value == "vec3(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR3, true);
    REQUIRE(value == "vec3(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4);
    REQUIRE(value == "vec4(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4, true);
    REQUIRE(value == "vec4(0.0)");
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
    REQUIRE(value == "vec3(1.0, 2.0, 3.0)");
    value = syntax->getValue(mx::Type::COLOR3, *color3Value, true);
    REQUIRE(value == "vec3(1.0, 2.0, 3.0)");

    mx::ValuePtr color4Value = mx::Value::createValue<mx::Color4>(mx::Color4(1.0f, 2.0f, 3.0f, 4.0f));
    value = syntax->getValue(mx::Type::COLOR4, *color4Value);
    REQUIRE(value == "vec4(1.0, 2.0, 3.0, 4.0)");
    value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
    REQUIRE(value == "vec4(1.0, 2.0, 3.0, 4.0)");

    std::vector<float> floatArray = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f };
    mx::ValuePtr floatArrayValue = mx::Value::createValue<std::vector<float>>(floatArray);
    value = syntax->getValue(mx::Type::FLOATARRAY, *floatArrayValue);
    REQUIRE(value == "float[7](0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7)");

    std::vector<int> intArray = { 1, 2, 3, 4, 5, 6, 7 };
    mx::ValuePtr intArrayValue = mx::Value::createValue<std::vector<int>>(intArray);
    value = syntax->getValue(mx::Type::INTEGERARRAY, *intArrayValue);
    REQUIRE(value == "int[7](1, 2, 3, 4, 5, 6, 7)");
}

TEST_CASE("GenShader: GLSL Implementation Check", "[genglsl]")
{
    mx::GenContext context(mx::GlslShaderGenerator::create());

    mx::StringSet generatorSkipNodeTypes;
    mx::StringSet generatorSkipNodeDefs;
    GenShaderUtil::checkImplementations(context, generatorSkipNodeTypes, generatorSkipNodeDefs, 54);
}

TEST_CASE("GenShader: GLSL Unique Names", "[genglsl]")
{
    mx::GenContext context(mx::GlslShaderGenerator::create());

    mx::FilePath currentPath = mx::FilePath::getCurrentPath();
    context.registerSourceCodeSearchPath(currentPath);

    GenShaderUtil::testUniqueNames(context, mx::Stage::PIXEL);
}

TEST_CASE("GenShader: Bind Light Shaders", "[genglsl]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FileSearchPath searchPath;
    searchPath.append(mx::FilePath::getCurrentPath());
    loadLibraries({ "libraries" }, searchPath, doc);

    mx::NodeDefPtr pointLightShader = doc->getNodeDef("ND_point_light");
    mx::NodeDefPtr spotLightShader = doc->getNodeDef("ND_spot_light");
    REQUIRE(pointLightShader != nullptr);
    REQUIRE(spotLightShader != nullptr);

    mx::GenContext context(mx::GlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);

    mx::HwShaderGenerator::bindLightShader(*pointLightShader, 42, context);
    REQUIRE_THROWS(mx::HwShaderGenerator::bindLightShader(*spotLightShader, 42, context));
    mx::HwShaderGenerator::unbindLightShader(42, context);
    REQUIRE_NOTHROW(mx::HwShaderGenerator::bindLightShader(*spotLightShader, 42, context));
    REQUIRE_NOTHROW(mx::HwShaderGenerator::bindLightShader(*pointLightShader, 66, context));
    mx::HwShaderGenerator::unbindLightShaders(context);
    REQUIRE_NOTHROW(mx::HwShaderGenerator::bindLightShader(*spotLightShader, 66, context));
}

enum class GlslType
{
    Glsl400,
    Glsl420,
    GlslVulkan
};

const std::string GlslTypeToString(GlslType e) throw()
{
    switch (e)
    {
        case GlslType::Glsl420:
            return "glsl420_layout";
        case GlslType::GlslVulkan:
            return "glsl420_vulkan";
        case GlslType::Glsl400:
        default:
            return "glsl400";
    }
}

static void generateGlslCode(GlslType type = GlslType::Glsl400)
{
    mx::FilePathVec testRootPaths;
    testRootPaths.push_back("resources/Materials/TestSuite");
    testRootPaths.push_back("resources/Materials/Examples/StandardSurface");
    const mx::FilePath libSearchPath = mx::FilePath::getCurrentPath();
    const mx::FileSearchPath srcSearchPath(libSearchPath.asString());
    bool writeShadersToDisk = false;

    const mx::GenOptions genOptions;
    mx::FilePath optionsFilePath("resources/Materials/TestSuite/_options.mtlx");

    const mx::FilePath logPath("genglsl_" + GlslTypeToString(type) + "_generate_test.txt");

    GlslShaderGeneratorTester tester((type == GlslType::GlslVulkan) ? mx::VkShaderGenerator::create() : mx::GlslShaderGenerator::create(),
                                     testRootPaths, libSearchPath, srcSearchPath, logPath, writeShadersToDisk);

    // Add resource binding context for glsl 4.20
    if (type == GlslType::Glsl420)
    {
        // Set binding context to handle resource binding layouts
        mx::GlslResourceBindingContextPtr glslresourceBinding(mx::GlslResourceBindingContext::create());
        glslresourceBinding->enableSeparateBindingLocations(true);
        tester.addUserData(mx::HW::USER_DATA_BINDING_CONTEXT, glslresourceBinding);
    }

    tester.validate(genOptions, optionsFilePath);
}

TEST_CASE("GenShader: GLSL Shader Generation", "[genglsl]")
{
    // Generate with standard GLSL i.e version 400
    generateGlslCode(GlslType::Glsl400);
}

TEST_CASE("GenShader: GLSL Shader with Layout Generation", "[genglsl]")
{
    // Generate GLSL with layout i.e version 400 + layout extension
    generateGlslCode(GlslType::Glsl420);
}

TEST_CASE("GenShader: Vulkan GLSL Shader", "[genglsl]")
{
    // Generate with GLSL for Vulkan i.e. version 450
    generateGlslCode(GlslType::GlslVulkan);
}
