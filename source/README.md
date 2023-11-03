# MaterialX C++ Libraries

The OSL, GLSL, MSL, and MDL modules can be conditionally built.

- [MaterialXCore](MaterialXCore) -- Core MaterialX elements and graph traversal
- [MaterialXFormat](MaterialXFormat) -- Support for XML serialization and file accessor utilities
- [MaterialXGenShader](MaterialXGenShader) -- Core shader generation support [<sup>1</sup>](#1)
- [MaterialXGenGlsl](MaterialXGenGlsl) -- Shader generation using OpenGL Shading Language
- [MaterialXGenOsl](MaterialXGenOsl) -- Shader generation using Open Shading Language
- [MaterialXGenMdl](MaterialXGenMdl) -- Shader generation using Material Definition Language
- [MaterialXGenMsl](MaterialXGenMsl) -- Shader generation using Metal Shading Language
- [MaterialXRender](MaterialXRender) -- Core rendering support for MaterialX
- [MaterialXRenderGlsl](MaterialXRenderGlsl) -- Rendering support using OpenGL Shading Language
- [MaterialXRenderOsl](MaterialXRenderOsl) -- Rendering support using Open Shading Language
- [MaterialXRenderMsl](MaterialXRenderMsl) -- Rendering support using Metal Shading Language
- [MaterialXTest](MaterialXTest) -- Unit tests for all MaterialX libraries
- [MaterialXView](MaterialXView) -- Default material viewer
- [PyMaterialX](PyMaterialX) -- Python bindings for C++ modules
- [JsMaterialX](JsMaterialX) -- JavaScript bindings for C++ modules

\[1\] <a class="anchor" id="1"></a> For more details, see the [Shader Generation Guide](../documents/DeveloperGuide/ShaderGeneration.md) and the [MaterialX Data Libraries](../libraries/README.md) containing node definitions and implementations.
