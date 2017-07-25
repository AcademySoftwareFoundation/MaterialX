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
        kFloat, 
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
        kInteger, 
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
        kBoolean, 
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
        kColor2, 
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
        kColor3, 
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
        kColor4, 
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
        kVector2, 
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
        kVector3, 
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
        kVector4, 
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
        kString, 
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
        kFilename, 
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
        kBSDF, 
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
        kEDF, 
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
        kVDF, 
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
        kSURFACE,
        TypeSyntax
        (
            "surfaceshader",
            "surfaceshader(vec3(0.0),vec3(0.0),1.0,1.0)",
            "",
            "struct surfaceshader { vec3 bsdf; vec3 edf; float ior; float alpha; };",
            "out vec4"
        )
    );

    addTypeSyntax
    (
        kVOLUME, 
        TypeSyntax
        (
            "volumeshader", 
            "volumeshader(vec3(0.0),vec3(0.0),vec3(0.0))",
            "",
            "struct volumeshader { vec3 vdf; vec3 edf; vec3 absorption; };",
            "out vec4"
        )
    );

    addTypeSyntax
    (
        kDISPLACEMENT, 
        TypeSyntax
        (
            "displacementshader", 
            "displacementshader(vec3(0.0),1.0)",
            "",
            "struct displacementshader { vec3 offset; float scale; };",
            "out vec3"
        )
    );


    //
    // Add value constructor syntax for data types that needs this
    //

    addValueConstructSyntax(
        kColor2, 
        ValueConstructSyntax(
            "vec2(", ")", // Value constructor syntax
            "{", "}",     // Value constructor syntax in a shader param initialization context
            {".r", ".g"}  // Syntax for each vector component
        )
    );

    addValueConstructSyntax
    (
        kColor3, 
        ValueConstructSyntax
        (
            "vec3(", ")",
            "{", "}",
            {".r", ".g", ".b"}
    )
    );

    addValueConstructSyntax
    (
        kColor4,
        ValueConstructSyntax
        (
            "vec4(", ")",
            "{", "}",
            {".r", ".g", ".b", ".a"}
    )
    );

    addValueConstructSyntax(
        kVector2, 
        ValueConstructSyntax
        (
            "vec2(", ")",
            "{", "}",
            { ".x", ".y"}
        )
    );

    addValueConstructSyntax
    (
        kVector3, 
        ValueConstructSyntax
        (
            "vec3(", ")",
            "{", "}",
            { ".x", ".y", ".z"}
        )
    );

    addValueConstructSyntax
    (
        kVector4, 
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
