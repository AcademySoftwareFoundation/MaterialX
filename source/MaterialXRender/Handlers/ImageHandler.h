//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_IMAGEHANDLER_H
#define MATERIALX_IMAGEHANDLER_H

/// @file
/// Image handler interfaces

#include <MaterialXCore/Types.h>

#include <cmath>
#include <map>
#include <array>

#include <MaterialXFormat/File.h>

namespace MaterialX
{
/// @class ImageDesc
/// Interface to describe an image. Images are assumed to be float type.
class ImageDesc
{
  public:
    /// Image base type identifier
    using BaseType = string;
    /// Set of base type identifiers
    using BaseTypeSet = std::set<BaseType>;

    /// Preset base type identifiers
    static BaseType BASETYPE_UINT8;
    static BaseType BASETYPE_HALF;
    static BaseType BASETYPE_FLOAT;

    /// Image type identifier
    using ImageType = string;

    /// Preset image type identifiers
    static ImageType IMAGETYPE_2D;

    /// Image width
    unsigned int width = 0; 
    /// Image height
    unsigned int height = 0;
    /// Number of channels
    unsigned int channelCount = 0;
    /// Number of mip map levels
    unsigned int mipCount = 0;
    /// CPU buffer. May be empty
    void* resourceBuffer = nullptr;
    /// Base type
    BaseType baseType = BASETYPE_UINT8;
    /// Image Type
    ImageType imageType = IMAGETYPE_2D;
    /// Hardware target dependent resource identifier. May be undefined.
    unsigned int resourceId = 0;

    /// Compute the number of mip map levels based on size of the image
    void computeMipCount()
    {
        mipCount = (unsigned int)std::log2(std::max(width, height)) + 1;
    }
};

/// Structure containing harware image description restrictions
class ImageDescRestrictions
{
  public:
    /// List of base types that can be supported
    ImageDesc::BaseTypeSet supportedBaseTypes;
};

/// @class ImageSamplingProperties
/// Interface to describe sampling properties for images.
class ImageSamplingProperties
{
  public:
    /// Address mode in U
    int uaddressMode = -1;
    /// Address mode in V
    int vaddressMode = -1;
    /// Filter type
    int filterType = -1;
    /// Default color
    Color4 defaultColor = { 0.0f, 0.0f, 0.0f, 1.0f };
};

/// Image description cache
using ImageDescCache = std::unordered_map<string, ImageDesc>;

/// Shared pointer to an ImageLoader
using ImageLoaderPtr = std::shared_ptr<class ImageLoader>;

/// @class ImageLoader
/// Abstract class representing an disk image loader
///
class ImageLoader
{
  public:
    /// Default constructor
    ImageLoader() {}

    /// Default destructor
    virtual ~ImageLoader() {}

    /// Stock extension names
    static string BMP_EXTENSION;
    static string EXR_EXTENSION;
    static string GIF_EXTENSION;
    static string HDR_EXTENSION;
    static string JPG_EXTENSION;
    static string JPEG_EXTENSION;
    static string PIC_EXTENSION;
    static string PNG_EXTENSION;
    static string PSD_EXTENSION;
    static string TGA_EXTENSION;
    static string TIF_EXTENSION;
    static string TIFF_EXTENSION;
    static string TXT_EXTENSION;
    static string TX_EXTENSION;
    static string TXR_EXTENSION;

    /// Returns a list of supported extensions
    /// @return List of support extensions
    const StringSet& supportedExtensions()
    {
        return _extensions;
    }

    /// Save image to disk. This method must be implemented by derived classes.
    /// @param filePath Path to save image to
    /// @param imageDesc Description of image
    /// @return if save succeeded
    /// @param verticalFlip Whether the image should be flipped in Y during save
    /// @return if save succeeded
    virtual bool saveImage(const FilePath& filePath,
                           const ImageDesc &imageDesc,
                           bool verticalFlip = false) = 0;

    /// Acquire an image from disk. This method must be implemented by derived classes.
    /// @param filePath Path to load image from
    /// @param imageDesc Description of image updated during load.
    /// @param restrictions Hardware image description restrictions. Default value is nullptr, meaning no restrictions.
    /// @return if load succeeded
    virtual bool acquireImage(const FilePath& filePath, ImageDesc &imageDesc, 
                              const ImageDescRestrictions* restrictions = nullptr) = 0;

  protected:
    /// List of supported string extensions
    StringSet _extensions;
};

/// Shared pointer to an ImageHandler
using ImageHandlerPtr = std::shared_ptr<class ImageHandler>;

/// Map of extensions to image loaders
using ImageLoaderMap = std::multimap<string, ImageLoaderPtr>;

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

    /// Static instance create function
    static ImageHandlerPtr create(ImageLoaderPtr imageLoader)
    {
        return std::make_shared<ImageHandler>(imageLoader);
    }

    /// Add additional image loaders. Useful to handle different file
    /// extensions
    /// @param loader Loader to add to list of available loaders.
    void addLoader(ImageLoaderPtr loader);
    
    /// Default destructor
    virtual ~ImageHandler() { };

    /// Get a list of extensions supported by the handler
    void supportedExtensions(StringSet& extensions);

    /// Save image to disk. This method must be implemented by derived classes.
    /// The first image loader which supports the file name extension will be used.
    /// @param filePath Name of file to save image to
    /// @param verticalFlip Whether the image should be flipped in Y during save
    /// @return if save succeeded
    virtual bool saveImage(const FilePath& filePath,
                           const ImageDesc &imageDesc,
                           bool verticalFlip = false);

    /// Acquire an image from disk. This method must be implemented by derived classes.
    /// The first image loader which supports the file name extension will be used.
    /// @param filePath Name of file to load image from.
    /// @param desc Description of image updated during load.
    /// @param generateMipMaps Generate mip maps if supported.
    /// @param fallbackColor Color of fallback image to use if failed to load.  If null is specified then
    /// no fallback image will be acquired.
    /// @return if load succeeded in loading image or created fallback image.
    virtual bool acquireImage(const FilePath& filePath, ImageDesc& desc, bool generateMipMaps, const Color4* fallbackColor);

    /// Utility to create a solid color color image 
    /// @param color Color to set
    /// @param imageDesc Description of image updated during load.
    /// @return if creation succeeded
    virtual bool createColorImage(const Color4& color,
                                  ImageDesc& imageDesc);
 
    /// Bind an image. Derived classes should implement this method to handle logical binding of 
    /// an image resource. The default implementation performs no action.
    /// @param identifier Identifier for image description to bind.
    /// @param samplingProperties Sampling properties for the image
    /// @return true if succeded to bind
    virtual bool bindImage(const string& identifier, const ImageSamplingProperties& samplingProperties);

    /// Clear the contents of the image cache.
    /// deleteImage() will be called for each cache description to 
    /// allow derived classes to clean up any associated resources.
    virtual void clearImageCache()
    {
        _imageCache.clear();
    }

    /// Set to the search path used for finding image files.
    void setSearchPath(const FileSearchPath& path);

    /// Resolve a path to a file using the registered search paths.
    FilePath findFile(const FilePath& filePath);

    /// Get the image search path
    const FileSearchPath& searchPath()
    {
        return _searchPath;
    }

  protected:
    /// Cache an image for reuse.
    /// @param identifier Description identifier to use.
    /// @param imageDesc Image description to cache
    void cacheImage(const string& identifier, const ImageDesc& imageDesc);

    /// Remove image description from the cache.
    /// @param identifier Identifier of description to remove.
    void uncacheImage(const string& identifier);

    /// Get an image description in the image cache if it exists
    /// @param identifier Description to search for.
    /// @return A null ptr is returned if not found.
    const ImageDesc* getCachedImage(const string& identifier);

    /// Return a reference to the image cache
    ImageDescCache& getImageCache()
    {
        return _imageCache;
    }

    /// Delete an image
    /// @param imageDesc Image description indicate which image to delete.
    /// Derived classes should override this method to clean up any related resources
    /// an image is deleted from the handler.
    virtual void deleteImage(ImageDesc& imageDesc);

    /// Return image description restrictions. By default nullptr is
    /// returned meaning no restrictions. Derived classes can override
    /// this to add restrictions specific to that handler.
    virtual const ImageDescRestrictions* getRestrictions() const { return nullptr; }

    /// Image loader utilities
    ImageLoaderMap _imageLoaders;
    /// Image description cache
    ImageDescCache _imageCache;

    /// Filename search path
    FileSearchPath _searchPath;
};

} // namespace MaterialX
#endif
