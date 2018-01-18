#include <MaterialXShaderGen/ShaderGenerators/OslSyntax.h>

namespace MaterialX
{

OslSyntax::OslSyntax()
{
    //
    // Add syntax information for each data type.
    //
    // TODO: Make this setup data driven (e.g read from a config file),
    //       to support new types without requiring a rebuild.
    //

    addTypeSyntax
    (
        kFloat, 
        TypeSyntax
        (
            "float",        // type name
            "0.0",          // default value
            "0.0",          // default value in a shader param initialization context
            "",             // custom type definition to add in source code
            "output float"  // type name in output context
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
            "output int" 
        )
    );

    addTypeSyntax
    (
        kBoolean, 
        TypeSyntax
        (
            "int", 
            "0", 
            "0",
            "#define true 1\n#define false 0",
            "output int"
        )
    );

    addTypeSyntax
    (
        kColor2, 
        TypeSyntax
        (
            "color2", 
            "color2(0.0, 0.0)", 
            "color2(0.0, 0.0)",
            "",
            "output color2"
        )
    );

    addTypeSyntax
    (
        kColor3, 
        TypeSyntax
        (
            "color", 
            "color(0.0, 0.0, 0.0)", 
            "color(0.0, 0.0, 0.0)",
            "",
            "output color"
        )
    );

    addTypeSyntax
    (
        kColor4,
        TypeSyntax
        (
            "color4",
            "color4(color(0,0,0), 0.0)",
            "color4(color(0,0,0), 0.0)",
            "",
            "output color4"
        )
    );

    addTypeSyntax
    (
        kVector2, 
        TypeSyntax
        (
            "vector2", 
            "vector2(0.0, 0.0)", 
            "vector2(0.0, 0.0)",
            "",
            "output vector2"
        )
    );

    addTypeSyntax
    (
        kVector3, 
        TypeSyntax
        (
            "vector", 
            "vector(0.0, 0.0, 0.0)", 
            "vector(0.0, 0.0, 0.0)",
            "",
            "output vector"
        )
    );

    addTypeSyntax
    (
        kVector4, 
        TypeSyntax
        (
            "vector4", 
            "vector4(0.0, 0.0, 0.0, 0.0)",
            "vector4(0.0, 0.0, 0.0, 0.0)",
            "",
            "output vector4"
        )
    );

    addTypeSyntax
    (
        kString, 
        TypeSyntax
        (
            "string", 
            "\"\"", 
            "\"\"",
            "",
            "output string"
        )
    );

    addTypeSyntax
    (
        kFilename, 
        TypeSyntax
        (
            "string", 
            "\"\"", 
            "\"\"",
            "",
            "output string"
        )
    );

    addTypeSyntax
    (
        kBSDF, 
        TypeSyntax
        (
            "BSDF", 
            "null_closure", 
            "0",
            "#define BSDF closure color",
            "output BSDF"
        )
    );

    addTypeSyntax
    (
        kEDF, 
        TypeSyntax
        (
            "EDF", 
            "null_closure", 
            "0",
            "#define EDF closure color",
            "output EDF"
        )
    );

    addTypeSyntax
    (
        kVDF, 
        TypeSyntax
        (
            "VDF", 
            "null_closure", 
            "0",
            "#define VDF closure color",
            "output VDF"
        )
    );

    addTypeSyntax
    (
        kSURFACE, 
        TypeSyntax
        (
            "surfaceshader", 
            "null_closure", 
            "0",
            "#define surfaceshader closure color",
            "output surfaceshader" 
        )
    );

    addTypeSyntax
    (
        kVOLUME, 
        TypeSyntax
        (
            "volumeshader", 
            "{0,0,0}", 
            "0",
            "struct volumeshader { VDF vdf; EDF edf; color absorption; };",
            "output volumeshader" 
        )
    );

    addTypeSyntax
    (
        kDISPLACEMENT, 
        TypeSyntax
        (
            "displacementshader", 
            "{0,0}", 
            "0",
            "struct displacementshader { vector offset; float scale; };",
            "output displacementshader"
        )
    );


    //
    // Add value constructor syntax for data types that needs this
    //

    addValueConstructSyntax(
        kColor2, 
        ValueConstructSyntax(
            "color2(", ")", // Value constructor syntax
            "color2(", ")", // Value constructor syntax in a shader param initialization context
            {".r", ".a"}    // Syntax for each vector component
        )
    );

    addValueConstructSyntax
    (
        kColor3, 
        ValueConstructSyntax
        (
            "color(", ")",
            "color(", ")",
            {"[0]", "[1]", "[2]"}
    )
    );

    addValueConstructSyntax
    (
        kColor4,
        ValueConstructSyntax
        (
            "pack(", ")",
            "pack(", ")",
            {".rgb[0]", ".rgb[1]", ".rgb[2]", ".a"}
        )
    );

    addValueConstructSyntax(
        kVector2, 
        ValueConstructSyntax
        (
            "vector2(", ")",
            "vector2(", ")",
            {".x", ".y"}
        )
    );

    addValueConstructSyntax
    (
        kVector3, 
        ValueConstructSyntax
        (
            "vector(", ")",
            "vector(", ")",
            {"[0]", "[1]", "[2]"}
        )
    );

    addValueConstructSyntax
    (
        kVector4, 
        ValueConstructSyntax
        (
            "pack(", ")",
            "pack(", ")",
            {".x", ".y", ".z", ".w"}
        )
    );

    addValueConstructSyntax
    (
        kString, 
        ValueConstructSyntax
        (
            "\"", "\"",
            "\"", "\"",
            {}
        )
    );

    addValueConstructSyntax
    (
        kFilename, 
        ValueConstructSyntax
        (
            "\"", "\"",
            "\"", "\"",
            {}
        )
    );
}

} // namespace MaterialX
