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

/// @class TextureBaker
/// A helper class for baking procedural material content to textures.
class TextureBaker : public GlslRenderer
{
  public:
    static TextureBakerPtr create(unsigned int width = 1024, unsigned int height = 1024, Image::BaseType baseType = Image::BaseType::UINT8)
    {
        return TextureBakerPtr(new TextureBaker(width, height, baseType));
    }

    /// Set the file extension for baked textures.
    void setExtension(const string& extension)
    {
        _extension = extension;
    }

    /// Return the file extension for baked textures.
    const string& getExtension()
    {
        return _extension;
    }

    /// Bake textures for all graph inputs of the given shader reference.
    void bakeShaderInputs(ShaderRefPtr shaderRef, GenContext& context, const FilePath& outputFolder);

    /// Bake textures for all graph inputs of the given shader node.
    void bakeShaderInputs(NodePtr shader, GenContext& context, const FilePath& outputFolder);

    /// Bake a texture for the given graph output.
    void bakeGraphOutput(OutputPtr output, GenContext& context, const FilePath& outputFolder);

    /// Write out the baked material document based on a shader reference
    void writeBakedDocument(ShaderRefPtr shaderRef, const FilePath& filename);

    /// Write out the baked material document based on a shader node
    void writeBakedDocument(NodePtr shader, const FilePath& filename);

  protected:
    TextureBaker(unsigned int width, unsigned int height, Image::BaseType baseType);

    // Generate a texture filename for the given graph output.
    FilePath generateTextureFilename(OutputPtr output);

  protected:
    ShaderGeneratorPtr _generator;
    string _udim;
    string _extension;
};

} // namespace MaterialX

#endif
