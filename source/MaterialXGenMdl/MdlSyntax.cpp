//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMdl/MdlSyntax.h>

#include <MaterialXGenShader/ShaderGenerator.h>

#include <MaterialXFormat/File.h>

#include <sstream>

MATERIALX_NAMESPACE_BEGIN

namespace
{

const string MARKER_MDL_VERSION_SUFFIX = "MDL_VERSION_SUFFIX";

class MdlFilenameTypeSyntax : public ScalarTypeSyntax
{
  public:
    MdlFilenameTypeSyntax(const Syntax* parent) :
        ScalarTypeSyntax(parent, "texture_2d", "texture_2d()", "texture_2d()")
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        const string outputValue = value.getValueString();
        if (outputValue.empty() || outputValue == "/")
        {
            return getDefaultValue(true);
        }
        // handle the empty texture, the fileprefix is passed
        // assuming it ends with a slash ...
        if (outputValue.back() == '/')
        {
            return getDefaultValue(true);
        }
        // ... or the last segment does not have an extension suffix
        size_t idx_s = outputValue.find_last_of('/');
        size_t idx_d = outputValue.find_last_of('.');
        if (idx_d == std::string::npos || (idx_s != std::string::npos && idx_s > idx_d))
        {
            return getDefaultValue(true);
        }

        // prefix a slash in order to make MDL resource paths absolute i.e. to be found
        // in the root of an MDL search path
        // do not add the slash in case the path is explicitly relative
        string pathSeparator("");
        FilePath path(outputValue);
        size_t len = outputValue.size();
        if (!path.isAbsolute() &&
            !(len > 2 && outputValue[0] == '.' && outputValue[1] == '.' && outputValue[2] == '/') &&
            !(len > 1 && outputValue[0] == '.' && outputValue[1] == '/'))
        {
            pathSeparator = "/";
        }

        // MDL is using leading slashes as separator
        return getName() + "(\"" + pathSeparator + path.asString(FilePath::FormatPosix) + "\", tex::gamma_linear)";
    }
};

class MdlArrayTypeSyntax : public ScalarTypeSyntax
{
  public:
    MdlArrayTypeSyntax(const Syntax* parent, const string& name) :
        ScalarTypeSyntax(parent, name, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING)
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        if (!isEmpty(value))
        {
            return getName() + "[](" + value.getValueString() + ")";
        }
        return EMPTY_STRING;
    }

  protected:
    virtual bool isEmpty(const Value& value) const = 0;
};

class MdlFloatArrayTypeSyntax : public MdlArrayTypeSyntax
{
  public:
    explicit MdlFloatArrayTypeSyntax(const Syntax* parent, const string& name) :
        MdlArrayTypeSyntax(parent, name)
    {
    }

  protected:
    bool isEmpty(const Value& value) const override
    {
        const vector<float>& valueArray = value.asA<vector<float>>();
        return valueArray.empty();
    }
};

class MdlIntegerArrayTypeSyntax : public MdlArrayTypeSyntax
{
  public:
    explicit MdlIntegerArrayTypeSyntax(const Syntax* parent, const string& name) :
        MdlArrayTypeSyntax(parent, name)
    {
    }

  protected:
    bool isEmpty(const Value& value) const override
    {
        const vector<int>& valueArray = value.asA<vector<int>>();
        return valueArray.empty();
    }
};

// For the color4 type we need even more specialization since it's a struct of a struct:
//
// struct color4 {
//    color rgb;
//    float a;
// }
//
class MdlColor4TypeSyntax : public AggregateTypeSyntax
{
  public:
    MdlColor4TypeSyntax(const Syntax* parent) :
        AggregateTypeSyntax(parent, "color4", "mk_color4(0.0)", "mk_color4(0.0)",
                            EMPTY_STRING, EMPTY_STRING, MdlSyntax::COLOR4_MEMBERS)
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        StringStream ss;

        // Set float format and precision for the stream
        const Value::FloatFormat fmt = Value::getFloatFormat();
        ss.setf(std::ios_base::fmtflags(
                    (fmt == Value::FloatFormatFixed ? std::ios_base::fixed : (fmt == Value::FloatFormatScientific ? std::ios_base::scientific : 0))),
                std::ios_base::floatfield);
        ss.precision(Value::getFloatPrecision());

        const Color4 c = value.asA<Color4>();
        ss << "mk_color4(" << c[0] << ", " << c[1] << ", " << c[2] << ", " << c[3] << ")";

        return ss.str();
    }
};

class MdlEnumSyntax : public AggregateTypeSyntax
{
  public:
    MdlEnumSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& defaultUniformValue, const StringVec& members) :
        AggregateTypeSyntax(parent, name, defaultValue, defaultUniformValue, EMPTY_STRING, EMPTY_STRING, members)
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        return _name + "_" + value.getValueString();
    }
};

} // anonymous namespace

const string MdlSyntax::CONST_QUALIFIER = "";
const string MdlSyntax::UNIFORM_QUALIFIER = "uniform";
const string MdlSyntax::SOURCE_FILE_EXTENSION = ".mdl";
const StringVec MdlSyntax::VECTOR2_MEMBERS = { ".x", ".y" };
const StringVec MdlSyntax::VECTOR3_MEMBERS = { ".x", ".y", ".z" };
const StringVec MdlSyntax::VECTOR4_MEMBERS = { ".x", ".y", ".z", ".w" };
const StringVec MdlSyntax::COLOR3_MEMBERS = { ".x", ".y", ".z" };
const StringVec MdlSyntax::COLOR4_MEMBERS = { ".x", ".y", ".z", ".a" };

const StringVec MdlSyntax::ADDRESSMODE_MEMBERS = { "constant", "clamp", "periodic", "mirror" };
const StringVec MdlSyntax::COORDINATESPACE_MEMBERS = { "model", "object", "world" };
const StringVec MdlSyntax::FILTERLOOKUPMODE_MEMBERS = { "closest", "linear", "cubic" };
const StringVec MdlSyntax::FILTERTYPE_MEMBERS = { "box", "gaussian" };
const StringVec MdlSyntax::DISTRIBUTIONTYPE_MEMBERS = { "ggx" };
const StringVec MdlSyntax::SCATTER_MODE_MEMBERS = { "R", "T", "RT" };
const StringVec MdlSyntax::SHEEN_MODE_MEMBERS = { "conty_kulla", "zeltner" };

const string MdlSyntax::PORT_NAME_PREFIX = "mxp_";

//
// MdlSyntax methods
//

MdlSyntax::MdlSyntax(TypeSystemPtr typeSystem) : Syntax(typeSystem)
{
    // Add in all reserved words and keywords in MDL
    // Formatted as in the MDL Specification 1.9.2 for easy comparing
    registerReservedWords(
        {   // Reserved words
            "annotation",       "double2",      "float",        "in",                           "operator",
            "auto",             "double2x2",    "float2",       "int",                          "package",
            "bool",             "double2x3",    "float2x2",     "int2",                         "return",
            "bool2",            "double3",      "float2x3",     "int3",                         "string",
            "bool3",            "double3x2",    "float3",       "int4",                         "struct",
            "bool4",            "double3x3",    "float3x2",     "intensity_mode",               "struct_category",
            "break",            "double3x4",    "float3x3",     "intensity_power",              "switch",
            "bsdf",             "double4",      "float3x4",     "intensity_radiant_exitance",   "texture_2d",
            "bsdf_measurement", "double4x3",    "float4",       "let",                          "texture_3d",
            "case",             "double4x4",    "float4x3",     "light_profile",                "texture_cube",
            "cast",             "double4x2",    "float4x4",     "material",                     "texture_ptex",
            "color",            "double2x4",    "float4x2",     "material_emission",            "true",
            "const",            "edf",          "float2x4",     "material_geometry",            "typedef",
            "continue",         "else",         "for",          "material_surface",             "uniform",
            "declarative",      "enum",         "hair_bsdf",    "material_volume",              "using",
            "default",          "export",       "if",           "mdl",                          "varying",
            "do",               "false",        "import",       "module",                       "vdf",
            "double",                                                                           "while",

            // Reserved for future use
            "catch",        "friend",   "half3x4",  "mutable",          "sampler",      "throw",
            "char",         "goto",     "half4",    "namespace",        "shader",       "try",
            "class",        "graph",    "half4x3",  "native",           "short",        "typeid",
            "const_cast",   "half",     "half4x4",  "new",              "signed",       "typename",
            "delete",       "half2",    "half4x2",  "out",              "sizeof",       "union",
            "dynamic_cast", "half2x2",  "half2x4",  "phenomenon",       "static",       "unsigned",
            "explicit",     "half2x3",  "inline",   "private",          "static_cast",  "virtual",
            "extern",       "half3",    "inout",    "protected",        "technique",    "void",
            "external",     "half3x2",  "lambda",   "public",           "template",     "volatile",
            "foreach",      "half3x3",  "long",     "reinterpret_cast", "this",         "wchar_t",
        });

    // Register restricted tokens in MDL
    StringMap tokens;
    tokens["\\b(_)"] = "u"; // Disallow names which begin with underscores
    registerInvalidTokens(tokens);

    //
    // Register type syntax handlers for each data type.
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
        std::make_shared<MdlFloatArrayTypeSyntax>(
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
        std::make_shared<MdlIntegerArrayTypeSyntax>(
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
            "color",
            "color(0.0)",
            "color(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            COLOR3_MEMBERS));

    registerTypeSyntax(
        Type::COLOR4,
        std::make_shared<MdlColor4TypeSyntax>(this));

    registerTypeSyntax(
        Type::VECTOR2,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "float2",
            "float2(0.0)",
            "float2(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VECTOR2_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR3,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "float3",
            "float3(0.0)",
            "float3(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VECTOR3_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR4,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "float4",
            "float4(0.0)",
            "float4(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VECTOR4_MEMBERS));

    registerTypeSyntax(
        Type::MATRIX33,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "float3x3",
            "float3x3(1.0)",
            "float3x3(1.0)"));

    registerTypeSyntax(
        Type::MATRIX44,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "float4x4",
            "float4x4(1.0)",
            "float4x4(1.0)"));

    registerTypeSyntax(
        Type::STRING,
        std::make_shared<StringTypeSyntax>(
            this,
            "string",
            "\"\"",
            "\"\""));

    registerTypeSyntax(
        Type::FILENAME,
        std::make_shared<MdlFilenameTypeSyntax>(this));

    registerTypeSyntax(
        Type::BSDF,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "material",
            "material()",
            "material()"));

    registerTypeSyntax(
        Type::EDF,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "material",
            "material()",
            "material()"));

    registerTypeSyntax(
        Type::VDF,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "material",
            "material()",
            "material()"));

    registerTypeSyntax(
        Type::SURFACESHADER,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "material",
            "material()",
            "material()"));

    registerTypeSyntax(
        Type::VOLUMESHADER,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "material",
            "material()",
            "material()"));

    registerTypeSyntax(
        Type::DISPLACEMENTSHADER,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "material",
            "material()",
            "material()"));

    registerTypeSyntax(
        Type::LIGHTSHADER,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "material",
            "material()",
            "material()"));

    registerTypeSyntax(
        Type::MATERIAL,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "material",
            "material()",
            "material()"));

    registerTypeSyntax(
        Type::MDL_ADDRESSMODE,
        std::make_shared<MdlEnumSyntax>(
            this,
            "mx_addressmode_type",
            "mx_addressmode_type_periodic",
            "mx_addressmode_type_periodic",
            ADDRESSMODE_MEMBERS));

    registerTypeSyntax(
        Type::MDL_COORDINATESPACE,
        std::make_shared<MdlEnumSyntax>(
            this,
            "mx_coordinatespace_type",
            "mx_coordinatespace_type_model",
            "mx_coordinatespace_type_model",
            COORDINATESPACE_MEMBERS));

    registerTypeSyntax(
        Type::MDL_FILTERLOOKUPMODE,
        std::make_shared<MdlEnumSyntax>(
            this,
            "mx_filterlookup_type",
            "mx_filterlookup_type_linear",
            "mx_filterlookup_type_linear",
            FILTERLOOKUPMODE_MEMBERS));

    registerTypeSyntax(
        Type::MDL_FILTERTYPE,
        std::make_shared<MdlEnumSyntax>(
            this,
            "mx_filter_type",
            "mx_filter_type_gaussian",
            "mx_filter_type_gaussian",
            FILTERTYPE_MEMBERS));

    registerTypeSyntax(
        Type::MDL_DISTRIBUTIONTYPE,
        std::make_shared<MdlEnumSyntax>(
            this,
            "mx_distribution_type",
            "mx_distribution_type_ggx",
            "mx_distribution_type_ggx",
            DISTRIBUTIONTYPE_MEMBERS));

    registerTypeSyntax(
        Type::MDL_SCATTER_MODE,
        std::make_shared<MdlEnumSyntax>(
            this,
            "mx_scatter_mode",
            "mx_scatter_mode_R",
            "mx_scatter_mode_R",
            SCATTER_MODE_MEMBERS));

    registerTypeSyntax(
        Type::MDL_SHEEN_MODE,
        std::make_shared<MdlEnumSyntax>(
            this,
            "mx_sheen_mode",
            "mx_sheen_mode_conty_kulla",
            "mx_sheen_mode_conty_kulla",
            SHEEN_MODE_MEMBERS));
}

TypeDesc MdlSyntax::getEnumeratedType(const string& value) const
{
    for (const TypeSyntaxPtr& syntax : getTypeSyntaxes())
    {
        const StringVec& members = syntax->getMembers();
        if (members.size())
        {
            // TODO: This logic assumes the enum values are strictly unique among all enum types.
            // We should find a more safe way to handled this.
            if (std::find(members.begin(), members.end(), value) != members.end())
            {
                auto pos = std::find(_typeSyntaxes.begin(), _typeSyntaxes.end(), syntax);
                if (pos != _typeSyntaxes.end())
                {
                    const size_t index = static_cast<size_t>(std::distance(_typeSyntaxes.begin(), pos));
                    for (auto item : _typeSyntaxIndexByType)
                    {
                        if (item.second == index)
                        {
                            return item.first;
                        }
                    }
                }
            }
        }
    }
    return Type::NONE;
}

string MdlSyntax::getArrayTypeSuffix(TypeDesc type, const Value& value) const
{
    if (type.isArray())
    {
        if (value.isA<vector<float>>())
        {
            const size_t size = value.asA<vector<float>>().size();
            return "[" + std::to_string(size) + "]";
        }
        else if (value.isA<vector<int>>())
        {
            const size_t size = value.asA<vector<int>>().size();
            return "[" + std::to_string(size) + "]";
        }
    }
    return string();
}

bool MdlSyntax::remapEnumeration(const string& value, TypeDesc type, const string& enumNames, std::pair<TypeDesc, ValuePtr>& result) const
{
    // Early out if not an enum input.
    if (enumNames.empty())
    {
        return false;
    }

    // Don't convert filenames or arrays.
    if (type == Type::FILENAME || type.isArray())
    {
        return false;
    }

    // Try remapping to an enum value.
    if (!value.empty())
    {
        result.first = getEnumeratedType(value);
        if (result.first == Type::NONE || (result.first.getSemantic() != TypeDesc::Semantic::SEMANTIC_ENUM))
        {
            return false;
        }

        StringVec valueElemEnumsVec = splitString(enumNames, ",");
        for (size_t i = 0; i < valueElemEnumsVec.size(); i++)
        {
            valueElemEnumsVec[i] = trimSpaces(valueElemEnumsVec[i]);
        }
        auto pos = std::find(valueElemEnumsVec.begin(), valueElemEnumsVec.end(), value);
        if (pos == valueElemEnumsVec.end())
        {
            throw ExceptionShaderGenError("Given value '" + value + "' is not a valid enum value.");
        }
        const int index = static_cast<int>(std::distance(valueElemEnumsVec.begin(), pos));
        result.second = Value::createValue<string>(valueElemEnumsVec[index]);

        return true;
    }

    return false;
}

void MdlSyntax::makeValidName(string& name) const
{
    Syntax::makeValidName(name);

    // MDL variables are not allowed to begin with underscore.
    if (!name.empty() && name[0] == '_')
    {
        name = "v" + name;
    }
}

string MdlSyntax::modifyPortName(const string& word) const
{
    return PORT_NAME_PREFIX + word;
}

string MdlSyntax::replaceSourceCodeMarkers(const string& nodeName, const string& soureCode, std::function<string(const string&)> lambda) const
{
    // An inline function call
    // Replace tokens of the format "{{<var>}}"
    static const string prefix("{{");
    static const string postfix("}}");

    size_t pos = 0;
    size_t i = soureCode.find_first_of(prefix);
    StringVec code;
    while (i != string::npos)
    {
        code.push_back(soureCode.substr(pos, i - pos));
        size_t j = soureCode.find_first_of(postfix, i + 2);
        if (j == string::npos)
        {
            throw ExceptionShaderGenError("Malformed inline expression in implementation for node " + nodeName);
        }
        const string marker = soureCode.substr(i + 2, j - i - 2);
        code.push_back(lambda(marker));
        pos = j + 2;
        i = soureCode.find_first_of(prefix, pos);
    }
    code.push_back(soureCode.substr(pos));
    return joinStrings(code, EMPTY_STRING);
}

const string& MdlSyntax::getMdlVersionSuffixMarker() const
{
    return MARKER_MDL_VERSION_SUFFIX;
}

MATERIALX_NAMESPACE_END
