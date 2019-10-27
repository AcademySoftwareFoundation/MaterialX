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
    static ImageHandlerPtr create(ImageLoaderPtr imageLoader)
    {
        return ImageHandlerPtr(new GLTextureHandler(imageLoader));
    }

    virtual ~GLTextureHandler() { }

    /// Acquire an image from the cache or file system.  If the image is not
    /// found in the cache, then each image loader will be applied in turn.
    bool acquireImage(const FilePath& filePath,
                      ImageDesc& imageDesc,
                      bool generateMipMaps,
                      const Color4* fallbackColor = nullptr) override;

    /// Bind an image. This method will bind the texture to an active texture
    /// unit as defined by the corresponding image description. The method
    /// will fail if there are not enough available image units to bind to.
    bool bindImage(const ImageDesc& desc, const ImageSamplingProperties& samplingProperties) override;

    /// Unbind an image. 
    virtual bool unbindImage(const ImageDesc& desc) override;

    /// Utility to map an address mode enumeration to an OpenGL address mode
    static int mapAddressModeToGL(ImageSamplingProperties::AddressMode addressModeEnum);

    /// Utility to map a filter type enumeration to an OpenGL filter type
    static int mapFilterTypeToGL(ImageSamplingProperties::FilterType filterTypeEnum);

    /// Returns the bound texture location for a given resource
    int getBoundTextureLocation(unsigned int resourceId) override;

  protected:
    // Protected constructor
    GLTextureHandler(ImageLoaderPtr imageLoader);

    // Delete an image. Any OpenGL texture resource and as well as any CPU-side
    // resource memory will be deleted.
    void deleteImage(ImageDesc& imageDesc) override;

    // Return restrictions specific to this handler
    const ImageDescRestrictions* getRestrictions() const override
    {
        return &_restrictions;
    }

    // Return the first free texture location that can be bound to.
    int getNextAvailableTextureLocation();

  protected:
    // Maximum number of available image units
    int _maxImageUnits;

    // Support restrictions
    ImageDescRestrictions _restrictions;

    std::vector<unsigned int> _boundTextureLocations;
};

} // namespace MaterialX

#endif
