# Unit tests

## Core Tests

The names of the files reflect the Element type being tested for the following:

- Document.cpp
- Element.cpp
- Geom.cpp
- Look.cpp
- Material.cpp
- Node.cpp
- Types.cpp
- Value.cpp

## Document utilities
- Observer.cpp : Document observation.
- Traversal.cpp : Document traversal.
- Util.cpp : Basic utilities.

## I/O Tests

- File.cpp : Basic file path tests.
- XmlIo.cpp : XML document I/O tests.

## Shader Generation Tests

- ShaderGen.cpp

Core shader generation framework tests.

- ShaderValid.cpp

Tests for all GLSL and OSL implementations. If rendering tests are enabled via the build options then each Element tested will be rendered if the appropriate backend support is available.
- GLSL:
    - Will execute on a Windows machine which supports OpenGL 4.0 or above.
- OSL: Uses the utilities "oslc" and "testrender" utilities from the
    [OSL distribution](https://github.com/imageworks/OpenShadingLanguage).
    - The test suite has been tested with version 1.9.10.
    - The following build options are required to be set:
        - MATERIALX_OSLC_EXECUTABLE: Full path to the `oslc` binary.
        - MATERIALX_TESTRENDER_EXECUTABLE: Full path to the `testrender` binary.
        - MATERIALX_OSL_INCLUDE_PATH: Full path to OSL include paths (i.e. location of `stdosl.h`).

Refer to the [test suite documentation](../../documents/TestSuite) for more information about the organization of the test suite data used for this test.
