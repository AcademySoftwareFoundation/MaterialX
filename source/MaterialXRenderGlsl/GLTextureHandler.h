//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GLTEXTUREHANDLER_H
#define MATERIALX_GLTEXTUREHANDLER_H

/// @file
/// OpenGL texture handler

#include <MaterialXRender/ImageHandler.h>

namespace MaterialX
{
/// Shared pointer to an OpenGL texture handler
using GLTextureHandlerPtr = std::shared_ptr<class GLTextureHandler>;

/// @class GLTextureHandler
/// An OpenGL texture handler class
class GLTextureHandler : public ImageHandler
{
  public:
    /// Static instance create function
    static GLTextureHandlerPtr create(ImageLoaderPtr imageLoader)
    {
        return std::make_shared<GLTextureHandler>(imageLoader);
    }

    /// Default constructor
    GLTextureHandler(ImageLoaderPtr imageLoader);

    /// Default destructor
    virtual ~GLTextureHandler() {}


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
    bool acquireImage(const FilePath& filePath,
                      ImageDesc& imageDesc,
                      bool generateMipMaps,
                      const Color4* fallbackColor = nullptr) override;

    /// Bind an image. This method will bind the texture to an active texture
    /// unit as defined by the corresponding image description. The method
    /// will fail if there are not enough available image units to bind to.
    /// @param identifier Identifier for image description to bind.
    /// @param samplingProperties Sampling properties for the image
    /// @return true if succeded to bind
    bool bindImage(const FilePath& filePath, const ImageSamplingProperties& samplingProperties) override;

    /// Utility to map an address mode enumeration to an OpenGL address mode
    static int mapAddressModeToGL(ImageSamplingProperties::AddressMode addressModeEnum);

    /// Utility to map a filter type enumeration to an OpenGL filter type
    static int mapFilterTypeToGL(ImageSamplingProperties::FilterType filterTypeEnum);

    /// Returns the bound texture location for a given resource
    int getBoundTextureLocation(unsigned int resourceId) override;

  protected:
    /// Delete an image
    /// @param imageDesc Image description indicate which image to delete.
    /// Any OpenGL texture resource and as well as any CPU side reosurce memory will be deleted.
    void deleteImage(MaterialX::ImageDesc& imageDesc) override;

    /// Return restrictions specific to this handler
    const ImageDescRestrictions* getRestrictions() const override
    {
        return &_restrictions;
    }

    /// Returns the first free texture location that can be bound to.
    int getNextAvailableTextureLocation();

    /// Maximum number of available image units
    int _maxImageUnits;

    /// Support restrictions
    ImageDescRestrictions _restrictions;

    std::vector<unsigned int> _boundTextureLocations;
};

} // namespace MaterialX
#endif
