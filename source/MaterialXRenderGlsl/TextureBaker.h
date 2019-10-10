//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TEXTUREBAKER
#define MATERIALX_TEXTUREBAKER

/// @file
/// Texture baking functionality

#include <MaterialXRenderGlsl/GlslRenderer.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// A shared pointer to a TextureBaker
using TextureBakerPtr = shared_ptr<class TextureBaker>;

class TextureBaker
{
  public:
    TextureBaker(unsigned int res = 1024);
    ~TextureBaker() { }

    static TextureBakerPtr create()
    {
        return std::make_shared<TextureBaker>();
    }

    /// Bake textures for all graph inputs of the given shader reference.
    void bakeShaderInputs(ShaderRefPtr shaderRef, const FileSearchPath& searchPath, GenContext& context, const FilePath& outputFolder);

    /// Bake a texture for the given graph output.
    void bakeGraphOutput(OutputPtr output, GenContext& context, const FilePath& outputFolder);

    /// Write out a document with baked images.
    void writeBakedDocument(ShaderRefPtr shaderRef, const FilePath& filename);

  protected:
    void setSearchPath(const FileSearchPath searchPath) { _searchPath = searchPath; }
    FileSearchPath getSearchPath() { return _searchPath; }

  protected:
    GlslRendererPtr _renderer;
    ShaderGeneratorPtr _generator;
    FileSearchPath _searchPath;
    string _fileSuffix;
};

} // namespace MaterialX

#endif
