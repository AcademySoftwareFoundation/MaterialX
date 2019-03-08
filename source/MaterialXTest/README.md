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

- GenShader.cpp : Core shader generation framework tests.
- GenGlsl.cpp : GLSL generation tests.
- GenOsl.cpp : OSL generation tests.
- GenShaderUtil.cpp : Utilities for generation tests.

Each language test will scan MaterialX files in the test suite to test code generation.
Refer to the [test suite documentation](../../documents/TestSuite) for more information about the organization of the test suite data used for testing.
