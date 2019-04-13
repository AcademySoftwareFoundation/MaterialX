# Core Specification Support

## C++ Modules
- [MaterialXCore](MaterialXCore): Support for the core MaterialX elements and graph traversal.
- [MaterialXFormat](MaterialXFormat): Support for XML serialization and file accessor utilities.
- [MaterialXGenShader](MaterialXGenShader) : Core shader generation support [<sup>1</sup>](#1)
- [MaterialXGenGlsl](MaterialXGenGlsl) : GLSL shading language generation support.
- [MaterialXGenOsl](MaterialXGenOsl) : OSL shading language generation support.
- [MaterialXTest](MaterialXTest) : Unit test module. The shader generation test suite resides [here](../resources/Materials/TestSuite).
- [MaterialXView](MaterialXView) : Sample material viewer.

## Python Modules

Python modules which provide wrappers for the provided C++ modules reside under the [PyMaterialX](PyMaterialX/README.md) folder.

\[1\] <a class="anchor" id="1"></a> Refer to [shader generation documentation](../documents/DeveloperGuide/ShaderGeneration.md) and the associated definition and implementation [libraries](../libraries).
