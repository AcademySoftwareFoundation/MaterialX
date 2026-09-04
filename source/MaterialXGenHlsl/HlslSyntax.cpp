//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHlsl/HlslSyntax.h>

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// HLSL has no native string type, represent as integer enum index.
class HlslStringTypeSyntax : public StringTypeSyntax
{
  public:
    HlslStringTypeSyntax(const Syntax* parent) :
        StringTypeSyntax(parent, "int", "0", "0") { }

    string getValue(const Value& /*value*/, bool /*uniform*/) const override
    {
        return "0";
    }
};

class HlslArrayTypeSyntax : public ScalarTypeSyntax
{
  public:
    HlslArrayTypeSyntax(const Syntax* parent, const string& name) :
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

class HlslFloatArrayTypeSyntax : public HlslArrayTypeSyntax
{
  public:
    explicit HlslFloatArrayTypeSyntax(const Syntax* parent, const string& name) :
        HlslArrayTypeSyntax(parent, name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<float> valueArray = value.asA<vector<float>>();
        return valueArray.size();
    }
};

class HlslIntegerArrayTypeSyntax : public HlslArrayTypeSyntax
{
  public:
    explicit HlslIntegerArrayTypeSyntax(const Syntax* parent, const string& name) :
        HlslArrayTypeSyntax(parent, name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<int> valueArray = value.asA<vector<int>>();
        return valueArray.size();
    }
};

// Vector / matrix aggregate syntax: HLSL accepts "float3(1, 2, 3)", "float4x4(...)"
// constructor calls for built-in vector and matrix types.
class HlslAggregateTypeSyntax : public AggregateTypeSyntax
{
  public:
    HlslAggregateTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
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

// User-defined struct aggregate syntax: HLSL has no Type(a, b) constructor for
// user structs, so emit brace initialization "{a, b}" instead.
class HlslStructInitTypeSyntax : public AggregateTypeSyntax
{
  public:
    HlslStructInitTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
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

        return valueString.empty() ? valueString : "{" + valueString + "}";
    }
};

} // anonymous namespace

const string HlslSyntax::INPUT_QUALIFIER = "";
const string HlslSyntax::OUTPUT_QUALIFIER = "out";
const string HlslSyntax::UNIFORM_QUALIFIER = "uniform";
const string HlslSyntax::CONSTANT_QUALIFIER = "const";
const string HlslSyntax::FLAT_QUALIFIER = "nointerpolation";
const string HlslSyntax::SOURCE_FILE_EXTENSION = ".hlsl";
const StringVec HlslSyntax::VEC2_MEMBERS = { ".x", ".y" };
const StringVec HlslSyntax::VEC3_MEMBERS = { ".x", ".y", ".z" };
const StringVec HlslSyntax::VEC4_MEMBERS = { ".x", ".y", ".z", ".w" };

//
// HlslSyntax methods
//

HlslSyntax::HlslSyntax(TypeSystemPtr typeSystem) :
    Syntax(typeSystem)
{
    // Reserved words and keywords in HLSL (Shader Model 5.x / 6.x)
    registerReservedWords({ "asm", "asm_fragment", "break", "case", "cbuffer", "centroid",
                            "class", "column_major", "compile", "compile_fragment", "const",
                            "continue", "default", "discard", "do", "double", "else",
                            "export", "extern", "false", "float", "for", "fxgroup",
                            "globallycoherent", "groupshared", "half", "if", "in", "inline",
                            "inout", "int", "interface", "line", "lineadj", "linear",
                            "matrix", "min10float", "min12int", "min16float", "min16int", "min16uint",
                            "namespace", "nointerpolation", "noperspective", "NULL", "out",
                            "packoffset", "pass", "pixelfragment", "point", "precise", "register",
                            "return", "row_major", "sample", "sampler", "shared", "snorm", "stateblock",
                            "stateblock_state", "static", "string", "struct", "switch", "tbuffer",
                            "technique", "technique10", "technique11", "texture", "texture1D",
                            "texture1DArray", "Texture2D", "Texture2DArray", "Texture2DMS",
                            "Texture2DMSArray", "Texture3D", "TextureCube", "TextureCubeArray",
                            "true", "typedef", "triangle", "triangleadj", "uint", "uniform",
                            "unorm", "unsigned", "vector", "vertexfragment", "void", "volatile",
                            "while", "SamplerState", "SamplerComparisonState", "SamplerTexture2D",
                            "Buffer", "RWBuffer", "ByteAddressBuffer", "RWByteAddressBuffer",
                            "StructuredBuffer", "RWStructuredBuffer", "AppendStructuredBuffer",
                            "ConsumeStructuredBuffer", "RWTexture1D", "RWTexture1DArray",
                            "RWTexture2D", "RWTexture2DArray", "RWTexture3D",
                            "InputPatch", "OutputPatch", "PointStream", "LineStream",
                            "TriangleStream", "ConstantBuffer" });

    //
    // Register syntax handlers for each data type.
    //

    // HLSL cbuffers cannot initialize uniforms inline; the uniform default value is
    // a string consumable by Value::createValueFromStrings.
    registerTypeSyntax(
        Type::FLOAT,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "float",
            "0.0",
            "0.0"));

    registerTypeSyntax(
        Type::FLOATARRAY,
        std::make_shared<HlslFloatArrayTypeSyntax>(
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
        std::make_shared<HlslIntegerArrayTypeSyntax>(
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
        std::make_shared<HlslAggregateTypeSyntax>(
            this,
            "float3",
            "float3(0.0, 0.0, 0.0)",
            "0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::COLOR4,
        std::make_shared<HlslAggregateTypeSyntax>(
            this,
            "float4",
            "float4(0.0, 0.0, 0.0, 0.0)",
            "0.0, 0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR2,
        std::make_shared<HlslAggregateTypeSyntax>(
            this,
            "float2",
            "float2(0.0, 0.0)",
            "0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC2_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR3,
        std::make_shared<HlslAggregateTypeSyntax>(
            this,
            "float3",
            "float3(0.0, 0.0, 0.0)",
            "0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR4,
        std::make_shared<HlslAggregateTypeSyntax>(
            this,
            "float4",
            "float4(0.0, 0.0, 0.0, 0.0)",
            "0.0, 0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::MATRIX33,
        std::make_shared<HlslAggregateTypeSyntax>(
            this,
            "float3x3",
            "float3x3(1,0,0,  0,1,0, 0,0,1)",
            "1,0,0,  0,1,0, 0,0,1"));

    registerTypeSyntax(
        Type::MATRIX44,
        std::make_shared<HlslAggregateTypeSyntax>(
            this,
            "float4x4",
            "float4x4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1)",
            "1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1"));

    registerTypeSyntax(
        Type::STRING,
        std::make_shared<HlslStringTypeSyntax>(this));

    registerTypeSyntax(
        Type::FILENAME,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "SamplerTexture2D",
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::BSDF,
        std::make_shared<HlslStructInitTypeSyntax>(
            this,
            "BSDF",
            "{float3(0.0, 0.0, 0.0), float3(1.0, 1.0, 1.0)}",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct BSDF { float3 response; float3 throughput; };"));

    registerTypeSyntax(
        Type::EDF,
        std::make_shared<HlslAggregateTypeSyntax>(
            this,
            "EDF",
            "EDF(0.0, 0.0, 0.0)",
            "0.0, 0.0, 0.0",
            "float3",
            "#define EDF float3"));

    registerTypeSyntax(
        Type::VDF,
        std::make_shared<HlslStructInitTypeSyntax>(
            this,
            "VDF",
            "{float3(0.0, 0.0, 0.0), float3(1.0, 1.0, 1.0)}",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct VDF { float3 response; float3 throughput; };"));

    registerTypeSyntax(
        Type::SURFACESHADER,
        std::make_shared<HlslStructInitTypeSyntax>(
            this,
            "surfaceshader",
            "{float3(0.0, 0.0, 0.0), float3(0.0, 0.0, 0.0)}",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct surfaceshader { float3 color; float3 transparency; };"));

    registerTypeSyntax(
        Type::VOLUMESHADER,
        std::make_shared<HlslStructInitTypeSyntax>(
            this,
            "volumeshader",
            "{float3(0.0, 0.0, 0.0), float3(0.0, 0.0, 0.0)}",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct volumeshader { float3 color; float3 transparency; };"));

    registerTypeSyntax(
        Type::DISPLACEMENTSHADER,
        std::make_shared<HlslStructInitTypeSyntax>(
            this,
            "displacementshader",
            "{float3(0.0, 0.0, 0.0), 1.0}",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct displacementshader { float3 offset; float scale; };"));

    registerTypeSyntax(
        Type::LIGHTSHADER,
        std::make_shared<HlslStructInitTypeSyntax>(
            this,
            "lightshader",
            "{float3(0.0, 0.0, 0.0), float3(0.0, 0.0, 0.0)}",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct lightshader { float3 intensity; float3 direction; };"));

    registerTypeSyntax(
        Type::MATERIAL,
        std::make_shared<HlslStructInitTypeSyntax>(
            this,
            "material",
            "{float3(0.0, 0.0, 0.0), float3(0.0, 0.0, 0.0)}",
            EMPTY_STRING,
            "surfaceshader",
            "#define material surfaceshader"));
}

bool HlslSyntax::typeSupported(const TypeDesc* type) const
{
    return *type != Type::STRING;
}

void HlslSyntax::makeValidName(string& name) const
{
    Syntax::makeValidName(name);
    if (std::isdigit(name[0]))
        name = "v_" + name;
}

bool HlslSyntax::remapEnumeration(const string& value, TypeDesc type, const string& enumNames, std::pair<TypeDesc, ValuePtr>& result) const
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

    // For HLSL we always convert to integer,
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

StructTypeSyntaxPtr HlslSyntax::createStructSyntax(const string& structTypeName, const string& defaultValue,
                                                   const string& uniformDefaultValue, const string& typeAlias,
                                                   const string& typeDefinition) const
{
    return std::make_shared<HlslStructTypeSyntax>(
        this,
        structTypeName,
        defaultValue,
        uniformDefaultValue,
        typeAlias,
        typeDefinition);
}

string HlslStructTypeSyntax::getValue(const Value& value, bool /* uniform */) const
{
    const AggregateValue& aggValue = static_cast<const AggregateValue&>(value);

    // HLSL has no Type(...) constructor for user structs; emit brace init.
    string result = "{";

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

    result += "}";

    return result;
}

MATERIALX_NAMESPACE_END
