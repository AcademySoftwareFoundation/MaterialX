//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenSlang/SlangSyntax.h>

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Since Slang doesn't support strings we use integers instead.
// TODO: Support options strings by converting to a corresponding enum integer
class SlangStringTypeSyntax : public StringTypeSyntax
{
  public:
    SlangStringTypeSyntax(const Syntax* parent) :
        StringTypeSyntax(parent, "int", "0", "0") { }

    string getValue(const Value& /*value*/, bool /*uniform*/) const override
    {
        return "0";
    }
};

class SlangArrayTypeSyntax : public ScalarTypeSyntax
{
  public:
    SlangArrayTypeSyntax(const Syntax* parent, const string& name) :
        ScalarTypeSyntax(parent, name, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING)
    {
    }

    string getValue(const Value& value, bool uniform) const override
    {
        if (uniform)
        {
            return value.getValueString();
        }

        size_t arraySize = getSize(value);
        if (arraySize > 0)
        {
            return "{" + value.getValueString() + "}";
        }
        return EMPTY_STRING;
    }

  protected:
    virtual size_t getSize(const Value& value) const = 0;
};

class SlangFloatArrayTypeSyntax : public SlangArrayTypeSyntax
{
  public:
    explicit SlangFloatArrayTypeSyntax(const Syntax* parent, const string& name) :
        SlangArrayTypeSyntax(parent, name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<float> valueArray = value.asA<vector<float>>();
        return valueArray.size();
    }
};

class SlangIntegerArrayTypeSyntax : public SlangArrayTypeSyntax
{
  public:
    explicit SlangIntegerArrayTypeSyntax(const Syntax* parent, const string& name) :
        SlangArrayTypeSyntax(parent, name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<int> valueArray = value.asA<vector<int>>();
        return valueArray.size();
    }
};

class SlangAggregateTypeSyntax : public AggregateTypeSyntax
{
  public:
    SlangAggregateTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                             const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING,
                             const StringVec& members = EMPTY_MEMBERS) :
        AggregateTypeSyntax(parent, name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, members)
    {
    }

    string getValue(const Value& value, bool uniform) const override
    {
        const string valueString = value.getValueString();
        if (uniform)
        {
            return valueString;
        }

        return valueString.empty() ? valueString : getName() + "(" + valueString + ")";
    }
};

} // anonymous namespace

const string SlangSyntax::INPUT_QUALIFIER = "";
const string SlangSyntax::OUTPUT_QUALIFIER = "out";
const string SlangSyntax::UNIFORM_QUALIFIER = "uniform";
const string SlangSyntax::CONSTANT_QUALIFIER = "const";
const string SlangSyntax::FLAT_QUALIFIER = "nointerpolation";
const string SlangSyntax::SOURCE_FILE_EXTENSION = ".slang";
const StringVec SlangSyntax::VEC2_MEMBERS = { ".x", ".y" };
const StringVec SlangSyntax::VEC3_MEMBERS = { ".x", ".y", ".z" };
const StringVec SlangSyntax::VEC4_MEMBERS = { ".x", ".y", ".z", ".w" };

//
// SlangSyntax methods
//

SlangSyntax::SlangSyntax(TypeSystemPtr typeSystem) :
    Syntax(typeSystem)
{
    // Add in all reserved words and keywords in Slang
    registerReservedWords({ "throws", "static", "const", "in", "out", "inout",
                            "ref", "__subscript", "__init", "property", "get", "set",
                            "class", "struct", "interface", "public", "private", "internal",
                            "protected", "typedef", "typealias", "uniform", "export", "groupshared",
                            "extension", "associatedtype", "namespace", "This", "using", "__generic",
                            "__exported", "import", "enum", "cbuffer", "tbuffer", "func",
                            "if", "else", "switch", "case", "default", "return",
                            "try", "throw", "throws", "catch", "while", "for",
                            "do", "static", "const", "in", "out", "inout",
                            "ref", "__subscript", "__init", "property", "get", "set",
                            "class", "struct", "interface", "public", "private", "internal",
                            "protected", "typedef", "typealias", "uniform", "export", "groupshared",
                            "extension", "associatedtype", "this", "namespace", "This", "using",
                            "__generic", "__exported", "import", "enum", "break", "continue",
                            "discard", "defer", "cbuffer", "tbuffer", "func", "is",
                            "as", "nullptr", "none", "true", "false", "SamplerTexture2D" });

    // Register restricted tokens in Slang
    // No invalid tokens

    //
    // Register syntax handlers for each data type.
    //

    // Slang cannot initialize uniforms from the code, so the uniform default value
    // will instead of string that can be passed to Value::createValueFromStrings
    registerTypeSyntax(
        Type::FLOAT,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "float",
            "0.0",
            "0.0"));

    registerTypeSyntax(
        Type::FLOATARRAY,
        std::make_shared<SlangFloatArrayTypeSyntax>(
            this,
            "float"));

    registerTypeSyntax(
        Type::INTEGER,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "int",
            "0",
            "0"));

    registerTypeSyntax(
        Type::INTEGERARRAY,
        std::make_shared<SlangIntegerArrayTypeSyntax>(
            this,
            "int"));

    registerTypeSyntax(
        Type::BOOLEAN,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "bool",
            "false",
            "false"));

    registerTypeSyntax(
        Type::COLOR3,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "float3",
            "float3(0.0)",
            "0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::COLOR4,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "float4",
            "float4(0.0)",
            "0.0, 0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR2,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "float2",
            "float2(0.0)",
            "0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC2_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR3,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "float3",
            "float3(0.0)",
            "0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR4,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "float4",
            "float4(0.0)",
            "0.0, 0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::MATRIX33,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "float3x3",
            "float3x3(1,0,0,  0,1,0, 0,0,1)",
            "1,0,0,  0,1,0, 0,0,1"));

    registerTypeSyntax(
        Type::MATRIX44,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "float4x4",
            "float4x4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1)",
            "1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1"));

    registerTypeSyntax(
        Type::STRING,
        std::make_shared<SlangStringTypeSyntax>(this));

    registerTypeSyntax(
        Type::FILENAME,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "SamplerTexture2D",
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::BSDF,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "BSDF",
            "BSDF(float3(0.0),float3(1.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct BSDF { float3 response; float3 throughput; };"));

    registerTypeSyntax(
        Type::EDF,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "EDF",
            "EDF(0.0)",
            "0.0, 0.0, 0.0",
            "float3",
            "#define EDF float3"));

    registerTypeSyntax(
        Type::VDF,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "BSDF",
            "BSDF(float3(0.0),float3(1.0))",
            EMPTY_STRING));

    registerTypeSyntax(
        Type::SURFACESHADER,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "surfaceshader",
            "surfaceshader(float3(0.0),float3(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct surfaceshader { float3 color; float3 transparency; };"));

    registerTypeSyntax(
        Type::VOLUMESHADER,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "volumeshader",
            "volumeshader(float3(0.0),float3(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct volumeshader { float3 color; float3 transparency; };"));

    registerTypeSyntax(
        Type::DISPLACEMENTSHADER,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "displacementshader",
            "displacementshader(float3(0.0),1.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct displacementshader { float3 offset; float scale; };"));

    registerTypeSyntax(
        Type::LIGHTSHADER,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "lightshader",
            "lightshader(float3(0.0),float3(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct lightshader { float3 intensity; float3 direction; };"));

    registerTypeSyntax(
        Type::MATERIAL,
        std::make_shared<SlangAggregateTypeSyntax>(
            this,
            "material",
            "material(float3(0.0),float3(0.0))",
            EMPTY_STRING,
            "surfaceshader",
            "#define material surfaceshader"));
}

bool SlangSyntax::typeSupported(const TypeDesc* type) const
{
    return *type != Type::STRING;
}

void SlangSyntax::makeValidName(string& name) const
{
    Syntax::makeValidName(name);
    if (std::isdigit(name[0]))
        name = "v_" + name;
}

bool SlangSyntax::remapEnumeration(const string& value, TypeDesc type, const string& enumNames, std::pair<TypeDesc, ValuePtr>& result) const
{
    // Early out if not an enum input.
    if (enumNames.empty())
    {
        return false;
    }

    // Don't convert already supported types
    if (type != Type::STRING)
    {
        return false;
    }

    // Early out if no valid value provided
    if (value.empty())
    {
        return false;
    }

    // For Slang we always convert to integer,
    // with the integer value being an index into the enumeration.
    result.first = Type::INTEGER;
    result.second = nullptr;

    // Try remapping to an enum value.
    StringVec valueElemEnumsVec = splitString(enumNames, ",");
    for (size_t i = 0; i < valueElemEnumsVec.size(); i++)
    {
        valueElemEnumsVec[i] = trimSpaces(valueElemEnumsVec[i]);
    }
    auto pos = std::find(valueElemEnumsVec.begin(), valueElemEnumsVec.end(), value);
    if (pos == valueElemEnumsVec.end())
    {
        throw ExceptionShaderGenError("Given value '" + value + "' is not a valid enum value for input.");
    }
    const int index = static_cast<int>(std::distance(valueElemEnumsVec.begin(), pos));
    result.second = Value::createValue<int>(index);

    return true;
}

StructTypeSyntaxPtr SlangSyntax::createStructSyntax(const string& structTypeName, const string& defaultValue,
                                                    const string& uniformDefaultValue, const string& typeAlias,
                                                    const string& typeDefinition) const
{
    return std::make_shared<SlangStructTypeSyntax>(
        this,
        structTypeName,
        defaultValue,
        uniformDefaultValue,
        typeAlias,
        typeDefinition);
}

string SlangStructTypeSyntax::getValue(const Value& value, bool /* uniform */) const
{
    const AggregateValue& aggValue = static_cast<const AggregateValue&>(value);

    string result = aggValue.getTypeString() + "(";

    string separator = "";
    for (const auto& memberValue : aggValue.getMembers())
    {
        result += separator;
        separator = ",";

        const string& memberTypeName = memberValue->getTypeString();
        const TypeDesc memberTypeDesc = _parent->getType(memberTypeName);

        // Recursively use the syntax to generate the output, so we can supported nested structs.
        result += _parent->getValue(memberTypeDesc, *memberValue, true);
    }

    result += ")";

    return result;
}

MATERIALX_NAMESPACE_END
