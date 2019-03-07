//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenGlsl/GlslSyntax.h>

#include <MaterialXTest/GenShaderUtil.h>

namespace mx = MaterialX;

TEST_CASE("GLSL Syntax Check", "[genglsl]")
{
    mx::SyntaxPtr syntax = mx::GlslSyntax::create();

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

TEST_CASE("GLSL Implementation Check", "[genglsl]")
{
    mx::GenContext context(mx::GlslShaderGenerator::create());

    std::set<std::string> generatorSkipNodeTypes;
    std::set<std::string> generatorSkipNodeDefs;
    generatorSkipNodeDefs.insert("ND_add_surfaceshader");
    generatorSkipNodeDefs.insert("ND_multiply_surfaceshaderF");
    generatorSkipNodeDefs.insert("ND_multiply_surfaceshaderC");
    generatorSkipNodeDefs.insert("ND_mix_surfaceshader");

    GenShaderUtil::checkImplementations(context, generatorSkipNodeTypes, generatorSkipNodeDefs);
}

TEST_CASE("GLSL Unique Names", "[genglsl]")
{
    mx::GenContext context(mx::GlslShaderGenerator::create());

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    context.registerSourceCodeSearchPath(searchPath);

    GenShaderUtil::testUniqueNames(context, mx::Stage::PIXEL);
}

class GLSLGenCodeGenerationTester : public GenShaderUtil::ShaderGeneratorTester
{
public:
    using ParentClass = GenShaderUtil::ShaderGeneratorTester;

    GLSLGenCodeGenerationTester(const mx::FilePath& testRootPath, const mx::FilePath& libSearchPath,
                                const mx::FileSearchPath& srcSearchPath, const mx::FilePath& logFilePath) 
        : GenShaderUtil::ShaderGeneratorTester(testRootPath, libSearchPath, srcSearchPath, logFilePath)
    {}

    void createGenerator() override
    {
        _shaderGenerator = mx::GlslShaderGenerator::create();
    }

    void addSkipNodeDefs() override
    {
        _skipNodeDefs.insert("ND_add_surfaceshader");
        _skipNodeDefs.insert("ND_multiply_surfaceshaderF");
        _skipNodeDefs.insert("ND_multiply_surfaceshaderC");
        _skipNodeDefs.insert("ND_mix_surfaceshader");
    }

    void setTestStages() override
    {
        _testStages.push_back(mx::Stage::VERTEX);
        _testStages.push_back(mx::Stage::PIXEL);
    }
};

static void generateGLSLCode()
{
    const mx::FilePath testRootPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite");
    const mx::FilePath libSearchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    const mx::FileSearchPath srcSearchPath(libSearchPath.asString());
    const mx::FilePath logPath("genglsl_glsl400_generate_test.txt");
    GLSLGenCodeGenerationTester tester(testRootPath, libSearchPath, srcSearchPath, logPath);

    const mx::GenOptions genOptions;
    tester.testGeneration(genOptions);
}

TEST_CASE("GLSL Shader Generation", "[genglsl]")
{
    generateGLSLCode();
}
