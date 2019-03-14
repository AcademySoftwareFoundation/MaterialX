//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_STBIMAGELOADER_H
#define MATERIALX_STBIMAGELOADER_H

/// @file
/// Image loader using the stb image library

#include <MaterialXRender/Handlers/ImageHandler.h>

namespace MaterialX
{
/// Shared pointer to an stbImageLoader
using StbImageLoaderPtr = std::shared_ptr<class StbImageLoader>;

/// @class StbImageLoader
/// Disk image loader wrapper using stb library
///
class StbImageLoader : public ImageLoader
{
  public:
    /// Static instance create function
    static StbImageLoaderPtr create() { return std::make_shared<StbImageLoader>(); }

    /// Default constructor. Set all extensions supported by stb
    StbImageLoader() 
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
    }

    /// Default destructor
    virtual ~StbImageLoader() {}    

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
