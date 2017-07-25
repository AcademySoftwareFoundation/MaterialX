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
            "color", 
            "color(0.0, 0.0, 0.0)", 
            "color(0.0, 0.0, 0.0)",
            "",
            "output color"
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
            "_color4(0.0, 0.0, 0.0, 0.0)",
            "{color(0,0,0),0}",
            "struct color4 { color rgb; float a; };\n"
            "color4 _color4(float r, float g, float b, float a)\n"
            "{\n"
            "    color4 c4;\n"
            "    c4.rgb = color(r, g, b);\n"
            "    c4.a = a;\n"
            "    return c4;\n"
            "}\n"
            "color4 _color4(color rgb, float a)\n"
            "{\n"
            "    color4 c4;\n"
            "    c4.rgb = rgb;\n"
            "    c4.a = a;\n"
            "    return c4;\n"
            "}",
            "output color4"
        )
    );

    addTypeSyntax
    (
        kVector2, 
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
            "_vector4(0.0, 0.0, 0.0, 0.0)", 
            "{vector(0,0,0),0}",
            "struct vector4 { vector xyz; float w; };\n"
            "vector4 _vector4(float x, float y, float z, float w)\n"
            "{\n"
            "    vector4 v4;\n"
            "    v4.xyz = vector(x, y, z);\n"
            "    v4.w = w;\n"
            "    return v4;\n"
            "}\n"
            "vector4 _vector4(vector xyz, float w)\n"
            "{\n"
            "    vector4 v4;\n"
            "    v4.xyz = xyz;\n"
            "    v4.w = w;\n"
            "    return v4;\n"
            "}",
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
            "string"
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
            "output closure color"
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
            "output closure color"
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
            "output closure color"
        )
    );

    addTypeSyntax
    (
        kSURFACE, 
        TypeSyntax
        (
            "surfaceshader", 
            "{0,0,0}", 
            "0",
            "struct surfaceshader { BSDF bsdf; EDF edf; float ior; };",
            "output closure color" 
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
            "struct volumeshader { VDF vdf; EDF  edf; color absorption; };",
            "output closure color" 
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
            "output vector"
        )
    );


    //
    // Add value constructor syntax for data types that needs this
    //

    addValueConstructSyntax(
        kColor2, 
        ValueConstructSyntax(
            "color(", ", 0)", // Value constructor syntax
            "color(", ", 0)", // Value constructor syntax in a shader param initialization context
            {"[0]", "[1]"}    // Syntax for each vector component
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
            "_color4(", ")",
            "_color4(", ")",
            {".rgb[0]", ".rgb[1]", ".rgb[2]", ".a"}
        )
    );

    addValueConstructSyntax(
        kVector2, 
        ValueConstructSyntax
        (
            "vector(", ", 0)",
            "vector(", ", 0)",
            {"[0]", "[1]"}
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
            "_vector4(", ")",
            "_vector4(", ")",
            {".xyz[0]", ".xyz[1]", ".xyz[2]", ".w"}
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
