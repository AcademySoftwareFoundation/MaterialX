#ifndef MATERIALX_TINYEXRIMAGELOADER_H
#define MATERIALX_TINYEXRIMAGELOADER_H

#include <MaterialXRender/Handlers/ImageHandler.h>

namespace MaterialX
{
/// Shared pointer to an TinyEXRImageLoader
using TinyEXRImageLoaderPtr = std::shared_ptr<class TinyEXRImageLoader>;

/// @class @TinyEXRImageLoader
/// Disk image loader wrapper using TinyEXR
///
class TinyEXRImageLoader : public ImageLoader
{
public:
    /// Static instance create function
    static TinyEXRImageLoaderPtr create() { return std::make_shared<TinyEXRImageLoader>(); }

    /// Exr extension string
    static std::string EXR_EXTENSION;

    /// Default constructor
    TinyEXRImageLoader() 
    {
        // Add EXR to list of supported extension
        _extensions.push_back(EXR_EXTENSION);
    }

    /// Default destructor
    virtual ~TinyEXRImageLoader() {}    

    /// Save image to disk. This method must be implemented by derived classes.
    /// @param fileName Name of file to save image to
    /// @param imageDesc Description of image
    /// @return if save succeeded
    bool saveImage(const std::string& fileName,
                   const ImageDesc &imageDesc) override;

    /// Load an image from disk. This method must be implemented by derived classes.
    /// @param fileName Name of file to load image from
    /// @param imageDesc Description of image updated during load.
    /// @param generateMipMaps Generate mip maps if supported.
    /// @return if load succeeded
    bool acquireImage(const std::string& fileName, ImageDesc &imageDesc, bool generateMipMaps) override;
};

} // namespace MaterialX;

#endif
