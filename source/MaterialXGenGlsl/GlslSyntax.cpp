//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/GlslSyntax.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Since GLSL doesn't support strings we use integers instead.
// TODO: Support options strings by converting to a corresponding enum integer
class GlslStringTypeSyntax : public StringTypeSyntax
{
  public:
    GlslStringTypeSyntax(const Syntax* parent) :
        StringTypeSyntax(parent, "int", "0", "0") { }

    string getValue(const Value& /*value*/, bool /*uniform*/) const override
    {
        return "0";
    }
};

class GlslArrayTypeSyntax : public ScalarTypeSyntax
{
  public:
    GlslArrayTypeSyntax(const Syntax* parent, const string& name) :
        ScalarTypeSyntax(parent, name, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING)
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        size_t arraySize = getSize(value);
        if (arraySize > 0)
        {
            return _name + "[" + std::to_string(arraySize) + "](" + value.getValueString() + ")";
        }
        return EMPTY_STRING;
    }

  protected:
    virtual size_t getSize(const Value& value) const = 0;
};

class GlslFloatArrayTypeSyntax : public GlslArrayTypeSyntax
{
  public:
    explicit GlslFloatArrayTypeSyntax(const Syntax* parent, const string& name) :
        GlslArrayTypeSyntax(parent, name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<float> valueArray = value.asA<vector<float>>();
        return valueArray.size();
    }
};

class GlslIntegerArrayTypeSyntax : public GlslArrayTypeSyntax
{
  public:
    explicit GlslIntegerArrayTypeSyntax(const Syntax* parent, const string& name) :
        GlslArrayTypeSyntax(parent, name)
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

const string GlslSyntax::INPUT_QUALIFIER = "in";
const string GlslSyntax::OUTPUT_QUALIFIER = "out";
const string GlslSyntax::UNIFORM_QUALIFIER = "uniform";
const string GlslSyntax::CONSTANT_QUALIFIER = "const";
const string GlslSyntax::FLAT_QUALIFIER = "flat";
const string GlslSyntax::SOURCE_FILE_EXTENSION = ".glsl";
const StringVec GlslSyntax::VEC2_MEMBERS = { ".x", ".y" };
const StringVec GlslSyntax::VEC3_MEMBERS = { ".x", ".y", ".z" };
const StringVec GlslSyntax::VEC4_MEMBERS = { ".x", ".y", ".z", ".w" };

//
// GlslSyntax methods
//

GlslSyntax::GlslSyntax(TypeSystemPtr typeSystem) :
    Syntax(typeSystem)
{
    // Add in all reserved words and keywords in GLSL
    registerReservedWords(
        { "centroid", "flat", "smooth", "noperspective", "patch", "sample",
          "break", "continue", "do", "for", "while", "switch", "case", "default",
          "if", "else,", "subroutine", "in", "out", "inout",
          "float", "double", "int", "void", "bool", "true", "false",
          "invariant", "discard", "return",
          "mat2", "mat3", "mat4", "dmat2", "dmat3", "dmat4",
          "mat2x2", "mat2x3", "mat2x4", "dmat2x2", "dmat2x3", "dmat2x4",
          "mat3x2", "mat3x3", "mat3x4", "dmat3x2", "dmat3x3", "dmat3x4",
          "mat4x2", "mat4x3", "mat4x4", "dmat4x2", "dmat4x3", "dmat4x4",
          "vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4", "bvec2", "bvec3", "bvec4", "dvec2", "dvec3", "dvec4",
          "uint", "uvec2", "uvec3", "uvec4",
          "lowp", "mediump", "highp", "precision",
          "sampler1D", "sampler2D", "sampler3D", "samplerCube",
          "sampler1DShadow", "sampler2DShadow", "samplerCubeShadow",
          "sampler1DArray", "sampler2DArray",
          "sampler1DArrayShadow", "sampler2DArrayShadow",
          "isampler1D", "isampler2D", "isampler3D", "isamplerCube",
          "isampler1DArray", "isampler2DArray",
          "usampler1D", "usampler2D", "usampler3D", "usamplerCube",
          "usampler1DArray", "usampler2DArray",
          "sampler2DRect", "sampler2DRectShadow", "isampler2DRect", "usampler2DRect",
          "samplerBuffer", "isamplerBuffer", "usamplerBuffer",
          "sampler2DMS", "isampler2DMS", "usampler2DMS",
          "sampler2DMSArray", "isampler2DMSArray", "usampler2DMSArray",
          "samplerCubeArray", "samplerCubeArrayShadow", "isamplerCubeArray", "usamplerCubeArray",
          "common", "partition", "active", "asm",
          "struct", "class", "union", "enum", "typedef", "template", "this", "packed", "goto",
          "inline", "noinline", "volatile", "public", "static", "extern", "external", "interface",
          "long", "short", "half", "fixed", "unsigned", "superp", "input", "output",
          "hvec2", "hvec3", "hvec4", "fvec2", "fvec3", "fvec4",
          "sampler3DRect", "filter",
          "image1D", "image2D", "image3D", "imageCube",
          "iimage1D", "iimage2D", "iimage3D", "iimageCube",
          "uimage1D", "uimage2D", "uimage3D", "uimageCube",
          "image1DArray", "image2DArray",
          "iimage1DArray", "iimage2DArray", "uimage1DArray", "uimage2DArray",
          "image1DShadow", "image2DShadow",
          "image1DArrayShadow", "image2DArrayShadow",
          "imageBuffer", "iimageBuffer", "uimageBuffer",
          "sizeof", "cast", "namespace", "using", "row_major",
          "mix", "sampler" });

    // Register restricted tokens in GLSL
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
        std::make_shared<GlslFloatArrayTypeSyntax>(
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
        std::make_shared<GlslIntegerArrayTypeSyntax>(
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
        std::make_shared<GlslStringTypeSyntax>(this));

    registerTypeSyntax(
        Type::FILENAME,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "sampler2D",
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::BSDF,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "BSDF",
            "BSDF(vec3(0.0),vec3(1.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct BSDF { vec3 response; vec3 throughput; };"));

    registerTypeSyntax(
        Type::EDF,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "EDF",
            "EDF(0.0)",
            "EDF(0.0)",
            "vec3",
            "#define EDF vec3"));

    registerTypeSyntax(
        Type::VDF,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "BSDF",
            "BSDF(vec3(0.0),vec3(1.0))",
            EMPTY_STRING));

    registerTypeSyntax(
        Type::SURFACESHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "surfaceshader",
            "surfaceshader(vec3(0.0),vec3(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct surfaceshader { vec3 color; vec3 transparency; };"));

    registerTypeSyntax(
        Type::VOLUMESHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "volumeshader",
            "volumeshader(vec3(0.0),vec3(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct volumeshader { vec3 color; vec3 transparency; };"));

    registerTypeSyntax(
        Type::DISPLACEMENTSHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "displacementshader",
            "displacementshader(vec3(0.0),1.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct displacementshader { vec3 offset; float scale; };"));

    registerTypeSyntax(
        Type::LIGHTSHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "lightshader",
            "lightshader(vec3(0.0),vec3(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct lightshader { vec3 intensity; vec3 direction; };"));

    registerTypeSyntax(
        Type::MATERIAL,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "material",
            "material(vec3(0.0),vec3(0.0))",
            EMPTY_STRING,
            "surfaceshader",
            "#define material surfaceshader"));
}

bool GlslSyntax::typeSupported(const TypeDesc* type) const
{
    return *type != Type::STRING;
}

bool GlslSyntax::remapEnumeration(const string& value, TypeDesc type, const string& enumNames, std::pair<TypeDesc, ValuePtr>& result) const
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

    // For GLSL we always convert to integer,
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

StructTypeSyntaxPtr GlslSyntax::createStructSyntax(const string& structTypeName, const string& defaultValue,
                                                   const string& uniformDefaultValue, const string& typeAlias,
                                                   const string& typeDefinition) const
{
    return std::make_shared<GlslStructTypeSyntax>(
        this,
        structTypeName,
        defaultValue,
        uniformDefaultValue,
        typeAlias,
        typeDefinition);
}

string GlslStructTypeSyntax::getValue(const Value& value, bool /* uniform */) const
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
