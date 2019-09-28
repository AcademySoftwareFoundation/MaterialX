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

#include <map>

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
    TextureBaker() :
        _frameBufferRes(1024),
        _fileSuffix(ImageLoader::PNG_EXTENSION)
    {
    }
    ~TextureBaker() { }

    static TextureBakerPtr create()
    {
        return std::make_shared<TextureBaker>();
    }

    /// Saves freshly made images for various outputs to disk
    void bakeAllInputTextures(const FileSearchPath& searchPath, ElementPtr elem, GenContext context, const string& udim, const FilePath& outputFolder);

    /// Saves freshly made image for specific output to disk
    void bakeTextureFromElementInput(ElementPtr elem, GenContext& context, const FilePath& outputFolder);

    /// Write out a document with baked images.
    void writeDocument(DocumentPtr& origDoc, TypedElementPtr elem, const FilePath& filename);

  protected:
    void init(GenOptions& options, ElementPtr input, const string udim);

    void setSearchPath(const FileSearchPath searchPath) { _searchPath = searchPath; }
    FileSearchPath getSearchPath() { return _searchPath; }

    void recordBakedTexture(const string input, const string outputFile) { _bakedTextures[input] = outputFile; }
    void recordNodegraphInput(const string input, const string type) { _bakedOutputs[input] = type; }

  protected:
    GlslValidatorPtr _rasterizer;
    ShaderGeneratorPtr _generator;

    unsigned int _frameBufferRes;
    string _fileSuffix;

    FileSearchPath _searchPath;

    std::map<string, string> _bakedTextures;
    std::map<string, string> _bakedOutputs;
};

} // namespace MaterialX

#endif // MATERIALX_TEXTUREBAKER
