# Core Specification Support

- [MaterialXCore](MaterialXCore): Support for the core MaterialX elements
and graph traversal.
- [MaterialXFormat](MaterialXFormat): XML serialization support.
- [PyMaterialX](PyMaterialX) : Core library Python API support

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
        -   Autodesk OGSFX Effect support in [MaterialXGenOgsFx](MaterialXGenOgsFx) module
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

Nodes and implementations which are not currently supported:
-   ambientocclusion
-   arrayappend
-   curveadjust
-   displacementshader and volumeshader and associated operations (add,
    multiply, mix)
-   mix surfaceshader for GLSL
-   Matrix33 type operations for OSL.

## Rendering Utilities

- [MaterialXRender](MaterialXRender) module.
- Geometry handler with OBJ format support.
- Image handler with formats supported by the "stb" open source loader.
- Render test suite: Windows only.
- GLSL and OSL program validators

## Test Framework

The unit tests are located in the [MaterialXTest](MaterialXTest/README.md) module.

This includes tests for core and shader generation. The tests executed are based on what build options have been enabled. The test suite for this resides [here](../documents/TestSuite).

## Viewing Utilities

- [MaterialXView](https://github.com/jstone-dev/MaterialX/blob/adsk_contrib/dev/README.md) module
- Sample material viewer which uses the core, shader generation and rendering utilities libraries.

## Build Options
By default MaterialXCore, MaterialXFormat and MaterialXGenShader are built.
- Python support is enabled via the MATERIALX_BUILD_PYTHON build variable.
- OSL shader generation is enabled via the MATERIALX_BUILD_GEN_OSL build variable.
- GLSL shader generation is enabled via the MATERIALX_BUILD_GEN_GLSL build variable.
- OGSFX shader generation is enabled via the MATERIALX_BUILD_GEN_OGSFX build variable. The GLSL shader generation build variable must also be enabled.
- Building of rendering utilities is enabled via the MATERIALX_BUILD_RENDER build variable. Execution of rendering tests is enabled via the MATERIALX_TEST_RENDER. These tests are currently only supported for the Windows platform.
