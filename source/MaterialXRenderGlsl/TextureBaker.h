//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TEXTUREBAKER
#define MATERIALX_TEXTUREBAKER

/// @file
/// TextureBaker

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXCore/Document.h>

#include <map>

namespace MaterialX
{

class TextureBaker;
class GlslValidator;
using GlslValidatorPtr = std::shared_ptr<GlslValidator>;

/// A shared pointer to a TextureBaker
using TextureBakerPtr = shared_ptr<TextureBaker>;
/// A shared pointer to a const TextureBaker
using ConstTextureBakerPtr = shared_ptr<const TextureBaker>;

class TextureBaker
{
  public:
    TextureBaker()
    {
    }
    ~TextureBaker() { }

    static TextureBakerPtr create()
    {
        return std::make_shared<TextureBaker>();
    }

    /// Saves freshly made images for various outputs to disk
    void bakeAllInputTextures(unsigned int frameBufferDim, const string& fileSuffix, const FileSearchPath& searchPath,
                              ElementPtr elem, GenContext& context, const string& udim, const FilePath& outputFolder);

    /// Saves freshly made image for specific output to disk
    void bakeTextureFromElementInput(ElementPtr elem, GenContext& context, const FilePath& outputFolder);

    /// Write out a document with baked images.
    void writeDocument(DocumentPtr& origDoc, TypedElementPtr elem, const FilePath& filename);

  protected:
    void setFileSuffix(const string fileSuffix) { _fileSuffix = fileSuffix; }
    const string getFileSuffix() { return _fileSuffix; }
    void setFrameBufferDim(int frameBufferDim) { _frameBufferDim = frameBufferDim; }
    int getFrameBufferDim() { return _frameBufferDim; }
    void setSearchPath(const FileSearchPath searchPath) { _searchPath = searchPath; }
    FileSearchPath getSearchPath() { return _searchPath; }

    bool alreadyBaked(const string output) { return _bakedOutputs.count(output) == 0; }
    void recordBakedTexture(const string input, const string outputFile) { _bakedTextures[input] = outputFile; }
    void recordNodegraphInput(const string input, const string type) { _bakedOutputs[input] = type; }

    /// Internal context initialization for texture baking
    void prepareTextureSpace(GenOptions& options, ElementPtr input, const string udim);
    /// Internal context cleanup for texture baking
    void cleanup(GenOptions& options);

    /// Our rasterizer that will do the rendering
    GlslValidatorPtr _rasterizer;
    /// Our shader generator
    ShaderGeneratorPtr _generator;

    /// Default file format for baked texture
    string _fileSuffix = ".png";

    /// dimensions for the texture
    unsigned int _frameBufferDim = 512;

    /// Path to look for textures
    FileSearchPath _searchPath;

    /// Map to keep track of textures baked so far
    std::map<string, string> _bakedTextures;
    /// Map to keep track of shader graph outputs baked so far
    std::map<string, string> _bakedOutputs;
};

} // namespace MaterialX

#endif // MATERIALX_TEXTUREBAKER
