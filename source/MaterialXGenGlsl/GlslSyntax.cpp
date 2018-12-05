#include <MaterialXGenGlsl/GlslSyntax.h>
#include <MaterialXGenShader/Shader.h>

#include <MaterialXGenShader/TypeDesc.h>

#include <memory>

namespace MaterialX
{

namespace
{
    // Since GLSL doesn't support strings we use integers instead.
    // TODO: Support options strings by converting to a corresponding enum integer
    class GlslStringTypeSyntax : public StringTypeSyntax
    {
    public:
        GlslStringTypeSyntax() : StringTypeSyntax("int", "0", "0") {}

        string getValue(const Value& /*value*/, bool /*uniform*/) const override
        {
            return "0";
        }
    };

    class GlslArrayTypeSyntax : public ScalarTypeSyntax
    {
    public:
        GlslArrayTypeSyntax(const string& name)
            : ScalarTypeSyntax(name, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING)
        {}

        string getValue(const Value& value, bool /*uniform*/) const override
        {
            size_t arraySize = getSize(value);
            if (arraySize > 0)
            {
                return _name + "[" + std::to_string(arraySize) + "](" + value.getValueString() + ")";
            }
            return EMPTY_STRING;
        }

        string getValue(const vector<string>& values, bool /*uniform*/) const override
        {
            if (values.empty())
            {
                throw ExceptionShaderGenError("No values given to construct an array value");
            }

            string result = _name + "[" + std::to_string(values.size()) + "](" + values[0];
            for (size_t i = 1; i<values.size(); ++i)
            {
                result += ", " + values[i];
            }
            result += ")";

            return result;
        }

    protected:
        virtual size_t getSize(const Value& value) const = 0;
    };

    class GlslFloatArrayTypeSyntax : public GlslArrayTypeSyntax
    {
    public:
        GlslFloatArrayTypeSyntax(const string& name)
            : GlslArrayTypeSyntax(name)
        {}        

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
        GlslIntegerArrayTypeSyntax(const string& name)
            : GlslArrayTypeSyntax(name)
        {}        

    protected:
        size_t getSize(const Value& value) const override
        {
            vector<int> valueArray = value.asA<vector<int>>();
            return valueArray.size();
        }

    };
}

const string GlslSyntax::OUTPUT_QUALIFIER = "out";
const string GlslSyntax::UNIFORM_QUALIFIER = "uniform";
const string GlslSyntax::CONSTANT_QUALIFIER = "const";
const vector<string> GlslSyntax::VEC2_MEMBERS = { ".x", ".y" };
const vector<string> GlslSyntax::VEC3_MEMBERS = { ".x", ".y", ".z" };
const vector<string> GlslSyntax::VEC4_MEMBERS = { ".x", ".y", ".z", ".w" };

GlslSyntax::GlslSyntax()
{
    // Add in all restricted names and keywords in GLSL
    registerRestrictedNames(
    {
        "centroid", "flat", "smooth", "noperspective", "patch", "sample",
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
        "sizeof", "cast", "namespace", "using", "row_major"
    });

    //
    // Register syntax handlers for each data type.
    //

    registerTypeSyntax
    (
        Type::FLOAT,
        std::make_shared<ScalarTypeSyntax>(
            "float", 
            "0.0", 
            "0.0")
    );

    registerTypeSyntax
    (
        Type::FLOATARRAY,
        std::make_shared<GlslFloatArrayTypeSyntax>(
            "float")
    );

    registerTypeSyntax
    (
        Type::INTEGER,
        std::make_shared<ScalarTypeSyntax>(
            "int", 
            "0", 
            "0")
    );

    registerTypeSyntax
    (
        Type::INTEGERARRAY,
        std::make_shared<GlslIntegerArrayTypeSyntax>(
            "int")
    );

    registerTypeSyntax
    (
        Type::BOOLEAN,
        std::make_shared<ScalarTypeSyntax>(
            "bool", 
            "false", 
            "false")
    );

    registerTypeSyntax
    (
        Type::COLOR2,
        std::make_shared<AggregateTypeSyntax>(
            "vec2", 
            "vec2(0.0)", 
            "vec2(0.0)", 
            EMPTY_STRING,
            EMPTY_STRING,
            VEC2_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::COLOR3,
        std::make_shared<AggregateTypeSyntax>(
            "vec3", 
            "vec3(0.0)", 
            "vec3(0.0)", 
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::COLOR4,
        std::make_shared<AggregateTypeSyntax>(
            "vec4", 
            "vec4(0.0)", 
            "vec4(0.0)", 
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::VECTOR2,
        std::make_shared<AggregateTypeSyntax>(
            "vec2", 
            "vec2(0.0)", 
            "vec2(0.0)", 
            EMPTY_STRING, 
            EMPTY_STRING, 
            VEC2_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::VECTOR3,
        std::make_shared<AggregateTypeSyntax>(
            "vec3", 
            "vec3(0.0)", 
            "vec3(0.0)", 
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::VECTOR4,
        std::make_shared<AggregateTypeSyntax>(
            "vec4", 
            "vec4(0.0)", 
            "vec4(0.0)", 
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS)
    );

    registerTypeSyntax
    (
        Type::MATRIX33,
        std::make_shared<AggregateTypeSyntax>(
            "mat3", 
            "mat3(1.0)", 
            "mat3(1.0)")
    );

    registerTypeSyntax
    (
        Type::MATRIX44,
        std::make_shared<AggregateTypeSyntax>(
            "mat4", 
            "mat4(1.0)", 
            "mat4(1.0)")
    );

    registerTypeSyntax
    (
        Type::STRING,
        std::make_shared<GlslStringTypeSyntax>()
    );

    registerTypeSyntax
    (
        Type::FILENAME,
        std::make_shared<ScalarTypeSyntax>(
            "sampler2D", 
            EMPTY_STRING, 
            EMPTY_STRING)
    );

    registerTypeSyntax
    (
        Type::BSDF,
        std::make_shared<AggregateTypeSyntax>(
            "BSDF", 
            "BSDF(0.0)", 
            "BSDF(0.0)", 
            "vec3")
    );

    registerTypeSyntax
    (
        Type::EDF,
        std::make_shared<AggregateTypeSyntax>(
            "EDF", 
            "EDF(0.0)", 
            "EDF(0.0)", 
            "vec3")
    );

    registerTypeSyntax
    (
        Type::VDF,
        std::make_shared<AggregateTypeSyntax>(
            "VDF", 
            "VDF(vec3(0.0),vec3(0.0))", 
            EMPTY_STRING, 
            EMPTY_STRING,
            "struct VDF { vec3 absorption; vec3 scattering; };")
    );

    registerTypeSyntax
    (
        Type::ROUGHNESSINFO,
        std::make_shared<AggregateTypeSyntax>(
            "roughnessinfo",
            "roughnessinfo(0.0, 0.0, 0.0, 0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct roughnessinfo { float roughness; float alpha; float alphaX; float alphaY; };")
    );

    registerTypeSyntax
    (
        Type::SURFACESHADER,
        std::make_shared<AggregateTypeSyntax>(
            "surfaceshader", 
            "surfaceshader(vec3(0.0),vec3(0.0))", 
            EMPTY_STRING,
            EMPTY_STRING,
            "struct surfaceshader { vec3 color; vec3 transparency; };")
    );

    registerTypeSyntax
    (
        Type::VOLUMESHADER,
        std::make_shared<AggregateTypeSyntax>(
            "volumeshader",
            "volumeshader(VDF(vec3(0.0),vec3(0.0)),EDF(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct volumeshader { VDF vdf; EDF edf; };")
    );

    registerTypeSyntax
    (
        Type::DISPLACEMENTSHADER,
        std::make_shared<AggregateTypeSyntax>(
            "displacementshader",
            "displacementshader(vec3(0.0),1.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct displacementshader { vec3 offset; float scale; };")
    );

    registerTypeSyntax
    (
        Type::LIGHTSHADER,
        std::make_shared<AggregateTypeSyntax>(
            "lightshader", 
            "lightshader(vec3(0.0),vec3(0.0))", 
            EMPTY_STRING,
            EMPTY_STRING,
            "struct lightshader { vec3 intensity; vec3 direction; };")
    );
}

const string& GlslSyntax::getOutputQualifier() const
{
    return OUTPUT_QUALIFIER;
}

bool GlslSyntax::typeSupported(const TypeDesc* type) const
{ 
    return type != Type::STRING; 
}

}
