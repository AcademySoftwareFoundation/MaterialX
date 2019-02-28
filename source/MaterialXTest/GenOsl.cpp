#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslSyntax.h>

namespace mx = MaterialX;

extern void checkImplementations(mx::GenContext& context, mx::ShaderGeneratorPtr generator,
    std::set<std::string> generatorSkipNodeTypes,
    std::set<std::string> generatorSkipNodeDefs);

extern void testUniqueNames(mx::GenContext& context, mx::ShaderGeneratorPtr shaderGenerator, const std::string& stage);

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
    mx::GenContext context;
    mx::ShaderGeneratorPtr generator = mx::OslShaderGenerator::create();

    std::set<std::string> generatorSkipNodeTypes;
    generatorSkipNodeTypes.insert("light");
    generatorSkipNodeTypes.insert("pointlight");
    generatorSkipNodeTypes.insert("directionallight");
    generatorSkipNodeTypes.insert("spotlight");
    std::set<std::string> generatorSkipNodeDefs;

    checkImplementations(context, generator, generatorSkipNodeTypes, generatorSkipNodeDefs);
}

TEST_CASE("OSL Unique Names", "[genosl]")
{
    mx::GenContext contex;
    mx::ShaderGeneratorPtr shaderGenerator = mx::OslShaderGenerator::create();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    contex.registerSourceCodeSearchPath(searchPath);
    // Add path to find OSL include files
    contex.registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

    testUniqueNames(contex, shaderGenerator, mx::OSL::STAGE);
}
