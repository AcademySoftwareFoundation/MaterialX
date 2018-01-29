#include <MaterialXShaderGen/ShaderGenerators/GlslSyntax.h>

namespace MaterialX
{

GlslSyntax::GlslSyntax()
{
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
            "{0.0, 0.0}",
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
            "{0.0, 0.0, 0.0}",
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
            "{0.0, 0.0, 0.0, 0.0}",
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
            "{0.0, 0.0}",
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
            "{0.0, 0.0, 0.0}",
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
            "{0.0, 0.0, 0.0, 0.0}",
            "",
            "out vec4"
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


    //
    // Add value constructor syntax for data types that needs this
    //

    addValueConstructSyntax(
        DataType::COLOR2,
        ValueConstructSyntax(
            "vec2(", ")", // Value constructor syntax
            "{", "}",     // Value constructor syntax in a shader param initialization context
            {".r", ".g"}  // Syntax for each vector component
        )
    );

    addValueConstructSyntax
    (
        DataType::COLOR3,
        ValueConstructSyntax
        (
            "vec3(", ")",
            "{", "}",
            {".r", ".g", ".b"}
    )
    );

    addValueConstructSyntax
    (
        DataType::COLOR4,
        ValueConstructSyntax
        (
            "vec4(", ")",
            "{", "}",
            {".r", ".g", ".b", ".a"}
    )
    );

    addValueConstructSyntax(
        DataType::VECTOR2,
        ValueConstructSyntax
        (
            "vec2(", ")",
            "{", "}",
            { ".x", ".y"}
        )
    );

    addValueConstructSyntax
    (
        DataType::VECTOR3,
        ValueConstructSyntax
        (
            "vec3(", ")",
            "{", "}",
            { ".x", ".y", ".z"}
        )
    );

    addValueConstructSyntax
    (
        DataType::VECTOR4,
        ValueConstructSyntax
        (
            "vec4(", ")",
            "{", "}",
            { ".x", ".y", ".z", ".w"}
        )
    );
}

string GlslSyntax::getValue(const Value& value, bool paramInit) const
{
    if (value.isA<string>())
    {
        // Since GLSL doesn't support strings we convert
        // to an integrer here
        // TODO: Support options strings by converting to a corresponding enum integer
        return "0";
    }
    return Syntax::getValue(value, paramInit);
}

}
