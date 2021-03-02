//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TEXTUREBAKER
#define MATERIALX_TEXTUREBAKER

/// @file
/// Texture baking functionality

#include <iostream>

#include <MaterialXCore/Unit.h>

#include <MaterialXRenderGlsl/GlslRenderer.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// A shared pointer to a TextureBaker
using TextureBakerPtr = shared_ptr<class TextureBaker>;

/// A vector of baked documents with their associated names.
using BakedDocumentVec = std::vector<std::pair<std::string, DocumentPtr>>;

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

    /// Set the distance unit to which textures are baked.  Defaults to meters.
    void setDistanceUnit(const string& unitSpace)
    {
        _distanceUnit = unitSpace;
    }

    /// Return the distance unit to which textures are baked.
    const string& getDistanceUnit() const
    {
        return _distanceUnit;
    }

    /// Set whether images should be averaged to generate constants.  Defaults to false.
    void setAverageImages(bool enable)
    {
        _averageImages = enable;
    }

    /// Return whether images should be averaged to generate constants.
    bool getAverageImages() const
    {
        return _averageImages;
    }

    /// Set whether uniform textures should be stored as constants.  Defaults to true.
    void setOptimizeConstants(bool enable)
    {
        _optimizeConstants = enable;
    }

    /// Return whether uniform textures should be stored as constants.
    bool getOptimizeConstants() const
    {
        return _optimizeConstants;
    }

    /// Set the output location for baked texture images.  Defaults to the root folder
    /// of the destination material.
    void setOutputImagePath(const FilePath& outputImagePath)
    {
        _outputImagePath = outputImagePath;
    }

    /// Get the current output location for baked texture images.
    const FilePath& getOutputImagePath()
    {
        return _outputImagePath;
    }

    /// Set the name of the baked graph element.
    void setBakedGraphName(const string& name)
    {
        _bakedGraphName= name;
    }

    /// Return the name of the baked graph element.
    const string& getBakedGraphName() const
    {
        return _bakedGraphName;
    }

    /// Set the name of the baked geometry info element.
    void setBakedGeomInfoName(const string& name)
    {
        _bakedGeomInfoName = name;
    }

    /// Return the name of the baked geometry info element.
    const string& getBakedGeomInfoName() const
    {
        return _bakedGeomInfoName;
    }

    /// Set the output stream for reporting progress and warnings.  Defaults to std::cout.
    void setOutputStream(std::ostream* outputStream)
    {
        _outputStream = outputStream;
    }

    /// Return the output stream for reporting progress and warnings.
    std::ostream* getOutputStream() const
    {
        return _outputStream;
    }

    /// Set baked texture resolution automatically. Defaults to false.
    /// If any images are found upstream from a shader input, then the output baked texture is the largest image resolution. 
    /// If no images are found, then the fixed resolution of the baker is used.
    void setAutoTextureResolution(bool enable)
    {
        _autoTextureResolution = enable;
    }

    /// Return whether automatic baked texture resolution is set.
    bool getAutoTextureResolution() const
    {
        return _autoTextureResolution;
    }

    /// Set whether to create a short name for baked images by hashing the baked image filenames
    /// This is useful for file systems which may have a maximum limit on filename size.
    /// By default names are not hashed.
    void setHashImageNames(bool enable)
    {
        _hashImageNames = enable;
    }

    /// Return whether automatic baked texture resolution is set.
    bool getHashImageNames() const
    {
        return _hashImageNames;
    }

    /// Set up the unit definitions to be used in baking.
    void setupUnitSystem(DocumentPtr unitDefinitions);

    /// Bake textures for all graph inputs of the given shader.
    void bakeShaderInputs(NodePtr material, NodePtr shader, GenContext& context, const string& udim = EMPTY_STRING);

    /// Bake a texture for the given graph output.
    void bakeGraphOutput(OutputPtr output, GenContext& context, const FilePath& filename);

    /// Optimize baked textures before writing.
    void optimizeBakedTextures(NodePtr shader);

    /// Write the baked material with textures to a document.
    DocumentPtr bakeMaterial(NodePtr shader, const StringVec& udimSet);

    /// Bake all materials in the given document and return them as a vector.
    BakedDocumentVec createBakeDocuments(DocumentPtr doc, const FileSearchPath& searchPath);

    /// Bake all materials in the given document and write them to disk.  If multiple documents are written,
    /// then the given output filename will be used as a template.
    void bakeAllMaterials(DocumentPtr doc, const FileSearchPath& searchPath, const FilePath& outputFileName);

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
    string _distanceUnit;
    bool _averageImages;
    bool _optimizeConstants;
    FilePath _outputImagePath;
    string _bakedGraphName;
    string _bakedGeomInfoName;
    std::ostream* _outputStream;
    bool _autoTextureResolution;
    bool _hashImageNames;

    ShaderGeneratorPtr _generator;
    ConstNodePtr _material;
    WorldSpaceInputs _worldSpaceShaderInputs;
    BakedImageMap _bakedImageMap;
    BakedConstantMap _bakedConstantMap;
};

} // namespace MaterialX

#endif
