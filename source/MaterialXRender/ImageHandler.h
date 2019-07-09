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
class VariableBlock;

/// A function to perform image buffer deallocation
using ImageBufferDeallocator = std::function<void(void*)>;

/// @class ImageDesc
/// Interface to describe an image. Images are assumed to be float type.
class ImageDesc
{
  public:
    ~ImageDesc()
    {
        freeResourceBuffer();
    }

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
    /// Deallocator to free resource buffer memory. If not defined then malloc() is
    /// assumed to have been used to allocate the buffer and corresponding free() is
    /// used to deallocate.
    ImageBufferDeallocator resourceBufferDeallocator = [](void *buffer)
    {
        free(buffer);
    };

    /// Compute the number of mip map levels based on size of the image
    void computeMipCount()
    {
        mipCount = (unsigned int)std::log2(std::max(width, height)) + 1;
    }

    /// Free any resource buffer memory
    void freeResourceBuffer();
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
    /// Set the properties based on data in a uniform block.
    /// @param fileNameUniform Name of the file name uniform. Used to find
    ///        corresponding sampler data in the uniform block
    /// @param uniformBlock Block containing sampler uniforms
    void setProperties(const string& fileNameUniform,
                       const VariableBlock& uniformBlock);

    /// Address mode options. Matches enumerations
    /// allowed for <image> address modes, except
    /// UNSPECIFIED which indicates no explicit mode was
    /// defined.
    enum class AddressMode : int
    { 
        UNSPECIFIED = -1,
        CONSTANT = 0,
        CLAMP = 1, 
        PERIODIC = 2,
        MIRROR = 3
    };

    /// Address mode in U
    AddressMode uaddressMode = AddressMode::UNSPECIFIED;
    /// Address mode in V
    AddressMode vaddressMode = AddressMode::UNSPECIFIED;

    /// Filter type options. Matches enumerations
    /// allowed for <image> filter types, except
    /// UNSPECIFIED which indicates no explicit type was
    /// defined.
    enum class FilterType : int
    {
        UNSPECIFIED = -1,
        CLOSEST = 0,
        LINEAR = 1,
        CUBIC = 2
    };

    /// Filter type
    FilterType filterType = FilterType::UNSPECIFIED;

    /// Default color. Corresponds to the "default"
    /// value on the <image> node definition.
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
    /// @param verticalFlip Whether the image should be flipped in Y during save
    /// @return if save succeeded
    virtual bool saveImage(const FilePath& filePath,
                           const ImageDesc &imageDesc,
                           bool verticalFlip = false) = 0;

    /// Load an image from disk. This method must be implemented by derived classes.
    /// @param filePath Path to load image from
    /// @param imageDesc Description of image updated during load.
    /// @param restrictions Hardware image description restrictions. Default value is nullptr, meaning no restrictions.
    /// @return if load succeeded
    virtual bool loadImage(const FilePath& filePath, ImageDesc &imageDesc,
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
    virtual ~ImageHandler()
    {
        clearImageCache();
    };

    /// Get a list of extensions supported by the handler
    void supportedExtensions(StringSet& extensions);

    /// Save image to disk. This method must be implemented by derived classes.
    /// The first image loader which supports the file name extension will be used.
    /// @param filePath Name of file to save image to
    /// @param imageDesc Description of image
    /// @param verticalFlip Whether the image should be flipped in Y during save
    /// @return if save succeeded
    virtual bool saveImage(const FilePath& filePath,
                           const ImageDesc &imageDesc,
                           bool verticalFlip = false);

    /// Acquire an image from the cache or file system.  If the image is not
    /// found in the cache, then each image loader will be applied in turn.
    /// @param filePath File path of the image.
    /// @param imageDesc On success, this image descriptor will be filled out
    ///    and assigned ownership of a resource buffer.
    /// @param generateMipMaps Generate mip maps if supported.
    /// @param fallbackColor Optional uniform color of a fallback texture
    ///    to create when the image cannot be loaded from the file system.
    ///    By default, no fallback texture is created.
    /// @return True if the image was successfully found in the cache or
    ///    file system.  Returns false if this call generated a fallback
    ///    texture.
    virtual bool acquireImage(const FilePath& filePath,
                              ImageDesc& imageDesc,
                              bool generateMipMaps,
                              const Color4* fallbackColor = nullptr);

    /// Utility to create a solid color color image
    /// @param color Color to set
    /// @param imageDesc Description of image updated during load.
    /// @return if creation succeeded
    virtual bool createColorImage(const Color4& color,
                                  ImageDesc& imageDesc);

    /// Bind an image. Derived classes should implement this method to handle logical binding of
    /// an image resource. The default implementation performs no action.
    /// @param filePath File path of image description to bind.
    /// @param samplingProperties Sampling properties for the image
    /// @return true if succeded to bind
    virtual bool bindImage(const FilePath& filePath, const ImageSamplingProperties& samplingProperties);

    /// Unbind an image. The default implementation performs no action.
    /// @param filePath File path to image description to unbind
    virtual bool unbindImage(const FilePath& filePath);

    /// Clear the contents of the image cache.
    /// deleteImage() will be called for each cache description to
    /// allow derived classes to clean up any associated resources.
    virtual void clearImageCache();

    /// Set the search path to be used for finding images on the file system.
    void setSearchPath(const FileSearchPath& path)
    {
        _searchPath = path;
    }

    /// Return the image search path
    const FileSearchPath& getSearchPath()
    {
        return _searchPath;
    }

    /// Resolve a path to a file using the registered search paths.
    FilePath findFile(const FilePath& filePath);

    /// Returns the bound texture location for a given resource
    virtual int getBoundTextureLocation(unsigned int)
    {
        return -1;
    }

    /// Perform UDIM token replace using an input file path and a list of token
    /// replacements (UDIM identifiers). A new path will be created for 
    /// each identifier.
    /// @param filePath File path with UDIM token
    /// @param udimIdentifiers List of UDIM identifiers
    /// @returns List of file paths
    static FilePathVec getUdimPaths(const FilePath& filePath, const StringVec& udimIdentifiers);

    /// Compute the UDIM coordinates for a set of UDIM identifiers
    /// @return List of UDIM coordinates
    static vector<Vector2> getUdimCoordinates(const StringVec& udimIdentifiers);

  protected:
    /// Cache an image for reuse.
    /// @param filePath File path of image to cache.
    /// @param imageDesc Image description to cache.
    void cacheImage(const string& filePath, const ImageDesc& imageDesc);

    /// Remove image description from the cache.
    /// @param filePath File path of image to remove.
    void uncacheImage(const string& filePath);

    /// Get an image description in the image cache if it exists
    /// @param filePath File path of image to find in the cache.
    /// @return A null ptr is returned if not found.
    const ImageDesc* getCachedImage(const string& filePath);

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
