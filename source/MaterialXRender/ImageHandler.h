//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_IMAGEHANDLER_H
#define MATERIALX_IMAGEHANDLER_H

/// @file
/// Image handler interfaces

#include <MaterialXFormat/File.h>

#include <MaterialXCore/Element.h>
#include <MaterialXCore/Types.h>

#include <cmath>
#include <map>

namespace MaterialX
{

extern const string IMAGE_PROPERTY_SEPARATOR;
extern const string UADDRESS_MODE_SUFFIX;
extern const string VADDRESS_MODE_SUFFIX;
extern const string FILTER_TYPE_SUFFIX;
extern const string DEFAULT_COLOR_SUFFIX;

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

    /// Compute the number of mip map levels based on size of the image
    void computeMipCount()
    {
        mipCount = (unsigned int) std::log2(std::max(width, height)) + 1;
    }

    /// Free any resource buffer memory
    void freeResourceBuffer();

  public:
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int channelCount = 0;
    unsigned int mipCount = 0;

    BaseType baseType = BASETYPE_UINT8;
    ImageType imageType = IMAGETYPE_2D;

    // CPU buffer. May be empty.
    void* resourceBuffer = nullptr;

    // Deallocator to free resource buffer memory. If not defined then malloc() is
    // assumed to have been used to allocate the buffer and corresponding free() is
    // used to deallocate.
    ImageBufferDeallocator resourceBufferDeallocator = nullptr;

    // Hardware target dependent resource identifier. May be undefined.
    unsigned int resourceId = 0;
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

    /// Address mode options. Matches enumerations allowed for image address
    /// modes, except UNSPECIFIED which indicates no explicit mode was defined.
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

    /// Filter type options. Matches enumerations allowed for image filter
    /// types, except UNSPECIFIED which indicates no explicit type was defined.
    enum class FilterType : int
    {
        UNSPECIFIED = -1,
        CLOSEST = 0,
        LINEAR = 1,
        CUBIC = 2
    };

    /// Filter type
    FilterType filterType = FilterType::UNSPECIFIED;

    /// Default color. Corresponds to the "default" value on the image
    /// node definition.
    Color4 defaultColor = { 0.0f, 0.0f, 0.0f, 1.0f };
};

/// A map from strings to image descriptions.
using ImageDescMap = std::unordered_map<string, ImageDesc>;

/// Shared pointer to an ImageLoader
using ImageLoaderPtr = std::shared_ptr<class ImageLoader>;

/// @class ImageLoader
/// Abstract base class for file-system image loaders
class ImageLoader
{
  public:
    ImageLoader()
    {
    }
    virtual ~ImageLoader() { }

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
    const StringSet& supportedExtensions() const
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

/// @class ImageHandler
/// Base image handler class. Keeps track of images which are loaded from
/// disk via supplied ImageLoader. Derived classes are responsible for
/// determinining how to perform the logic for "binding" of these resources
/// for a given target (such as a given shading language).
class ImageHandler
{
  public:
    static ImageHandlerPtr create(ImageLoaderPtr imageLoader)
    {
        return ImageHandlerPtr(new ImageHandler(imageLoader));
    }

    virtual ~ImageHandler()
    {
        clearImageCache();
    }

    /// Add another image loader to the handler, which will be invoked if
    /// existing loaders cannot load a given image.
    void addLoader(ImageLoaderPtr loader);

    /// Get a list of extensions supported by the handler.
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
    /// @param color Uniform color of the image to create
    /// @param imageDesc On success, the newly created image description
    /// @return True if the image was successfully created
    virtual bool createColorImage(const Color4& color, ImageDesc& imageDesc);

    /// Bind an image. Derived classes should implement this method to handle logical binding of
    /// an image resource. The default implementation performs no action.
    /// @param desc The image description to bind
    /// @param samplingProperties Sampling properties for the image
    virtual bool bindImage(const ImageDesc& desc, const ImageSamplingProperties& samplingProperties);

    /// Unbind an image. The default implementation performs no action.
    /// @param desc The image description to unbind
    virtual bool unbindImage(const ImageDesc& desc);

    /// Clear the contents of the image cache.
    /// deleteImage() will be called for each cache description to
    /// allow derived classes to clean up any associated resources.
    void clearImageCache();

    /// Set the search path to be used for finding images on the file system.
    void setSearchPath(const FileSearchPath& path)
    {
        _searchPath = path;
    }

    /// Return the image search path
    const FileSearchPath& getSearchPath() const
    {
        return _searchPath;
    }

    /// Set the filename resolver for images.
    void setFilenameResolver(StringResolverPtr resolver)
    {
        _resolver = resolver;
    }

    /// Return the filename resolver for images.
    StringResolverPtr getFilenameResolver() const
    {
        return _resolver;
    }

    /// Find the given file on the registered search path.
    FilePath findFile(const FilePath& filePath)
    {
        return _searchPath.find(filePath);
    }

    /// Return the bound texture location for a given resource.
    virtual int getBoundTextureLocation(unsigned int)
    {
        return -1;
    }

  protected:
    // Protected constructor.
    ImageHandler(ImageLoaderPtr imageLoader);

    // Add an image description to the cache.
    void cacheImage(const string& filePath, const ImageDesc& imageDesc);

    // Return the cached image description, if found; otherwise
    // return a null pointer.
    const ImageDesc* getCachedImage(const FilePath& filePath);

    // Delete an image. Derived classes should override this method to clean
    // up any related resources when an image is deleted from the handler.
    virtual void deleteImage(ImageDesc& imageDesc);

    // Return image description restrictions. By default nullptr is
    // returned meaning no restrictions. Derived classes can override
    // this to add restrictions specific to that handler.
    virtual const ImageDescRestrictions* getRestrictions() const { return nullptr; }

  protected:
    ImageLoaderMap _imageLoaders;
    ImageDescMap _imageCache;
    FileSearchPath _searchPath;
    StringResolverPtr _resolver;
};

} // namespace MaterialX

#endif
