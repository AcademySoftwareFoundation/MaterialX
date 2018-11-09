#ifndef MATERIALX_IMAGEHANDLER_H
#define MATERIALX_IMAGEHANDLER_H

#include <string>
#include <memory>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <map>

namespace MaterialX
{
/// @class @ImageDesc
/// Interface to describe an image. Images are assumed to be float type.
class ImageDesc
{
  public:
    /// Image width
    unsigned width = 0; 
    /// Image height
    unsigned height = 0;
    /// Number of channels
    unsigned int channelCount = 0;
    /// Number of mip map levels
    unsigned int mipCount = 0;
    /// CPU buffer. May be empty
    float* resourceBuffer = nullptr;
    /// Hardware target dependent resource identifier. May be empty
    unsigned int resourceId = 0;

    /// Compute the number of mip map levels based on size of the image
    void computeMipCount()
    {
        mipCount = (unsigned int)std::log2(std::max(width, height)) + 1;
    }
};

/// class @ImageSamplingProperties
/// Interface to describe sampling properties for images.
class ImageSamplingProperties
{
  public:
    /// Address mode in U
    int uaddressMode;
    /// Address mode in V
    int vaddressMode;
    /// Filter type
    int filterType;
    /// Default color
    float defaultColor[4];
};

/// Image description cache
using ImageDescCache = std::unordered_map<std::string, ImageDesc>;

/// Shared pointer to an ImageLoader
using ImageLoaderPtr = std::shared_ptr<class ImageLoader>;

/// @class @ImageLoader
/// Abstract class representing an disk image loader
///
class ImageLoader
{
  public:
    /// Default constructor
    ImageLoader() {}

    /// Default destructor
    virtual ~ImageLoader() {}

    /// Returns a list of supported extensions
    /// @return List of support extensions
    const std::vector<std::string>& supportedExtensions()
    {
        return _extensions;
    }

    /// Save image to disk. This method must be implemented by derived classes.
    /// @param fileName Name of file to save image to
    /// @param imageDesc Description of image
    /// @return if save succeeded
    virtual bool saveImage(const std::string& fileName,
                           const ImageDesc &imageDesc) = 0;

    /// Acquire an image from disk. This method must be implemented by derived classes.
    /// @param fileName Name of file to load image from
    /// @param imageDesc Description of image updated during load.
    /// @param generateMipMaps Generate mip maps if supported.
    /// @return if load succeeded
    virtual bool acquireImage(const std::string& fileName, ImageDesc &imageDesc, bool generateMipMaps) = 0;

  protected:
    /// List of supported string extensions
    std::vector<std::string> _extensions;
};

/// Shared pointer to an ImageHandler
using ImageHandlerPtr = std::shared_ptr<class ImageHandler>;

/// Map of extensions to image loaders
using ImageLoaderMap = std::multimap<std::string, ImageLoaderPtr>;

/// @class @ImageHandler
/// A image handler class. Keeps track of images which are loaded
/// from disk via supplied ImageLoader. Derive classes are responsible
/// for determinine how to perform the logic for "binding" of these resources
/// for a given target (such as a given shading language).
///
class ImageHandler
{
  public:
    /// Constructor. Assume at least one loader must be supplied.
    ImageHandler(ImageLoaderPtr imageLoader);

    /// Add additional image loaders. Useful to handle different file
    /// extensions
    /// @param loader Loader to add to list of available loaders.
    void addLoader(ImageLoaderPtr loader);
    
    /// Default destructor
    virtual ~ImageHandler() {}

    /// Save image to disk. This method must be implemented by derived classes.
    /// The first image loader which supports the file name extension will be used.
    /// @param fileName Name of file to save image to
    /// @param imageDesc Description of image
    /// @return if save succeeded
    virtual bool saveImage(const std::string& fileName,
                           const ImageDesc &imageDesc);

    /// Acquire an image from disk. This method must be implemented by derived classes.
    /// The first image loader which supports the file name extension will be used.
    /// @param fileName Name of file to load image from.
    /// @param imageDesc Description of image updated during load.
    /// @param generateMipMaps Generate mip maps if supported.
    /// @return if load succeeded
    virtual bool acquireImage(std::string& fileName, ImageDesc& desc, bool generateMipMaps);

    /// Utility to create a solid color color image 
    /// @param color Color to set
    /// @param imageDesc Description of image updated during load.
    /// @return if creation succeeded
    virtual bool createColorImage(float color[4],
                                  ImageDesc& imageDesc);
 
    /// Bind an image. Derived classes must implement this method
    /// to handle logical binding of an image resource.
    /// @param identifier Identifier for image description to bind.
    /// @param samplingProperties Sampling properties for the image
    /// @return true if succeded to bind
    virtual bool bindImage(const std::string& /*identifier*/, const ImageSamplingProperties& /*samplingProperties*/) = 0;

    /// Clear the contents of the image cache.
    /// deleteImage() will be called for each cache description to 
    /// allow derived classes to clean up any associated resources.
    virtual void clearImageCache()
    {
        _imageCache.clear();
    }

  protected:
    /// Cache an image for reuse.
    /// @param identifier Description identifier to use.
    /// @param imageDesc Image description to cache
    void cacheImage(const std::string& identifier, const ImageDesc& imageDesc);

    /// Remove image description from the cache.
    /// @param identifier Identifier of description to remove.
    void uncacheImage(const std::string& identifier);

    /// Get an image description in the image cache if it exists
    /// @param identifier Description to search for.
    /// @return A null ptr is returned if not found.
    const ImageDesc* getCachedImage(const std::string& identifier);

    /// Return a reference to the image cache
    ImageDescCache& getImageCache()
    {
        return _imageCache;
    }

    /// Delete an image
    /// @param imageDesc Image description indicate which image to delete.
    /// Derived classes must implement this method to clean up resources
    /// when the image cache is cleared.
    virtual void deleteImage(ImageDesc& /*imageDesc*/) = 0;

    /// Image loader utilities
    ImageLoaderMap _imageLoaders;
    /// Image description cache
    ImageDescCache _imageCache;
};

} // namespace MaterialX
#endif
