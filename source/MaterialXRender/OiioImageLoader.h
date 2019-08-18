//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_OIIOIMAGELOADER_H
#define MATERIALX_OIIOIMAGELOADER_H

/// @file
/// Image loader wrapper using OpenImageIO 

#include <MaterialXRender/ImageHandler.h>

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
        _extensions.insert(BMP_EXTENSION);
        _extensions.insert(GIF_EXTENSION);
        _extensions.insert(HDR_EXTENSION);
        _extensions.insert(JPG_EXTENSION);
        _extensions.insert(JPEG_EXTENSION);
        _extensions.insert(PIC_EXTENSION);
        _extensions.insert(PNG_EXTENSION);
        _extensions.insert(PSD_EXTENSION);
        _extensions.insert(TGA_EXTENSION);
        _extensions.insert(EXR_EXTENSION);
        _extensions.insert(TIF_EXTENSION);
        _extensions.insert(TIFF_EXTENSION);
        _extensions.insert(TX_EXTENSION);
        _extensions.insert(TXT_EXTENSION);
        _extensions.insert(TXR_EXTENSION);
    }

    /// Default destructor
    virtual ~OiioImageLoader() {}    

    /// Save image to disk. This method must be implemented by derived classes.
    /// @param filePath Path to file to save image to
    /// @param imageDesc Description of image
    /// @param verticalFlip Whether the image should be flipped in Y during save
    /// @return if save succeeded
    bool saveImage(const FilePath& filePath,
                   const ImageDesc &imageDesc,
                   bool verticalFlip = false) override;

    /// Load an image from disk. This method must be implemented by derived classes.
    /// @param filePath Path to file to load image from
    /// @param imageDesc Description of image updated during load.
    /// @param restrictions Hardware image description restrictions. Default value is nullptr, meaning no restrictions.
    /// @return if load succeeded
    bool loadImage(const FilePath& filePath, ImageDesc &imageDesc,
                   const ImageDescRestrictions* restrictions = nullptr) override;
};

} // namespace MaterialX

#endif
