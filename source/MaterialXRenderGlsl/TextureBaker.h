//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TEXTUREBAKER
#define MATERIALX_TEXTUREBAKER

/// @file
/// Texture baking functionality

#include <MaterialXCore/Unit.h>

#include <MaterialXRenderGlsl/GlslRenderer.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// A shared pointer to a TextureBaker
using TextureBakerPtr = shared_ptr<class TextureBaker>;

/// @class TextureBaker
/// A helper class for baking procedural material content to textures.
/// TODO: Add support for graphs containing geometric nodes such as position
///       and normal.
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
    const string& getExtension() const
    {
        return _extension;
    }

    /// Set the color space in which color textures are encoded.
    ///
    /// By default, this color space is srgb_texture, and color inputs are
    /// automatically transformed to this space by the baker.  If another color
    /// space is set, then the input graph is responsible for transforming
    /// colors to this space.
    void setColorSpace(const string& colorSpace)
    {
        _colorSpace = colorSpace;
    }

    /// Return the color space in which color textures are encoded.
    const string& getColorSpace() const
    {
        return _colorSpace;
    }

    /// Set the real unit conversion system used during baking
    void setupUnitSystem(DocumentPtr unitDefinitions);

    /// Set the target unit space. The default unit space is "meter" for distance.
    void setTargetUnitSpace(const string& unitSpace)
    {
        _targetUnitSpace = unitSpace;
    }

    /// Return the target unit space 
    const string& getTargetUnitSpace() const
    {
        return _targetUnitSpace;
    }

    /// Set whether images should be averaged to generate constants.   void setAverageImages(bool enable)
    void setAverageImages(bool enable)
    {
        _averageImages = enable;
    }

    /// Return whether images should be averaged to generate constants.
    bool getAverageImages()
    {
        return _averageImages;
    }

    /// Set whether uniform textures should be stored as constants.  Defaults to true.
    void setOptimizeConstants(bool enable)
    {
        _optimizeConstants = enable;
    }

    /// Return whether uniform textures should be stored as constants.
    bool getOptimizeConstants()
    {
        return _optimizeConstants;
    }

    /// Bake textures for all graph inputs of the given shader.
    void bakeShaderInputs(NodePtr material, NodePtr shader, GenContext& context, const FilePath& outputFolder, const string& udim = EMPTY_STRING);

    /// Bake a texture for the given graph output.
    void bakeGraphOutput(OutputPtr output, GenContext& context, const FilePath& filename);

    /// Optimize baked textures before writing.
    void optimizeBakedTextures();

    /// Write out the baked material and textures.
    void writeBakedMaterial(const FilePath& filename, const StringVec& udimSet);

    /// Generate a baked version of each material in the input document.
    void bakeAllMaterials(DocumentPtr doc, const FileSearchPath& imageSearchPath, const FilePath& outputFilename);

  protected:
    TextureBaker(unsigned int width, unsigned int height, Image::BaseType baseType);

    // Generate a texture filename for the given graph output.
    FilePath generateTextureFilename(OutputPtr output, const string& srName, const string& udim);

  protected:
    class BakedImage
    {
      public:
        ImagePtr image;
        bool isUniform = false;
        Color4 uniformColor;
        FilePath filename;
    };
    class BakedConstant
    {
      public:
        Color4 color;
        bool isDefault = false;
    };
    using BakedImageVec = vector<BakedImage>;
    using BakedImageMap = std::unordered_map<OutputPtr, BakedImageVec>;
    using BakedConstantMap = std::unordered_map<OutputPtr, BakedConstant>;

    using WorldSpaceInputs = std::unordered_map<string, NodePtr>;

  protected:
    string _extension;
    string _colorSpace;
    string _targetUnitSpace;
    bool _averageImages;
    bool _optimizeConstants;

    ShaderGeneratorPtr _generator;
    ConstNodePtr _shader;
    ConstNodePtr _material;
    WorldSpaceInputs _worldSpaceShaderInputs;
    BakedImageMap _bakedImageMap;
    BakedConstantMap _bakedConstantMap;
};

} // namespace MaterialX

#endif
