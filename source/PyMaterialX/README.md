# Python Support

A Python package is created from the following modules. Shader-generator modules are built when their CMake options are enabled (for example `MATERIALX_BUILD_GEN_WGSL` for WGSL).

- [PyMaterialXCore](PyMaterialXCore): Python module for MaterialX core
- [PyMaterialXFormat](PyMaterialXFormat): Python module for XML serialization support
- [PyMaterialXGenShader](PyMaterialXGenShader) : Python module for core shader generation
- [PyMaterialXGenOsl](PyMaterialXGenOsl) : Python module for OSL shader generation
- [PyMaterialXGenGlsl](PyMaterialXGenGlsl) : Python module for GLSL shader generation
- [PyMaterialXGenMdl](PyMaterialXGenMdl) : Python module for MDL shader generation
- [PyMaterialXGenMsl](PyMaterialXGenMsl) : Python module for MSL shader generation
- [PyMaterialXGenSlang](PyMaterialXGenSlang) : Python module for Slang shader generation
- [PyMaterialXGenWgsl](PyMaterialXGenWgsl) : Python module for WGSL shader generation (see [WGSL Shader Generation](../../documents/DeveloperGuide/WGSLShaderGeneration.md))
