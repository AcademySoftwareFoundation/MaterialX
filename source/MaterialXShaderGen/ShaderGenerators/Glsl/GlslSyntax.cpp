#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslSyntax.h>

namespace MaterialX
{

GlslSyntax::GlslSyntax()
{
    // Add in all restricted names and keywords in GLSL
    addRestrictedNames(
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
    // Add syntax information for each data type.
    //
    // TODO: Make this setup data driven (e.g read from a json file),
    //       to support new types without requiring a rebuild.
    //

    addTypeSyntax
    (
        DataType::FLOAT,
        TypeSyntax
        (
            "float",     // type name
            "0.0",       // default value
            "0.0",       // default value in a shader param initialization context
            "",          // custom type definition to add in source code
            "out float"  // type name in output context
        )
    );

    addTypeSyntax
    (
        DataType::INTEGER,
        TypeSyntax
        (
            "int", 
            "0", 
            "0",
            "",
            "out int"
        )
    );

    addTypeSyntax
    (
        DataType::BOOLEAN,
        TypeSyntax
        (
            "bool", 
            "false", 
            "false",
            "",
            "out bool"
        )
    );

    addTypeSyntax
    (
        DataType::COLOR2,
        TypeSyntax
        (
            "vec2", 
            "vec2(0.0)",
            "vec2(0.0)",
            "",
            "out vec2"
        )
    );

    addTypeSyntax
    (
        DataType::COLOR3,
        TypeSyntax
        (
            "vec3", 
            "vec3(0.0)",
            "vec3(0.0)",
            "",
            "out vec3"
        )
    );

    addTypeSyntax
    (
        DataType::COLOR4,
        TypeSyntax
        (
            "vec4",
            "vec4(0.0)",
            "vec4(0.0)",
            "",
            "out vec4"
        )
    );

    addTypeSyntax
    (
        DataType::VECTOR2,
        TypeSyntax
        (
            "vec2",
            "vec2(0.0)",
            "vec2(0.0)",
            "",
            "out vec2"
        )
    );

    addTypeSyntax
    (
        DataType::VECTOR3,
        TypeSyntax
        (
            "vec3",
            "vec3(0.0)",
            "vec3(0.0)",
            "",
            "out vec3"
        )
    );

    addTypeSyntax
    (
        DataType::VECTOR4,
        TypeSyntax
        (
            "vec4",
            "vec4(0.0)",
            "vec4(0.0)",
            "",
            "out vec4"
        )
    );

    addTypeSyntax
    (
        DataType::MATRIX3,
        TypeSyntax
        (
            "mat3",
            "mat3(1.0)",
            "mat3(1.0)",
            "",
            "out mat3"
        )
    );

    addTypeSyntax
    (
        DataType::MATRIX4,
        TypeSyntax
        (
            "mat4",
            "mat4(1.0)",
            "mat4(1.0)",
            "",
            "out mat4"
        )
    );

    addTypeSyntax
    (
        DataType::STRING,
        TypeSyntax
        (
            "int", 
            "0", 
            "0",
            "",
            "out int"
        )
    );

    addTypeSyntax
    (
        DataType::FILENAME,
        TypeSyntax
        (
            "sampler2D", 
            "", 
            "",
            "",
            "out sampler2D"
        )
    );

    addTypeSyntax
    (
        DataType::BSDF,
        TypeSyntax
        (
            "BSDF",
            "BSDF(vec3(0.0),vec3(0.0))",
            "",
            "struct BSDF { vec3 fr; vec3 ft; };",
            "out BSDF"
        )
    );

    addTypeSyntax
    (
        DataType::EDF,
        TypeSyntax
        (
            "EDF",
            "EDF(0.0)",
            "",
            "#define EDF vec3",
            "out EDF"
        )
    );

    addTypeSyntax
    (
        DataType::VDF,
        TypeSyntax
        (
            "VDF",
            "VDF(vec3(0.0),vec3(0.0))",
            "",
            "struct VDF { vec3 absorption; vec3 scattering; };",
            "out VDF"
        )
    );

    addTypeSyntax
    (
        DataType::SURFACE,
        TypeSyntax
        (
            "surfaceshader",
            "surfaceshader(vec3(0.0),vec3(0.0))",
            "",
            "struct surfaceshader { vec3 color; vec3 transparency; };",
            "out surfaceshader"
        )
    );

    addTypeSyntax
    (
        DataType::VOLUME,
        TypeSyntax
        (
            "volumeshader", 
            "volumeshader(VDF(vec3(0.0),vec3(0.0)),vec3(0.0))",
            "",
            "struct volumeshader { VDF vdf; vec3 edf; };",
            "out volumeshader"
        )
    );

    addTypeSyntax
    (
        DataType::DISPLACEMENT,
        TypeSyntax
        (
            "displacementshader", 
            "displacementshader(vec3(0.0),1.0)",
            "",
            "struct displacementshader { vec3 offset; float scale; };",
            "out displacementshader"
        )
    );

    addTypeSyntax
    (
        DataType::LIGHT,
        TypeSyntax
        (
            "lightshader",
            "lightshader(vec3(0.0),vec3(0.0))",
            "",
            "struct lightshader { vec3 intensity; vec3 direction; };",
            "out lightshader"
        )
    );

    //
    // Add value constructor syntax for data types that needs this
    //

    addValueConstructSyntax(
        DataType::COLOR2,
        ValueConstructSyntax(
            "vec2(", ")", // Value constructor syntax
            "vec2(", ")", // Value constructor syntax in a shader param initialization context
            {".r", ".g"}  // Syntax for each vector component
        )
    );

    addValueConstructSyntax
    (
        DataType::COLOR3,
        ValueConstructSyntax
        (
            "vec3(", ")",
            "vec3(", ")",
            {".r", ".g", ".b"}
    )
    );

    addValueConstructSyntax
    (
        DataType::COLOR4,
        ValueConstructSyntax
        (
            "vec4(", ")",
            "vec4(", ")",
            {".r", ".g", ".b", ".a"}
    )
    );

    addValueConstructSyntax(
        DataType::VECTOR2,
        ValueConstructSyntax
        (
            "vec2(", ")",
            "vec2(", ")",
            { ".x", ".y"}
        )
    );

    addValueConstructSyntax
    (
        DataType::VECTOR3,
        ValueConstructSyntax
        (
            "vec3(", ")",
            "vec3(", ")",
            { ".x", ".y", ".z"}
        )
    );

    addValueConstructSyntax
    (
        DataType::VECTOR4,
        ValueConstructSyntax
        (
            "vec4(", ")",
            "vec4(", ")",
            { ".x", ".y", ".z", ".w"}
        )
    );

    addValueConstructSyntax
    (
        DataType::MATRIX3,
        ValueConstructSyntax
        (
            "mat3(", ")",
            "mat3(", ")",
            { }
        )
    );

    addValueConstructSyntax
    (
        DataType::MATRIX4,
        ValueConstructSyntax
        (
            "mat4(", ")",
            "mat4(", ")",
            {}
        )
    );

}

string GlslSyntax::getValue(const Value& value, const string& type, bool paramInit) const
{
    if (type == DataType::STRING)
    {
        // Since GLSL doesn't support strings we convert
        // to an integrer here
        // TODO: Support options strings by converting to a corresponding enum integer
        return "0";
    }
    return Syntax::getValue(value, type, paramInit);
}

}
