//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_OIIOIMAGELOADER_H
#define MATERIALX_OIIOIMAGELOADER_H

/// @file
/// Image loader wrapper using OpenImageIO 

#include <MaterialXRender/Handlers/ImageHandler.h>

namespace MaterialX
{
/// Shared pointer to an OiioImageLoader
using OiioImageLoaderPtr = std::shared_ptr<class OiioImageLoader>;

/// @class OiioImageLoader
/// Disk image loader wrapper using OpenImageIO library
///
class OiioImageLoader : public ImageLoader
{
  public:
    /// Static instance create function
    static OiioImageLoaderPtr create() { return std::make_shared<OiioImageLoader>(); }

    /// Default constructor. Set all extensions supported by stb
    OiioImageLoader() 
    {
        _extensions.push_back(BMP_EXTENSION);
        _extensions.push_back(GIF_EXTENSION);
        _extensions.push_back(HDR_EXTENSION);
        _extensions.push_back(JPG_EXTENSION);
        _extensions.push_back(JPEG_EXTENSION);
        _extensions.push_back(PIC_EXTENSION);
        _extensions.push_back(PNG_EXTENSION);
        _extensions.push_back(PSD_EXTENSION);
        _extensions.push_back(TGA_EXTENSION);
        _extensions.push_back(EXR_EXTENSION);
        _extensions.push_back(TIF_EXTENSION);
        _extensions.push_back(TIFF_EXTENSION);
        _extensions.push_back(TXT_EXTENSION);
        _extensions.push_back(TXR_EXTENSION);
    }

    /// Default destructor
    virtual ~OiioImageLoader() {}    

    /// Save image to disk. This method must be implemented by derived classes.
    /// @param filePath Path to file to save image to
    /// @param imageDesc Description of image
    /// @return if save succeeded
    bool saveImage(const FilePath& filePath,
                   const ImageDesc &imageDesc) override;

    /// Load an image from disk. This method must be implemented by derived classes.
    /// @param filePath Path to file to load image from
    /// @param imageDesc Description of image updated during load.
    /// @param generateMipMaps Generate mip maps if supported.
    /// @return if load succeeded
    bool acquireImage(const FilePath& filePath, ImageDesc &imageDesc, bool generateMipMaps) override;
};

} // namespace MaterialX;

#endif
