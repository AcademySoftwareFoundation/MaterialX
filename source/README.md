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

The following nodes and implementations are not currently supported:
-   ambientocclusion
-   arrayappend
-   curveadjust
-   displacementshader and volumeshader and associated operations (add,
    multiply, mix)
-   mix surfaceshader for GLSL

## Rendering Utilities

The [MaterialXRender](MaterialXRender) module contains the following base rendering
and validation utilities:
  - Geometry handler framework with sample OBJ format support via "TinyObjLoader" (https://github.com/syoyo/tinyobjloader).
  - Image handler framework with sample support via the "stb" loader (https://github.com/nothings/stb). A wrapper to use OpenImageIO (https://github.com/OpenImageIO/oiio) is also include if the build flag is enabled.
-  [MaterialXRenderHw](MaterialXRenderHw) : Provides base hardware rendering support.
-  [MaterialXRenderGlsl](MaterialXRenderGlsl) : Provides GLSL support.
-  [MaterialXRenderOsl](MaterialXRenderOsl) : Provides OSL support.

## Test Framework

The unit tests are located in the [MaterialXTest](MaterialXTest/README.md) module.

This includes tests for core and shader generation. The tests executed are based on what build options have been enabled. The test suite for this resides [here](../documents/TestSuite).

## Viewing Utilities

- [MaterialXView](https://github.com/jstone-dev/MaterialX/blob/adsk_contrib/dev/README.md) module
- Sample material viewer which uses the core, shader generation and rendering utilities libraries.

## Build Options
By default MaterialXCore, MaterialXFormat and MaterialXGenShader are built.
- Python support is enabled via the MATERIALX_BUILD_PYTHON build variable.
- OGSFX shader generation is enabled via the MATERIALX_BUILD_GEN_OGSFX build variable.
- Arnold shader generation is enabled via theMATERIALX_BUILD_GEN_ARNOLD build variable.
- Building of rendering utilities is enabled via the MATERIALX_BUILD_RENDER build variable. Execution of rendering tests is enabled via the MATERIALX_TEST_RENDER. These tests are currently only supported for the Windows platform.
