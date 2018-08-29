#include <MaterialXShaderGen/ShaderGenerators/Osl/OslSyntax.h>
#include <MaterialXShaderGen/Shader.h>

#include <memory>
#include <sstream>

namespace MaterialX
{

namespace
{
    // In OSL vector2, vector4, color2 and color4 are custom struct types and require a different 
    // value syntax for uniforms. So override the aggregate type syntax to support this.
    class OslStructTypeSyntax : public AggregateTypeSyntax
    {
    public:
        OslStructTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue,
            const string& typeDefStatement = EMPTY_STRING, const vector<string>& members = EMPTY_MEMBERS)
            : AggregateTypeSyntax(name, defaultValue, uniformDefaultValue, typeDefStatement, members)
        {}

        string OslStructTypeSyntax::getValue(const Value& value, bool uniform) const override
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

        string OslStructTypeSyntax::getValue(const vector<string>& values, bool uniform) const override
        {
            if (values.empty())
            {
                throw ExceptionShaderGenError("No values given to construct a value");
            }

            string result = uniform ? "{" : getName() + "(" + values[0];
            for (size_t i = 1; i<values.size(); ++i)
            {
                result += ", " + values[i];
            }
            result += uniform ? "}" : ")";

            return result;
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
        OslColor4TypeSyntax() 
            : OslStructTypeSyntax("color4", "color4(color(0.0), 0.0)", "{color(0.0), 0.0}", EMPTY_STRING, OslSyntax::COLOR4_MEMBERS)
        {}

        string getValue(const Value& value, bool uniform) const override
        {
            std::stringstream ss;

            // Set float format and precision for the stream
            const Value::FloatFormat fmt = Value::getFloatFormat();
            ss.setf(std::ios_base::fmtflags(
                (fmt == Value::FloatFormatFixed ? std::ios_base::fixed :
                (fmt == Value::FloatFormatScientific ? std::ios_base::scientific : 0))),
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

        string getValue(const vector<string>& values, bool uniform) const override
        {
            if (values.size() < 4)
            {
                throw ExceptionShaderGenError("Too few values given to construct a color4 value");
            }

            if (uniform)
            {
                return "{color(" + values[0] + ", " + values[1] + ", " + values[2] + "), " + values[3] + "}";
            }
            else
            {
                return "color4(color(" + values[0] + ", " + values[1] + ", " + values[2] + "), " + values[3] + ")";
            }
        }
    };
}

const string OslSyntax::OUTPUT_QUALIFIER = "output";
const vector<string> OslSyntax::VECTOR_MEMBERS  = { "[0]", "[1]", "[2]" };
const vector<string> OslSyntax::VECTOR2_MEMBERS = { ".x", ".y" };
const vector<string> OslSyntax::VECTOR4_MEMBERS = { ".x", ".y", ".z", ".w" };
const vector<string> OslSyntax::COLOR2_MEMBERS  = { ".r", ".a" };
const vector<string> OslSyntax::COLOR4_MEMBERS  = { ".rgb[0]", ".rgb[1]", ".rgb[2]", ".a" };

OslSyntax::OslSyntax()
{
    // Add in all restricted names and keywords in OSL
    registerRestrictedNames(
    {
        "and", "break", "closure", "color", "continue", "do", "else", "emit", "float", "for", "if", "illuminance",
        "illuminate", "int", "matrix", "normal", "not", "or", "output", "point", "public", "return", "string",
        "struct", "vector", "void", "while",
        "bool", "case", "catch", "char", "class", "const", "delete", "default", "double", "enum", "extern",
        "false", "friend", "goto", "inline", "long", "new", "operator", "private", "protected", "short",
        "signed", "sizeof", "static", "switch", "template", "this", "throw", "true", "try", "typedef", "uniform",
        "union", "unsigned", "varying", "virtual", "volatile",
        "emission"
    });

    //
    // Register type syntax handlers for each data type.
    //

    registerTypeSyntax
    (
        DataType::FLOAT,
        std::make_unique<ScalarTypeSyntax>(
            "float", 
            "0.0", 
            "0.0")
    );

    registerTypeSyntax
    (
        DataType::INTEGER,
        std::make_unique<ScalarTypeSyntax>(
            "int", 
            "0", 
            "0")
    );

    registerTypeSyntax
    (
        DataType::BOOLEAN,
        std::make_unique<ScalarTypeSyntax>(
            "int", 
            "0", 
            "0", 
            "#define true 1\n#define false 0")
    );

    registerTypeSyntax
    (
        DataType::COLOR2,
        std::make_unique<OslStructTypeSyntax>(
            "color2", 
            "color2(0.0, 0.0)", 
            "{0.0, 0.0}", 
            EMPTY_STRING,
            COLOR2_MEMBERS)
    );

    registerTypeSyntax
    (
        // Note: the color type in OSL is a built in type and 
        // should not use the custom OslStructTypeSyntax.
        DataType::COLOR3,
        std::make_unique<AggregateTypeSyntax>(
            "color", 
            "color(0.0)", 
            "color(0.0)",
            EMPTY_STRING,
            VECTOR_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::COLOR4,
        std::make_unique<OslColor4TypeSyntax>()
    );

    registerTypeSyntax
    (
        DataType::VECTOR2,
        std::make_unique<OslStructTypeSyntax>(
            "vector2", 
            "vector2(0.0, 0.0)", 
            "{0.0, 0.0}",
            EMPTY_STRING,
            VECTOR2_MEMBERS)
    );

    registerTypeSyntax
    (
        // Note: the vector type in OSL is a built in type and 
        // should not use the custom OslStructTypeSyntax.
        DataType::VECTOR3,
        std::make_unique<AggregateTypeSyntax>(
            "vector", 
            "vector(0.0)", 
            "vector(0.0)",
            EMPTY_STRING,
            VECTOR_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::VECTOR4,
        std::make_unique<OslStructTypeSyntax>(
            "vector4", 
            "vector4(0.0, 0.0, 0.0, 0.0)", 
            "{0.0, 0.0, 0.0, 0.0}",
            EMPTY_STRING,
            VECTOR4_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::MATRIX3,
        std::make_unique<AggregateTypeSyntax>(
            "matrix", 
            "matrix(1.0)", 
            "matrix(1.0)")
    );

    registerTypeSyntax
    (
        DataType::MATRIX4,
        std::make_unique<AggregateTypeSyntax>(
            "matrix", 
            "matrix(1.0)", 
            "matrix(1.0)")
    );

    registerTypeSyntax
    (
        DataType::STRING,
        std::make_unique<StringTypeSyntax>(
            "string", 
            "\"\"", 
            "\"\"")
    );

    registerTypeSyntax
    (
        DataType::FILENAME,
        std::make_unique<StringTypeSyntax>(
            "string", 
            "\"\"", 
            "\"\"")
    );

    registerTypeSyntax
    (
        DataType::BSDF,
        std::make_unique<ScalarTypeSyntax>(
            "BSDF", 
            "null_closure", 
            "0", 
            "#define BSDF closure color")
    );

    registerTypeSyntax
    (
        DataType::EDF,
        std::make_unique<ScalarTypeSyntax>(
            "EDF", 
            "null_closure", 
            "0", 
            "#define EDF closure color")
    );

    registerTypeSyntax
    (
        DataType::VDF,
        std::make_unique<ScalarTypeSyntax>(
            "VDF", 
            "null_closure", 
            "0", 
            "#define VDF closure color")
    );

    registerTypeSyntax
    (
        DataType::SURFACE,
        std::make_unique<ScalarTypeSyntax>(
            "surfaceshader", 
            "null_closure", 
            "0", 
            "#define surfaceshader closure color")
    );

    registerTypeSyntax
    (
        DataType::VOLUME,
        std::make_unique<ScalarTypeSyntax>(
            "volumeshader", 
            "null_closure", 
            "0", 
            "#define volumeshader closure color")
    );

    registerTypeSyntax
    (
        DataType::DISPLACEMENT, 
        std::make_unique<OslStructTypeSyntax>(
            "displacementshader", 
            "{vector(0.0), 0.0}", 
            "{vector(0.0), 0.0}", 
            "struct displacementshader { vector offset; float scale; };")
    );

    registerTypeSyntax
    (
        DataType::LIGHT,
        std::make_unique<ScalarTypeSyntax>(
            "lightshader", 
            "null_closure", 
            "0", 
            "#define lightshader closure color")
    );
}

const string& OslSyntax::getOutputQualifier() const
{
    return OUTPUT_QUALIFIER;
}

} // namespace MaterialX
