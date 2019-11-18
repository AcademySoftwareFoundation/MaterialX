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

#include <map>

namespace MaterialX
{

extern const string IMAGE_PROPERTY_SEPARATOR;
extern const string UADDRESS_MODE_SUFFIX;
extern const string VADDRESS_MODE_SUFFIX;
extern const string FILTER_TYPE_SUFFIX;
extern const string DEFAULT_COLOR_SUFFIX;

class Image;
class VariableBlock;

/// A shared pointer to an image
using ImagePtr = shared_ptr<Image>;

/// A function to perform image buffer deallocation
using ImageBufferDeallocator = std::function<void(void*)>;

/// @class Image
/// A class representing an image in memory
class Image
{
  public:
    enum class BaseType
    {
        UINT8 = 0,
        HALF = 1,
        FLOAT = 2
    };

  public:
    /// Create an empty image with the given properties.
    static ImagePtr create(unsigned int width, unsigned int height, unsigned int channelCount, BaseType baseType = BaseType::UINT8)
    {
        return ImagePtr(new Image(width, height, channelCount, baseType));
    }

    /// Create a constant color image with the given properties.
    static ImagePtr createConstantColor(unsigned int width, unsigned int height, const Color4& color);

    ~Image();

    /// Return the width of the image.
    unsigned int getWidth() const
    {
        return _width;
    }

    /// Return the height of the image.
    unsigned int getHeight() const
    {
        return _height;
    }

    /// Return the channel count of the image.
    unsigned int getChannelCount() const
    {
        return _channelCount;
    }

    /// Return the base type of the image.
    BaseType getBaseType() const
    {
        return _baseType;
    }

    /// Return the stride of our base type in bytes.
    unsigned int getBaseStride() const;

    /// Return the maximum number of mipmaps for this image.
    unsigned int getMaxMipCount() const;

    /// Set the resource buffer for this image.
    void setResourceBuffer(void* buffer)
    {
        _resourceBuffer = buffer;
    }

    /// Return the resource buffer for this image.
    void* getResourceBuffer() const
    {
        return _resourceBuffer;
    }

    /// Set the resource buffer deallocator for this image.
    void setResourceBufferDeallocator(ImageBufferDeallocator deallocator)
    {
        _resourceBufferDeallocator = deallocator;
    }

    /// Return the resource buffer deallocator for this image.
    ImageBufferDeallocator getResourceBufferDeallocator() const
    {
        return _resourceBufferDeallocator;
    }

    /// Set the resource ID for this image.
    void setResourceId(unsigned int id)
    {
        _resourceId = id;
    }

    /// Return the resource ID for this image.
    unsigned int getResourceId() const
    {
        return _resourceId;
    }

    /// Return the texel color at the given coordinates.  If the coordinates
    /// or image resource buffer are invalid, then an exception is thrown.
    Color4 getTexelColor(unsigned int x, unsigned int y) const;

    /// Allocate a resource buffer for this image that matches its properties.
    void createResourceBuffer();

    /// Release the resource buffer for this image.
    void releaseResourceBuffer();

  protected:
    Image(unsigned int width, unsigned int height, unsigned int channelCount, BaseType baseType);

  protected:
    unsigned int _width;
    unsigned int _height;
    unsigned int _channelCount;
    BaseType _baseType;

    void* _resourceBuffer;
    ImageBufferDeallocator _resourceBufferDeallocator;
    unsigned int _resourceId;
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

/// A map from strings to images.
using ImageMap = std::unordered_map<string, ImagePtr>;

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

    /// Standard image file extensions
    static const string BMP_EXTENSION;
    static const string EXR_EXTENSION;
    static const string GIF_EXTENSION;
    static const string HDR_EXTENSION;
    static const string JPG_EXTENSION;
    static const string JPEG_EXTENSION;
    static const string PIC_EXTENSION;
    static const string PNG_EXTENSION;
    static const string PSD_EXTENSION;
    static const string TGA_EXTENSION;
    static const string TIF_EXTENSION;
    static const string TIFF_EXTENSION;
    static const string TXT_EXTENSION;
    static const string TX_EXTENSION;
    static const string TXR_EXTENSION;

    /// Returns a list of supported extensions
    /// @return List of support extensions
    const StringSet& supportedExtensions() const
    {
        return _extensions;
    }

    /// Save an image to the file system. This method must be implemented by derived classes.
    /// @param filePath Path to save image to
    /// @param imageDesc Description of image
    /// @param verticalFlip Whether the image should be flipped in Y during save
    /// @return if save succeeded
    virtual bool saveImage(const FilePath& filePath,
                           ImagePtr image,
                           bool verticalFlip = false) = 0;

    /// Load an image from the file system. This method must be implemented by derived classes.
    /// @param filePath The requested image file path.
    /// @return On success, a shared pointer to the loaded image; otherwise an empty shared pointer.
    virtual ImagePtr loadImage(const FilePath& filePath) = 0;

  protected:
    // List of supported string extensions
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
                           ImagePtr image,
                           bool verticalFlip = false);

    /// Acquire an image from the cache or file system.  If the image is not
    /// found in the cache, then each image loader will be applied in turn.
    /// @param filePath File path of the image.
    /// @param generateMipMaps Generate mip maps if supported.
    /// @param fallbackColor Optional uniform color of a fallback texture
    ///    to create when the image cannot be loaded from the file system.
    ///    By default, no fallback texture is created.
    /// @param message Optional pointer to a message string, where any warning
    ///    or error messages from the acquire operation will be stored.
    /// @return On success, a shared pointer to the acquired Image.
    virtual ImagePtr acquireImage(const FilePath& filePath,
                                  bool generateMipMaps,
                                  const Color4* fallbackColor = nullptr,
                                  string* message = nullptr);

    /// Bind an image. Derived classes should implement this method to handle logical binding of
    /// an image resource. The default implementation performs no action.
    /// @param desc The image to bind
    /// @param samplingProperties Sampling properties for the image
    virtual bool bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties);

    /// Unbind an image. The default implementation performs no action.
    /// @param desc The image to unbind
    virtual bool unbindImage(ImagePtr image);

    /// Clear the contents of the image cache.
    /// deleteImage() will be called for each cached image to
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

    /// Return the bound texture location for a given resource.
    virtual int getBoundTextureLocation(unsigned int)
    {
        return -1;
    }

  protected:
    // Protected constructor.
    ImageHandler(ImageLoaderPtr imageLoader);

    // Add an image to the cache.
    void cacheImage(const string& filePath, ImagePtr image);

    // Return the cached image, if found; otherwise return an empty
    // shared pointer.
    ImagePtr getCachedImage(const FilePath& filePath);

    // Delete an image. Derived classes should override this method to clean
    // up any related resources when an image is deleted from the handler.
    virtual void deleteImage(ImagePtr image);

  protected:
    ImageLoaderMap _imageLoaders;
    ImageMap _imageCache;
    FileSearchPath _searchPath;
    StringResolverPtr _resolver;
};

} // namespace MaterialX

#endif
