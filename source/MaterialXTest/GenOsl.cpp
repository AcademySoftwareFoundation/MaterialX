#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslSyntax.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXTest/GenShaderUtil.h>

namespace mx = MaterialX;

TEST_CASE("OSL Syntax", "[genosl]")
{
    mx::SyntaxPtr syntax = mx::OslSyntax::create();

    REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "float");
    REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "color");
    REQUIRE(syntax->getTypeName(mx::Type::VECTOR3) == "vector");
    REQUIRE(syntax->getTypeName(mx::Type::FLOATARRAY) == "float");
    REQUIRE(syntax->getTypeName(mx::Type::INTEGERARRAY) == "int");
    REQUIRE(mx::Type::FLOATARRAY->isArray());
    REQUIRE(mx::Type::INTEGERARRAY->isArray());

    REQUIRE(syntax->getTypeName(mx::Type::BSDF) == "BSDF");
    REQUIRE(syntax->getOutputTypeName(mx::Type::BSDF) == "output BSDF");

    // Set fixed precision with one digit
    mx::Value::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

    std::string value;
    value = syntax->getDefaultValue(mx::Type::FLOAT);
    REQUIRE(value == "0.0");
    value = syntax->getDefaultValue(mx::Type::COLOR3);
    REQUIRE(value == "color(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR3, true);
    REQUIRE(value == "color(0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4);
    REQUIRE(value == "color4(color(0.0), 0.0)");
    value = syntax->getDefaultValue(mx::Type::COLOR4, true);
    REQUIRE(value == "{color(0.0), 0.0}");
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
    REQUIRE(value == "color4(color(1.0, 2.0, 3.0), 4.0)");
    value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
    REQUIRE(value == "{color(1.0, 2.0, 3.0), 4.0}");

    std::vector<float> floatArray = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f };
    mx::ValuePtr floatArrayValue = mx::Value::createValue<std::vector<float>>(floatArray);
    value = syntax->getValue(mx::Type::FLOATARRAY, *floatArrayValue);
    REQUIRE(value == "{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7}");

    std::vector<int> intArray = { 1, 2, 3, 4, 5, 6, 7 };
    mx::ValuePtr intArrayValue = mx::Value::createValue<std::vector<int>>(intArray);
    value = syntax->getValue(mx::Type::INTEGERARRAY, *intArrayValue);
    REQUIRE(value == "{1, 2, 3, 4, 5, 6, 7}");
}

TEST_CASE("OSL Implementation Check", "[genosl]")
{
    mx::ShaderGeneratorPtr generator = mx::OslShaderGenerator::create();

    std::set<std::string> generatorSkipNodeTypes;
    generatorSkipNodeTypes.insert("light");
    generatorSkipNodeTypes.insert("pointlight");
    generatorSkipNodeTypes.insert("directionallight");
    generatorSkipNodeTypes.insert("spotlight");
    std::set<std::string> generatorSkipNodeDefs;

    GenShaderUtil::checkImplementations(generator, generatorSkipNodeTypes, generatorSkipNodeDefs);
}

TEST_CASE("OSL Unique Names", "[genosl]")
{
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGeneratorPtr shaderGenerator = mx::OslShaderGenerator::create();
    shaderGenerator->registerSourceCodeSearchPath(searchPath);
    // Add path to find OSL include files
    shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

    GenShaderUtil::testUniqueNames(shaderGenerator, mx::Shader::PIXEL_STAGE);
}

class OSLGenCodeGenerationTester : public GenShaderUtil::ShaderGeneratorTester
{
public:
    using ParentClass = GenShaderUtil::ShaderGeneratorTester;

    OSLGenCodeGenerationTester(const mx::FilePath& searchPath, const mx::FilePath& testRootPath,
        const mx::FilePath& logFilePath) : GenShaderUtil::ShaderGeneratorTester(searchPath, testRootPath, logFilePath)
    {}

    void createGenerator() override
    {
        _shaderGenerator = mx::OslShaderGenerator::create();
        _shaderGenerator->registerSourceCodeSearchPath(_searchPath);
        // Add path to find OSL include files
        _shaderGenerator->registerSourceCodeSearchPath(_searchPath / mx::FilePath("stdlib/osl"));

        if (!_shaderGenerator)
        {
            _logFile << ">> Failed to create OSL generator" << std::endl;
        }
    }

    void setTestStages() override
    {
        _testStages.push_back(mx::Shader::PIXEL_STAGE);
    }
};

static void generateOSLCode()
{
    const mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    const mx::FilePath testRootPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite");
    const mx::FilePath logPath("genosl_vanilla_generate_test.txt");
    OSLGenCodeGenerationTester tester(searchPath, testRootPath, logPath);
 
    const mx::GenOptions genOptions;
    tester.testGeneration(genOptions);
}

TEST_CASE("OSL Shader Generation", "[genosl]")
{
    generateOSLCode();
}
