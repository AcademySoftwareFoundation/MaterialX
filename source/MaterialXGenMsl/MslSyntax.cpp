//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMsl/MslSyntax.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Since MSL doesn't support strings we use integers instead.
// TODO: Support options strings by converting to a corresponding enum integer
class MslStringTypeSyntax : public StringTypeSyntax
{
  public:
    MslStringTypeSyntax(const Syntax* parent) :
        StringTypeSyntax(parent, "int", "0", "0") { }

    string getValue(const Value& /*value*/, bool /*uniform*/) const override
    {
        return "0";
    }
};

class MslArrayTypeSyntax : public ScalarTypeSyntax
{
  public:
    MslArrayTypeSyntax(const Syntax* parent, const string& name) :
        ScalarTypeSyntax(parent, name, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING)
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
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

class MslFloatArrayTypeSyntax : public MslArrayTypeSyntax
{
  public:
    explicit MslFloatArrayTypeSyntax(const Syntax* parent, const string& name) :
        MslArrayTypeSyntax(parent, name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<float> valueArray = value.asA<vector<float>>();
        return valueArray.size();
    }
};

class MslIntegerArrayTypeSyntax : public MslArrayTypeSyntax
{
  public:
    explicit MslIntegerArrayTypeSyntax(const Syntax* parent, const string& name) :
        MslArrayTypeSyntax(parent, name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<int> valueArray = value.asA<vector<int>>();
        return valueArray.size();
    }
};

} // anonymous namespace

const string MslSyntax::INPUT_QUALIFIER = "in";
const string MslSyntax::OUTPUT_QUALIFIER = "out";
const string MslSyntax::UNIFORM_QUALIFIER = "constant";
const string MslSyntax::CONSTANT_QUALIFIER = "const";
const string MslSyntax::FLAT_QUALIFIER = "flat";
const string MslSyntax::SOURCE_FILE_EXTENSION = ".metal";
const string MslSyntax::STRUCT_KEYWORD = "struct";
const StringVec MslSyntax::VEC2_MEMBERS = { ".x", ".y" };
const StringVec MslSyntax::VEC3_MEMBERS = { ".x", ".y", ".z" };
const StringVec MslSyntax::VEC4_MEMBERS = { ".x", ".y", ".z", ".w" };

//
// MslSyntax methods
//

MslSyntax::MslSyntax(TypeSystemPtr typeSystem) : Syntax(typeSystem)
{
    // Add in all reserved words and keywords in MSL
    registerReservedWords(
        { "centroid", "flat", "smooth", "noperspective", "patch", "sample",
          "break", "continue", "do", "for", "while", "switch", "case", "default",
          "if", "else,", "subroutine", "in", "out", "inout",
          "float", "double", "int", "void", "bool", "true", "false",
          "invariant", "discard_fragment", "return",
          "float2x2", "float2x3", "float2x4",
          "float3x2", "float3x3", "float3x4",
          "float4x2", "float4x3", "float4x4",
          "float2", "float3", "float4", "int2", "int3", "int4", "bool2", "bool3", "bool4",
          "uint", "uint2", "uint3", "uint4",
          "lowp", "mediump", "highp", "precision",
          "sampler",
          "common", "partition", "active", "asm",
          "struct", "class", "union", "enum", "typedef", "template", "this", "packed",
          "inline", "noinline", "volatile", "public", "static", "extern", "external", "interface",
          "long", "short", "half", "fixed", "unsigned", "superp", "input", "output",
          "half2", "half3", "half4",
          "sampler3DRect", "filter",
          "texture1d", "texture2d", "texture3d", "textureCube",
          "buffer",
          "sizeof", "cast", "namespace", "using", "row_major",
          "mix", "sampler" });

    // Register restricted tokens in MSL
    StringMap tokens;
    tokens["__"] = "_";
    tokens["gl_"] = "gll";
    tokens["webgl_"] = "webgll";
    tokens["_webgl"] = "wwebgl";
    registerInvalidTokens(tokens);

    //
    // Register syntax handlers for each data type.
    //

    registerTypeSyntax(
        Type::FLOAT,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "float",
            "0.0",
            "0.0"));

    registerTypeSyntax(
        Type::FLOATARRAY,
        std::make_shared<MslFloatArrayTypeSyntax>(
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
        std::make_shared<MslIntegerArrayTypeSyntax>(
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
        std::make_shared<AggregateTypeSyntax>(
            this,
            "vec3",
            "vec3(0.0)",
            "vec3(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::COLOR4,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "vec4",
            "vec4(0.0)",
            "vec4(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR2,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "vec2",
            "vec2(0.0)",
            "vec2(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC2_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR3,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "vec3",
            "vec3(0.0)",
            "vec3(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR4,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "vec4",
            "vec4(0.0)",
            "vec4(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::MATRIX33,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "mat3",
            "mat3(1.0)",
            "mat3(1.0)"));

    registerTypeSyntax(
        Type::MATRIX44,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "mat4",
            "mat4(1.0)",
            "mat4(1.0)"));

    registerTypeSyntax(
        Type::STRING,
        std::make_shared<MslStringTypeSyntax>(this));

    registerTypeSyntax(
        Type::FILENAME,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "MetalTexture",
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::BSDF,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "BSDF",
            "BSDF{float3(0.0),float3(1.0)}",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct BSDF { float3 response; float3 throughput; };"));

    registerTypeSyntax(
        Type::EDF,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "EDF",
            "EDF(0.0)",
            "EDF(0.0)",
            "float3",
            "#define EDF float3"));

    registerTypeSyntax(
        Type::VDF,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "BSDF",
            "BSDF{float3(0.0),float3(1.0)}",
            EMPTY_STRING));

    registerTypeSyntax(
        Type::SURFACESHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "surfaceshader",
            "surfaceshader{float3(0.0),float3(0.0)}",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct surfaceshader { float3 color; float3 transparency; };"));

    registerTypeSyntax(
        Type::VOLUMESHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "volumeshader",
            "volumeshader{float3(0.0),float3(0.0)}",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct volumeshader { float3 color; float3 transparency; };"));

    registerTypeSyntax(
        Type::DISPLACEMENTSHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "displacementshader",
            "displacementshader{float3(0.0),1.0}",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct displacementshader { float3 offset; float scale; };"));

    registerTypeSyntax(
        Type::LIGHTSHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "lightshader",
            "lightshader{float3(0.0),float3(0.0)}",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct lightshader { float3 intensity; float3 direction; };"));

    registerTypeSyntax(
        Type::MATERIAL,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "material",
            "material{float3(0.0),float3(0.0)}",
            EMPTY_STRING,
            "surfaceshader",
            "#define material surfaceshader"));
}

string MslSyntax::getOutputTypeName(TypeDesc type) const
{
    const TypeSyntax& syntax = getTypeSyntax(type);
    return "thread " + syntax.getName() + "&";
}

bool MslSyntax::typeSupported(const TypeDesc* type) const
{
    return *type != Type::STRING;
}

bool MslSyntax::remapEnumeration(const string& value, TypeDesc type, const string& enumNames, std::pair<TypeDesc, ValuePtr>& result) const
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

    // For MSL we always convert to integer,
    // with the integer value being an index into the enumeration.
    result.first = Type::INTEGER;
    result.second = nullptr;

    // Try remapping to an enum value.
    if (!value.empty())
    {
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
    }

    return true;
}

MATERIALX_NAMESPACE_END
