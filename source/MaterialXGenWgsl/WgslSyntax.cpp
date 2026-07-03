//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/WgslSyntax.h>

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// WGSL has no string type, so MaterialX string/enum inputs are represented as integers.
class WgslStringTypeSyntax : public StringTypeSyntax
{
  public:
    WgslStringTypeSyntax(const Syntax* parent) :
        StringTypeSyntax(parent, "i32", "0", "0") { }

    string getValue(const Value& /*value*/, bool /*uniform*/) const override
    {
        return "0";
    }
};

// WGSL aggregate construction uses the type name as the constructor: vec3f(...), BSDF(...).
class WgslAggregateTypeSyntax : public AggregateTypeSyntax
{
  public:
    WgslAggregateTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
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

const string WgslSyntax::INPUT_QUALIFIER = "";
const string WgslSyntax::OUTPUT_QUALIFIER = "";
// WGSL uniforms are declared via var<uniform> in the resource binding context, so there is
// no per-variable uniform keyword.
const string WgslSyntax::UNIFORM_QUALIFIER = "uniform";
const string WgslSyntax::CONSTANT_QUALIFIER = "const";
// Integer varyings need @interpolate(flat); handled where varyings are emitted.
const string WgslSyntax::FLAT_QUALIFIER = "@interpolate(flat)";
const string WgslSyntax::SOURCE_FILE_EXTENSION = ".wgsl";
const StringVec WgslSyntax::VEC2_MEMBERS = { ".x", ".y" };
const StringVec WgslSyntax::VEC3_MEMBERS = { ".x", ".y", ".z" };
const StringVec WgslSyntax::VEC4_MEMBERS = { ".x", ".y", ".z", ".w" };

//
// WgslSyntax methods
//

WgslSyntax::WgslSyntax(TypeSystemPtr typeSystem) :
    Syntax(typeSystem)
{
    // Register WGSL keywords and reserved words (https://www.w3.org/TR/WGSL/#keyword-summary).
    registerReservedWords({
        // Keywords
        "alias", "break", "case", "const", "const_assert", "continue", "continuing",
        "default", "diagnostic", "discard", "else", "enable", "false", "fn", "for",
        "if", "let", "loop", "override", "requires", "return", "struct", "switch",
        "true", "var", "while",
        // Reserved words
        "NULL", "Self", "abstract", "active", "alignas", "alignof", "as", "asm",
        "asm_fragment", "async", "attribute", "auto", "await", "become", "cast", "catch",
        "class", "co_await", "co_return", "co_yield", "coherent", "column_major", "common",
        "compile", "compile_fragment", "concept", "const_cast", "consteval", "constexpr",
        "constinit", "crate", "debugger", "decltype", "delete", "demote", "demote_to_helper",
        "do", "dynamic_cast", "enum", "explicit", "export", "extends", "extern", "external",
        "fallthrough", "filter", "final", "finally", "friend", "from", "fxgroup", "get",
        "goto", "groupshared", "highp", "impl", "implements", "import", "inline", "instanceof",
        "interface", "layout", "lowp", "macro", "macro_rules", "match", "mediump", "meta",
        "mod", "module", "move", "mut", "mutable", "namespace", "new", "nil", "noexcept",
        "noinline", "nointerpolation", "non_coherent", "noncoherent", "noperspective", "null",
        "nullptr", "of", "operator", "package", "packoffset", "partition", "pass", "patch",
        "pixelfragment", "precise", "precision", "premerge", "priv", "protected", "pub",
        "public", "readonly", "ref", "regardless", "register", "reinterpret_cast", "require",
        "resource", "restrict", "self", "set", "shared", "sizeof", "smooth", "snorm", "static",
        "static_assert", "static_cast", "std", "subroutine", "super", "target", "template",
        "this", "thread_local", "throw", "trait", "try", "type", "typedef", "typeid", "typename",
        "typeof", "union", "unless", "unorm", "unsafe", "unsized", "use", "using", "varying",
        "virtual", "volatile", "wgsl", "where", "with", "writeonly", "yield",
        // Common built-in type/intrinsic names that must not collide with generated variables.
        "f32", "i32", "u32", "vec2f", "vec3f", "vec4f", "mat3x3f", "mat4x4f", "texture_2d",
        "sampler", "mix", "select" });

    //
    // Register syntax handlers for each data type.
    //

    registerTypeSyntax(
        Type::FLOAT,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "f32",
            "0.0",
            "0.0"));

    registerTypeSyntax(
        Type::FLOATARRAY,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "f32",
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::INTEGER,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "i32",
            "0",
            "0"));

    registerTypeSyntax(
        Type::INTEGERARRAY,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "i32",
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::BOOLEAN,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "bool",
            "false",
            "false"));

    registerTypeSyntax(
        Type::COLOR3,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "vec3f",
            "vec3f(0.0)",
            "0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::COLOR4,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "vec4f",
            "vec4f(0.0)",
            "0.0, 0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR2,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "vec2f",
            "vec2f(0.0)",
            "0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC2_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR3,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "vec3f",
            "vec3f(0.0)",
            "0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR4,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "vec4f",
            "vec4f(0.0)",
            "0.0, 0.0, 0.0, 0.0",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::MATRIX33,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "mat3x3f",
            "mat3x3f(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)",
            "1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0"));

    registerTypeSyntax(
        Type::MATRIX44,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "mat4x4f",
            "mat4x4f(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0)",
            "1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0"));

    registerTypeSyntax(
        Type::STRING,
        std::make_shared<WgslStringTypeSyntax>(this));

    // File textures are split into a texture_2d<f32> + sampler pair; the generator handles
    // FILENAME specially in emitVariableDeclaration / emitFunctionDefinitionParameter / emitInput.
    registerTypeSyntax(
        Type::FILENAME,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "texture_2d<f32>",
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::BSDF,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "BSDF",
            "BSDF(vec3f(0.0), vec3f(1.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::EDF,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "EDF",
            "EDF(0.0)",
            "0.0, 0.0, 0.0",
            "vec3f",
            EMPTY_STRING));

    registerTypeSyntax(
        Type::VDF,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "VDF",
            "VDF(vec3f(0.0), vec3f(1.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::SURFACESHADER,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "surfaceshader",
            "surfaceshader(vec3f(0.0), vec3f(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::VOLUMESHADER,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "volumeshader",
            "volumeshader(vec3f(0.0), vec3f(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::DISPLACEMENTSHADER,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "displacementshader",
            "displacementshader(vec3f(0.0), 1.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::LIGHTSHADER,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "lightshader",
            "lightshader(vec3f(0.0), vec3f(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::MATERIAL,
        std::make_shared<WgslAggregateTypeSyntax>(
            this,
            "material",
            "material(vec3f(0.0), vec3f(0.0))",
            EMPTY_STRING,
            "surfaceshader",
            EMPTY_STRING));
}

void WgslSyntax::makeValidName(string& name) const
{
    Syntax::makeValidName(name);
    // WGSL identifiers cannot begin with a digit.
    if (!name.empty() && std::isdigit(static_cast<unsigned char>(name[0])))
        name = "v_" + name;
}

bool WgslSyntax::remapEnumeration(const string& value, TypeDesc type, const string& enumNames, std::pair<TypeDesc, ValuePtr>& result) const
{
    // Early out if not an enum input.
    if (enumNames.empty())
    {
        return false;
    }

    // Don't convert already supported types.
    if (type != Type::STRING)
    {
        return false;
    }

    // Early out if no valid value provided.
    if (value.empty())
    {
        return false;
    }

    // WGSL has no strings, so enum strings are remapped to an integer index.
    result.first = Type::INTEGER;
    result.second = nullptr;

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

StructTypeSyntaxPtr WgslSyntax::createStructSyntax(const string& structTypeName, const string& defaultValue,
                                                   const string& uniformDefaultValue, const string& typeAlias,
                                                   const string& typeDefinition) const
{
    return std::make_shared<WgslStructTypeSyntax>(
        this,
        structTypeName,
        defaultValue,
        uniformDefaultValue,
        typeAlias,
        typeDefinition);
}

string WgslStructTypeSyntax::getValue(const Value& value, bool /* uniform */) const
{
    const AggregateValue& aggValue = static_cast<const AggregateValue&>(value);

    string result = aggValue.getTypeString() + "(";

    string separator = "";
    for (const auto& memberValue : aggValue.getMembers())
    {
        result += separator;
        separator = ", ";

        const string& memberTypeName = memberValue->getTypeString();
        const TypeDesc memberTypeDesc = _parent->getType(memberTypeName);

        // Recursively use the syntax to generate the output, so nested structs are supported.
        result += _parent->getValue(memberTypeDesc, *memberValue, true);
    }

    result += ")";

    return result;
}

MATERIALX_NAMESPACE_END
