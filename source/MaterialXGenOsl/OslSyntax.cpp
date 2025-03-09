//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenOsl/OslSyntax.h>

#include <MaterialXGenShader/ShaderGenerator.h>

#include <sstream>

MATERIALX_NAMESPACE_BEGIN

namespace
{

class OslBooleanTypeSyntax : public ScalarTypeSyntax
{
  public:
    OslBooleanTypeSyntax(const Syntax* parent) :
        ScalarTypeSyntax(parent, "int", "0", "0", EMPTY_STRING, "#define true 1\n#define false 0")
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        return value.asA<bool>() ? "1" : "0";
    }
};

class OslArrayTypeSyntax : public ScalarTypeSyntax
{
  public:
    OslArrayTypeSyntax(const Syntax* parent, const string& name) :
        ScalarTypeSyntax(parent, name, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING)
    {
    }

    string getValue(const Value& value, bool uniform) const override
    {
        if (!isEmpty(value))
        {
            return "{" + value.getValueString() + "}";
        }
        // OSL disallows arrays without initialization when specified as input uniform
        else if (uniform)
        {
            throw ExceptionShaderGenError("Uniform array cannot initialize to a empty value.");
        }
        return EMPTY_STRING;
    }

  protected:
    virtual bool isEmpty(const Value& value) const = 0;
};

class OslFloatArrayTypeSyntax : public OslArrayTypeSyntax
{
  public:
    explicit OslFloatArrayTypeSyntax(const Syntax* parent, const string& name) :
        OslArrayTypeSyntax(parent, name)
    {
    }

  protected:
    bool isEmpty(const Value& value) const override
    {
        vector<float> valueArray = value.asA<vector<float>>();
        return valueArray.empty();
    }
};

class OslIntegerArrayTypeSyntax : public OslArrayTypeSyntax
{
  public:
    explicit OslIntegerArrayTypeSyntax(const Syntax* parent, const string& name) :
        OslArrayTypeSyntax(parent, name)
    {
    }

  protected:
    bool isEmpty(const Value& value) const override
    {
        vector<int> valueArray = value.asA<vector<int>>();
        return valueArray.empty();
    }
};

// In OSL vector2, vector4, and color4 are custom struct types and require a different
// value syntax for uniforms. So override the aggregate type syntax to support this.
class OslStructTypeSyntax : public AggregateTypeSyntax
{
  public:
    OslStructTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                        const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING,
                        const StringVec& members = EMPTY_MEMBERS) :
        AggregateTypeSyntax(parent, name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, members)
    {
    }

    string getValue(const Value& value, bool uniform) const override
    {
        if (uniform)
        {
            return "{" + value.getValueString() + "}";
        }
        else
        {
            return getName() + "(" + value.getValueString() + ")";
        }
    }
};

// For the color4 type we need even more specialization since it's a struct of a struct:
//
// struct color4 {
//    color rgb;
//    float a;
// }
//
class OslColor4TypeSyntax : public OslStructTypeSyntax
{
  public:
    OslColor4TypeSyntax(const Syntax* parent) :
        OslStructTypeSyntax(parent, "color4", "color4(color(0.0), 0.0)", "{color(0.0), 0.0}", EMPTY_STRING, EMPTY_STRING, OslSyntax::COLOR4_MEMBERS)
    {
    }

    string getValue(const Value& value, bool uniform) const override
    {
        StringStream ss;

        // Set float format and precision for the stream
        const Value::FloatFormat fmt = Value::getFloatFormat();
        ss.setf(std::ios_base::fmtflags(
                    (fmt == Value::FloatFormatFixed ? std::ios_base::fixed : (fmt == Value::FloatFormatScientific ? std::ios_base::scientific : 0))),
                std::ios_base::floatfield);
        ss.precision(Value::getFloatPrecision());

        const Color4 c = value.asA<Color4>();

        if (uniform)
        {
            ss << "{color(" << c[0] << ", " << c[1] << ", " << c[2] << "), " << c[3] << "}";
        }
        else
        {
            ss << "color4(color(" << c[0] << ", " << c[1] << ", " << c[2] << "), " << c[3] << ")";
        }

        return ss.str();
    }
};

class OSLMatrix3TypeSyntax : public AggregateTypeSyntax
{
  public:
    OSLMatrix3TypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                         const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING,
                         const StringVec& members = EMPTY_MEMBERS) :
        AggregateTypeSyntax(parent, name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, members)
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        StringVec values = splitString(value.getValueString(), ",");
        if (values.empty())
        {
            throw ExceptionShaderGenError("No values given to construct a value");
        }

        // Write the value using a stream to maintain any float formatting set
        // using Value::setFloatFormat() and Value::setFloatPrecision()
        StringStream ss;
        ss << getName() << "(";
        for (size_t i = 0; i < values.size(); i++)
        {
            ss << values[i] << ", ";
            if ((i + 1) % 3 == 0)
            {
                ss << "0.000"
                   << ", ";
            }
        }
        static string ROW_4("0.000, 0.000, 0.000, 1.000");
        ss << ROW_4 << ")";

        return ss.str();
    }
};

class OSLFilenameTypeSyntax : public AggregateTypeSyntax
{
  public:
    OSLFilenameTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                          const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING,
                          const StringVec& members = EMPTY_MEMBERS) :
        AggregateTypeSyntax(parent, name, defaultValue, uniformDefaultValue, typeAlias, typeDefinition, members)
    {
    }

    string getValue(const ShaderPort* port, bool uniform) const override
    {
        if (!port)
        {
            return EMPTY_STRING;
        }

        const string prefix = uniform ? "{" : getName() + "(";
        const string suffix = uniform ? "}" : ")";
        const string filename = port->getValue() ? port->getValue()->getValueString() : EMPTY_STRING;
        return prefix + "\"" + filename + "\", \"" + port->getColorSpace() + "\"" + suffix;
    }

    string getValue(const Value& value, bool uniform) const override
    {
        const string prefix = uniform ? "{" : getName() + "(";
        const string suffix = uniform ? "}" : ")";
        return prefix + "\"" + value.getValueString() + "\", \"\"" + suffix;
    }
};

} // anonymous namespace

const string OslSyntax::OUTPUT_QUALIFIER = "output";
const string OslSyntax::SOURCE_FILE_EXTENSION = ".osl";
const StringVec OslSyntax::VECTOR_MEMBERS = { "[0]", "[1]", "[2]" };
const StringVec OslSyntax::VECTOR2_MEMBERS = { ".x", ".y" };
const StringVec OslSyntax::VECTOR4_MEMBERS = { ".x", ".y", ".z", ".w" };
const StringVec OslSyntax::COLOR4_MEMBERS = { ".rgb[0]", ".rgb[1]", ".rgb[2]", ".a" };

//
// OslSyntax methods
//

OslSyntax::OslSyntax(TypeSystemPtr typeSystem) : Syntax(typeSystem)
{
    // Add in all reserved words and keywords in OSL
    registerReservedWords(
        { // OSL types and keywords
          "and", "break", "closure", "color", "continue", "do", "else", "emit", "float", "for", "if", "illuminance",
          "illuminate", "int", "matrix", "normal", "not", "or", "output", "point", "public", "return", "string",
          "struct", "vector", "void", "while",
          "bool", "case", "catch", "char", "class", "const", "delete", "default", "double", "enum", "extern",
          "false", "friend", "goto", "inline", "long", "new", "operator", "private", "protected", "short",
          "signed", "sizeof", "static", "switch", "template", "this", "throw", "true", "try", "typedef", "uniform",
          "union", "unsigned", "varying", "virtual", "volatile",
          // OSL standard library functions names
          "degrees", "radians", "cos", "sin", "tan", "acos", "asin", "atan", "atan2", "cosh", "sinh", "tanh",
          "pow", "log", "log2", "log10", "logb", "sqrt", "inversesqrt", "cbrt", "hypot", "abs", "fabs", "sign",
          "floor", "ceil", "round", "trunc", "fmod", "mod", "min", "max", "clamp", "mix", "select", "isnan",
          "isinf", "isfinite", "erf", "erfc", "cross", "dot", "length", "distance", "normalize", "faceforward",
          "reflect", "fresnel", "transform", "transformu", "rotate", "luminance", "blackbody", "wavelength_color",
          "transformc", "determinant", "transpose", "step", "smoothstep", "linearstep", "smooth_linearstep", "aastep",
          "hash", "strlen", "getchar", "startswith", "endswith", "substr", "stof", "stoi", "concat", "textureresource",
          "backfacing", "raytype", "iscameraray", "isdiffuseray", "isglossyray", "isshadowray", "getmatrix",
          "emission", "background", "diffuse", "oren_nayer", "translucent", "phong", "ward", "microfacet",
          "reflection", "transparent", "debug", "holdout", "subsurface", "sheen",
          "oren_nayar_diffuse_bsdf", "burley_diffuse_bsdf", "dielectric_bsdf", "conductor_bsdf", "generalized_schlick_bsdf",
          "translucent_bsdf", "transparent_bsdf", "subsurface_bssrdf", "sheen_bsdf", "uniform_edf", "anisotropic_vdf",
          "medium_vdf", "layer", "artistic_ior" });

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
        std::make_shared<OslFloatArrayTypeSyntax>(
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
        std::make_shared<OslIntegerArrayTypeSyntax>(
            this,
            "int"));

    registerTypeSyntax(
        Type::BOOLEAN,
        std::make_shared<OslBooleanTypeSyntax>(this));

    registerTypeSyntax(
        // Note: the color type in OSL is a built in type and
        // should not use the custom OslStructTypeSyntax.
        Type::COLOR3,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "color",
            "color(0.0)",
            "color(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VECTOR_MEMBERS));

    registerTypeSyntax(
        Type::COLOR4,
        std::make_shared<OslColor4TypeSyntax>(this));

    registerTypeSyntax(
        Type::VECTOR2,
        std::make_shared<OslStructTypeSyntax>(
            this,
            "vector2",
            "vector2(0.0, 0.0)",
            "{0.0, 0.0}",
            EMPTY_STRING,
            EMPTY_STRING,
            VECTOR2_MEMBERS));

    registerTypeSyntax(
        // Note: the vector type in OSL is a built in type and
        // should not use the custom OslStructTypeSyntax.
        Type::VECTOR3,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "vector",
            "vector(0.0)",
            "vector(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VECTOR_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR4,
        std::make_shared<OslStructTypeSyntax>(
            this,
            "vector4",
            "vector4(0.0, 0.0, 0.0, 0.0)",
            "{0.0, 0.0, 0.0, 0.0}",
            EMPTY_STRING,
            EMPTY_STRING,
            VECTOR4_MEMBERS));

    registerTypeSyntax(
        Type::MATRIX33,
        std::make_shared<OSLMatrix3TypeSyntax>(
            this,
            "matrix",
            "matrix(1.0)",
            "matrix(1.0)"));

    registerTypeSyntax(
        Type::MATRIX44,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "matrix",
            "matrix(1.0)",
            "matrix(1.0)"));

    registerTypeSyntax(
        Type::STRING,
        std::make_shared<StringTypeSyntax>(
            this,
            "string",
            "\"\"",
            "\"\""));

    registerTypeSyntax(
        Type::FILENAME,
        std::make_shared<OSLFilenameTypeSyntax>(
            this,
            "textureresource ",
            "textureresource (\"\", \"\")",
            "(\"\", \"\")",
            EMPTY_STRING,
            "struct textureresource { string filename; string colorspace; };"));

    registerTypeSyntax(
        Type::BSDF,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "BSDF",
            "null_closure",
            "0",
            "closure color",
            "#define BSDF closure color"));

    registerTypeSyntax(
        Type::EDF,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "EDF",
            "null_closure",
            "0",
            "closure color",
            "#define EDF closure color"));

    registerTypeSyntax(
        Type::VDF,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "VDF",
            "null_closure",
            "0",
            "closure color",
            "#define VDF closure color"));

    registerTypeSyntax(
        Type::SURFACESHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "surfaceshader",
            "surfaceshader(null_closure, null_closure, 1.0)",
            "{ 0, 0, 1.0 }",
            "closure color",
            "struct surfaceshader { closure color bsdf; closure color edf; float opacity; };"));

    registerTypeSyntax(
        Type::VOLUMESHADER,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "volumeshader",
            "null_closure",
            "0",
            "closure color",
            "#define volumeshader closure color"));

    registerTypeSyntax(
        Type::DISPLACEMENTSHADER,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "displacementshader",
            "vector(0.0)",
            "vector(0.0)",
            "vector",
            "#define displacementshader vector"));

    registerTypeSyntax(
        Type::LIGHTSHADER,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "lightshader",
            "null_closure",
            "0",
            "closure color",
            "#define lightshader closure color"));

    registerTypeSyntax(
        Type::MATERIAL,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "MATERIAL",
            "null_closure",
            "0",
            "closure color",
            "#define MATERIAL closure color"));
}

const string& OslSyntax::getOutputQualifier() const
{
    return OUTPUT_QUALIFIER;
}

MATERIALX_NAMESPACE_END
