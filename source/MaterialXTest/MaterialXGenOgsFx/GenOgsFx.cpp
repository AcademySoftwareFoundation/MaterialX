//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>
#include <MaterialXTest/MaterialXGenGlsl/GenGlsl.h>

#include <MaterialXFormat/File.h>

#include <MaterialXGenShader/TypeDesc.h>

#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>
#include <MaterialXGenOgsFx/OgsFxSyntax.h>

namespace mx = MaterialX;

TEST_CASE("GenShader: OGSFX Syntax", "[genogsfx]")
{
    mx::SyntaxPtr syntax = mx::OgsFxSyntax::create();

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

TEST_CASE("GenShader: OGSFX Implementation Check", "[genogsfx]")
{
    mx::GenContext context(mx::OgsFxShaderGenerator::create());

    mx::StringSet generatorSkipNodeTypes;
    mx::StringSet generatorSkipNodeDefs;
    GenShaderUtil::checkImplementations(context, generatorSkipNodeTypes, generatorSkipNodeDefs, 62);
}

TEST_CASE("GenShader: OGSFX Unique Names", "[genogsfx]")
{
    mx::GenContext context(mx::OgsFxShaderGenerator::create());

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    context.registerSourceCodeSearchPath(searchPath);

    GenShaderUtil::testUniqueNames(context, mx::Stage::EFFECT);
}

class OgsFxShaderGeneratorTester : public GlslShaderGeneratorTester
{
  public:
    OgsFxShaderGeneratorTester(const mx::FilePathVec& testRootPaths, const mx::FilePath& libSearchPath,
                               const mx::FileSearchPath& srcSearchPath, const mx::FilePath& logFilePath, 
                               bool writeShadersToDisk) :
            GlslShaderGeneratorTester(mx::OgsFxShaderGenerator::create(), testRootPaths, libSearchPath, srcSearchPath, logFilePath, writeShadersToDisk)
    {}

    void setTestStages() override
    {
        _testStages.push_back(mx::Stage::EFFECT);
    }

  protected:
    void getImplementationWhiteList(mx::StringSet& whiteList) override
    {
        whiteList =
        {
            "ambientocclusion", "arrayappend", "backfacing", "screen", "curveadjust", "displacementshader",
            "volumeshader", "IM_constant_", "IM_dot_", "IM_geompropvalue", "IM_light_genglsl",
            "IM_point_light_genglsl", "IM_spot_light_genglsl", "IM_directional_light_genglsl", "IM_angle", "material", "ND_material",
            "ND_backface_util", "IM_backface_util_genglsl"
        };
        ShaderGeneratorTester::getImplementationWhiteList(whiteList);
    }
};

static void generateOgsFxCode()
{
    const mx::FilePath testRootPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite");
    const mx::FilePath testRootPath2 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/StandardSurface");
    const mx::FilePath testRootPath3 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/UsdPreviewSurface");
    const mx::FilePath testRootPath4 = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/Examples/Units");
    mx::FilePathVec testRootPaths;
    testRootPaths.push_back(testRootPath);
    testRootPaths.push_back(testRootPath2);
    testRootPaths.push_back(testRootPath3);
    testRootPaths.push_back(testRootPath4);
    const mx::FilePath libSearchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    const mx::FileSearchPath srcSearchPath(libSearchPath.asString());
    const mx::FilePath logPath("genglsl_ogsfx_generate_test.txt");

    bool writeShadersToDisk = true;
    OgsFxShaderGeneratorTester tester(testRootPaths, libSearchPath, srcSearchPath, logPath, writeShadersToDisk);

    const mx::GenOptions genOptions;
    mx::FilePath optionsFilePath = testRootPath / mx::FilePath("_options.mtlx");
    tester.validate(genOptions, optionsFilePath);
}

TEST_CASE("GenShader: OGSFX Shader Generation", "[genogsfx]")
{
    generateOgsFxCode();
}
