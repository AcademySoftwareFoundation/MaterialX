//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>

#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>
#include <MaterialXGenOgsFx/OgsFxSyntax.h>
#include <MaterialXGenOgsFx/MayaGlslPluginShaderGenerator.h>

#include <MaterialXTest/GenShaderUtil.h>
#include <MaterialXTest/GenGlsl.h>

namespace mx = MaterialX;

TEST_CASE("OGSFX Syntax", "[genogsfx]")
{
    mx::SyntaxPtr syntax = mx::OgsFxSyntax::create();

    REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "float");
    REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "vec3");
    REQUIRE(syntax->getTypeName(mx::Type::VECTOR3) == "vec3");

    REQUIRE(syntax->getTypeName(mx::Type::BSDF) == "BSDF");
    REQUIRE(syntax->getOutputTypeName(mx::Type::BSDF) == "out BSDF");

    // Set fixed precision with one digit
    mx::Value::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

    std::string value;
    value = syntax->getDefaultValue(mx::Type::FLOAT);
    REQUIRE(value == "0.0");
    value = syntax->getDefaultValue(mx::Type::COLOR3);
    REQUIRE(value == "vec3(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR3, true);
    REQUIRE(value == "{0.0, 0.0, 0.0}");
    value = syntax->getDefaultValue(mx::Type::COLOR4);
    REQUIRE(value == "vec4(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4, true);
    REQUIRE(value == "{0.0, 0.0, 0.0, 0.0}");

    mx::ValuePtr floatValue = mx::Value::createValue<float>(42.0f);
    value = syntax->getValue(mx::Type::FLOAT, *floatValue);
    REQUIRE(value == "42.0");
    value = syntax->getValue(mx::Type::FLOAT, *floatValue, true);
    REQUIRE(value == "42.0");

    mx::ValuePtr color3Value = mx::Value::createValue<mx::Color3>(mx::Color3(1.0f, 2.0f, 3.0f));
    value = syntax->getValue(mx::Type::COLOR3, *color3Value);
    REQUIRE(value == "vec3(1.0, 2.0, 3.0)");
    value = syntax->getValue(mx::Type::COLOR3, *color3Value, true);
    REQUIRE(value == "{1.0, 2.0, 3.0}");

    mx::ValuePtr color4Value = mx::Value::createValue<mx::Color4>(mx::Color4(1.0f, 2.0f, 3.0f, 4.0f));
    value = syntax->getValue(mx::Type::COLOR4, *color4Value);
    REQUIRE(value == "vec4(1.0, 2.0, 3.0, 4.0)");
    value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
    REQUIRE(value == "{1.0, 2.0, 3.0, 4.0}");
}

TEST_CASE("OGSFX Implementation Check", "[genogsfx]")
{
    mx::GenContext context(mx::OgsFxShaderGenerator::create());

    std::set<std::string> generatorSkipNodeTypes;
    std::set<std::string> generatorSkipNodeDefs;
    generatorSkipNodeDefs.insert("ND_add_surfaceshader");
    generatorSkipNodeDefs.insert("ND_multiply_surfaceshaderF");
    generatorSkipNodeDefs.insert("ND_multiply_surfaceshaderC");
    generatorSkipNodeDefs.insert("ND_mix_surfaceshader");

    GenShaderUtil::checkImplementations(context, generatorSkipNodeTypes, generatorSkipNodeDefs);
}

TEST_CASE("OGSFX Unique Names", "[genogsfx]")
{
    mx::GenContext context(mx::OgsFxShaderGenerator::create());

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    context.registerSourceCodeSearchPath(searchPath);

    GenShaderUtil::testUniqueNames(context, mx::HW::FX_STAGE);
}

class OgsFxShaderGeneratorTester : public GlslShaderGeneratorTester
{
public:
    OgsFxShaderGeneratorTester(const mx::FilePath& testRootPath, const mx::FilePath& libSearchPath,
                                 const mx::FileSearchPath& srcSearchPath, const mx::FilePath& logFilePath) :
            GlslShaderGeneratorTester(testRootPath, libSearchPath, srcSearchPath, logFilePath)
    {}

    // Only the generator differs for now between OGSFX and GLSL testers
    void createGenerator() override
    {
        _shaderGenerator = mx::OgsFxShaderGenerator::create();
    }
};

static void generateOGSFXCode()
{
    const mx::FilePath testRootPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite");
    const mx::FilePath libSearchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    const mx::FileSearchPath srcSearchPath(libSearchPath.asString());
    const mx::FilePath logPath("genglsl_ogsfx_generate_test.txt");
    OgsFxShaderGeneratorTester tester(testRootPath, libSearchPath, srcSearchPath, logPath);

    const mx::GenOptions genOptions;
    tester.testGeneration(genOptions);
}

TEST_CASE("OGSFX Shader Generation", "[genogsfx]")
{
    generateOGSFXCode();
}
