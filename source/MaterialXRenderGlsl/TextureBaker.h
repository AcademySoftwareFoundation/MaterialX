//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TEXTUREBAKER
#define MATERIALX_TEXTUREBAKER

/// @file
/// Texture baking functionality

#include <MaterialXRender/ImageHandler.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

class TextureBaker;
class GlslValidator;
using GlslValidatorPtr = std::shared_ptr<GlslValidator>;

/// A shared pointer to a TextureBaker
using TextureBakerPtr = shared_ptr<TextureBaker>;

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
    GlslValidatorPtr _rasterizer;
    ShaderGeneratorPtr _generator;
    FileSearchPath _searchPath;
    string _fileSuffix;
};

} // namespace MaterialX

#endif // MATERIALX_TEXTUREBAKER
