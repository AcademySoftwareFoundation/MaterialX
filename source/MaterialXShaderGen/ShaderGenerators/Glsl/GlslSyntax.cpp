#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslSyntax.h>

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
}


const string GlslSyntax::OUTPUT_QUALIFIER = "out";
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
            "bool", 
            "false", 
            "false")
    );

    registerTypeSyntax
    (
        DataType::COLOR2,
        std::make_unique<AggregateTypeSyntax>(
            "vec2", 
            "vec2(0.0)", 
            "vec2(0.0)", 
            EMPTY_STRING, 
            VEC2_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::COLOR3,
        std::make_unique<AggregateTypeSyntax>(
            "vec3", 
            "vec3(0.0)", 
            "vec3(0.0)", 
            EMPTY_STRING, 
            VEC3_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::COLOR4,
        std::make_unique<AggregateTypeSyntax>(
            "vec4", 
            "vec4(0.0)", 
            "vec4(0.0)", 
            EMPTY_STRING, 
            VEC4_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::VECTOR2,
        std::make_unique<AggregateTypeSyntax>(
            "vec2", 
            "vec2(0.0)", 
            "vec2(0.0)", 
            EMPTY_STRING, 
            VEC2_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::VECTOR3,
        std::make_unique<AggregateTypeSyntax>(
            "vec3", 
            "vec3(0.0)", 
            "vec3(0.0)", 
            EMPTY_STRING, 
            VEC3_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::VECTOR4,
        std::make_unique<AggregateTypeSyntax>(
            "vec4", 
            "vec4(0.0)", 
            "vec4(0.0)", 
            EMPTY_STRING, 
            VEC4_MEMBERS)
    );

    registerTypeSyntax
    (
        DataType::MATRIX3,
        std::make_unique<AggregateTypeSyntax>(
            "mat3", 
            "mat3(1.0)", 
            "mat3(1.0)")
    );

    registerTypeSyntax
    (
        DataType::MATRIX4,
        std::make_unique<AggregateTypeSyntax>(
            "mat4", 
            "mat4(1.0)", 
            "mat4(1.0)")
    );

    registerTypeSyntax
    (
        DataType::STRING,
        std::make_unique<GlslStringTypeSyntax>()
    );

    registerTypeSyntax
    (
        DataType::FILENAME,
        std::make_unique<ScalarTypeSyntax>(
            "sampler2D", 
            EMPTY_STRING, 
            EMPTY_STRING)
    );

    registerTypeSyntax
    (
        DataType::BSDF,
        std::make_unique<AggregateTypeSyntax>(
            "BSDF", 
            "BSDF(0.0)", 
            "BSDF(0.0)", 
            "#define BSDF vec3")
    );

    registerTypeSyntax
    (
        DataType::EDF,
        std::make_unique<AggregateTypeSyntax>(
            "EDF", 
            "EDF(0.0)", 
            "EDF(0.0)", 
            "#define EDF vec3")
    );

    registerTypeSyntax
    (
        DataType::VDF,
        std::make_unique<AggregateTypeSyntax>(
            "VDF", 
            "VDF(vec3(0.0),vec3(0.0))", 
            EMPTY_STRING, 
            "struct VDF { vec3 absorption; vec3 scattering; };")
    );

    registerTypeSyntax
    (
        DataType::SURFACE,
        std::make_unique<AggregateTypeSyntax>(
            "surfaceshader", 
            "surfaceshader(vec3(0.0),vec3(0.0))", 
            EMPTY_STRING,
            "struct surfaceshader { vec3 color; vec3 transparency; };")
    );

    registerTypeSyntax
    (
        DataType::VOLUME,
        std::make_unique<AggregateTypeSyntax>(
            "volumeshader",
            "volumeshader(VDF(vec3(0.0),vec3(0.0)),EDF(0.0))",
            EMPTY_STRING,
            "struct volumeshader { VDF vdf; EDF edf; };")
    );

    registerTypeSyntax
    (
        DataType::DISPLACEMENT,
        std::make_unique<AggregateTypeSyntax>(
            "displacementshader",
            "displacementshader(vec3(0.0),1.0)",
            EMPTY_STRING,
            "struct displacementshader { vec3 offset; float scale; };")
    );

    registerTypeSyntax
    (
        DataType::LIGHT,
        std::make_unique<AggregateTypeSyntax>(
            "lightshader", 
            "lightshader(vec3(0.0),vec3(0.0))", 
            EMPTY_STRING, 
            "struct lightshader { vec3 intensity; vec3 direction; };")
    );
}

const string& GlslSyntax::getOutputQualifier() const
{
    return OUTPUT_QUALIFIER;
}

}
