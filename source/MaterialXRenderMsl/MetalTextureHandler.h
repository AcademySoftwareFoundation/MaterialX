//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GLTEXTUREHANDLER_H
#define MATERIALX_GLTEXTUREHANDLER_H

/// @file
/// Metal texture handler

#include <MaterialXRenderMsl/Export.h>

#include <MaterialXRender/ImageHandler.h>

#include <unordered_map>
#include <stack>

#import <Metal/Metal.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to an Metal texture handler
using MetalTextureHandlerPtr = std::shared_ptr<class MetalTextureHandler>;

/// @class MetalTextureHandler
/// An Metal texture handler class
class MX_RENDERMSL_API MetalTextureHandler : public ImageHandler
{
  public:
    static MetalTextureHandlerPtr create(id<MTLDevice> device, ImageLoaderPtr imageLoader)
    {
        return MetalTextureHandlerPtr(new MetalTextureHandler(device, imageLoader));
        //std::make_shared<MetalTextureHandler>(device, imageLoader);// MetalTextureHandlerPtr(imageLoader));
    }

    /// Bind an image. This method will bind the texture to an active texture
    /// unit as defined by the corresponding image description. The method
    /// will fail if there are not enough available image units to bind to.
    bool bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties) override;
    
    bool bindImage(id<MTLRenderCommandEncoder> renderCmdEncoder,
                   int textureUnit,
                   ImagePtr image);
    
    id<MTLSamplerState> getSamplerState(const ImageSamplingProperties& samplingProperties);
    
    /// Unbind an image.
    bool unbindImage(ImagePtr image) override;

    id<MTLTexture>      getMTLTextureForImage(unsigned int index) const;
    id<MTLSamplerState> getMTLSamplerStateForImage(unsigned int index);
    
    /// Create rendering resources for the given image.
    bool createRenderResources(ImagePtr image, bool generateMipMaps) override;

    /// Release rendering resources for the given image, or for all cached images
    /// if no image pointer is specified.
    void releaseRenderResources(ImagePtr image = nullptr) override;

    /// Return the bound texture location for a given resource
    int getBoundTextureLocation(unsigned int resourceId);

    /// Utility to map an address mode enumeration to an Metal address mode
    static MTLSamplerAddressMode mapAddressModeToMetal(ImageSamplingProperties::AddressMode addressModeEnum);

    /// Utility to map a filter type enumeration to an Metal filter type
    static void mapFilterTypeToMetal(ImageSamplingProperties::FilterType filterTypeEnum, bool enableMipmaps, MTLSamplerMinMagFilter& minMagFilter, MTLSamplerMipFilter& mipFilter);

    /// Utility to map generic texture properties to Metal texture formats.
    static void mapTextureFormatToMetal(Image::BaseType baseType, unsigned int channelCount, bool srgb,
                                        MTLDataType& dataType, MTLPixelFormat& pixelFormat);
    
    static size_t getTextureBaseTypeSize(Image::BaseType baseType);

  protected:
    // Protected constructor
    MetalTextureHandler(id<MTLDevice> device, ImageLoaderPtr imageLoader);

  protected:
    std::vector<unsigned int> _boundTextureLocations;
    
    std::unordered_map<unsigned int, id<MTLTexture>> _metalTextureMap;
    std::unordered_map<unsigned int, std::pair<ImagePtr, ImageSamplingProperties>> _imageBindingInfo;
    std::unordered_map<ImageSamplingProperties, id<MTLSamplerState>, ImageSamplingKeyHasher> _imageSamplerStateMap;
    
    id<MTLDevice> _device;
};

MATERIALX_NAMESPACE_END

#endif
