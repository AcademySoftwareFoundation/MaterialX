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

    /// Bind an image. This method will bind the texture to an active texture
    /// unit as defined by the corresponding image description. The method
    /// will fail if there are not enough available image units to bind to.
    bool bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties) override;

    /// Unbind an image. 
    bool unbindImage(ImagePtr image) override;

    /// Create rendering resources for the given image.
    bool createRenderResources(ImagePtr image, bool generateMipMaps) override;

    // Release rendering resources for the given image.
    void releaseRenderResources(ImagePtr image) override;

    /// Return the bound texture location for a given resource
    int getBoundTextureLocation(unsigned int resourceId);

    /// Utility to map an address mode enumeration to an OpenGL address mode
    static int mapAddressModeToGL(ImageSamplingProperties::AddressMode addressModeEnum);

    /// Utility to map a filter type enumeration to an OpenGL filter type
    static int mapFilterTypeToGL(ImageSamplingProperties::FilterType filterTypeEnum, bool enableMipmaps);

    static void mapTextureFormatToGL(Image::BaseType baseType, unsigned int channelCount, bool srgb,
                                     int& glType, int& glFormat, int& glInternalFormat);

  protected:
    // Protected constructor
    GLTextureHandler(ImageLoaderPtr imageLoader);

    // Return the first free texture location that can be bound to.
    int getNextAvailableTextureLocation();

  protected:
    std::vector<unsigned int> _boundTextureLocations;
};

} // namespace MaterialX

#endif
