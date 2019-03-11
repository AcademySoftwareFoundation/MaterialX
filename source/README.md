# Core Specification Support

- [MaterialXCore](MaterialXCore): Support for the core MaterialX elements
and graph traversal.
- [MaterialXFormat](MaterialXFormat): XML serialization support.
- [PyMaterialX](PyMaterialX/README.md) : Core library Python API support

# Shader Generation Support

## Supported APIs
-   C++
-   Python

## Supported Backends

-   Core shader generation API: [MaterialXGenShader](MaterialXGenShader) module
    -  Refer to document in module folder.
-   Shader Generation languages supported:
    -   GLSL
        -   Version 4 or higher
        -   Core support in [MaterialXGenGLSL](MaterialXGenGLSL) module
    -   OSL:
        -   [MaterialXGenOsl](MaterialXGenOsl) module.
        -   Uses an implementation which differs from the reference OSL
            implementation

## Definition Libraries

-  Standard library ([stdlib](../documents/Libraries/stdlib)) implementations for supported languages.
-  PBR library ([pbrlib](../documents/Libraries/pbrlib)): node definitions and implementations for supported languages.
-   Basic "default" non-LUT color management transforms for OSL and
    GLSL:
    -   lin_rec709, gamma18, gamma22, gamm24, acescg, srgb_texture
-   Basic "light" definitions and implementations for GLSL:
    -   point, directional, spotlight, IBL

## Library Tree Structure
- Refer to documentation in the [Libraries](../documents/Libraries) folder.

## Unsupported definitions for Shader Generation

The following nodes and implementations are not currently supported:
-   ambientocclusion
-   arrayappend
-   curveadjust
-   displacementshader and volumeshader and associated operations (add,
    multiply, mix)
-   mix surfaceshader for GLSL

## Test Framework

The unit tests are located in the [MaterialXTest](MaterialXTest/README.md) module.

This includes tests for core and shader generation. The tests executed are based on what build options have been enabled. The test suite for this resides [here](../documents/TestSuite).

## Build Options
By default MaterialXCore, MaterialXFormat and MaterialXGenShader, MaterialXGenGlsl and MaterialXGenOsl are built.
- Python support is enabled via the MATERIALX_BUILD_PYTHON build variable.

