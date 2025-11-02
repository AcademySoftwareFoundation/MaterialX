//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SLANGTEXTUREHANDLER_H
#define MATERIALX_SLANGTEXTUREHANDLER_H

/// @file
/// OpenGL texture handler

#include <MaterialXRenderSlang/Export.h>
#include <MaterialXRenderSlang/SlangContext.h>

#include <MaterialXRender/ImageHandler.h>

#include <map>
#include <unordered_map>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to an Slang texture handler
using SlangTextureHandlerPtr = std::shared_ptr<class SlangTextureHandler>;
using SlangContextPtr = std::shared_ptr<class SlangContext>;

/// @class GLTextureHandler
/// An OpenGL texture handler class
class MX_RENDERSLANG_API SlangTextureHandler : public ImageHandler
{
    friend class SlangProgram;

  public:
    static SlangTextureHandlerPtr create(SlangContextPtr context, ImageLoaderPtr imageLoader)
    {
        return SlangTextureHandlerPtr(new SlangTextureHandler(std::move(context), imageLoader));
    }

    ~SlangTextureHandler() { }

    /// This method binds the image and its sampling properties.
    /// If the underlying resources are missing, they are created.
    /// The values are actually just cached in the Handler, and bound when bindImage with ShaderCursor is provided.
    bool bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties) override;

  protected:
    /// The actual image binding. The standard bindImage must have been called before.
    bool bindImage(rhi::ShaderCursor cursor, ImagePtr image);

  public:
    rhi::ComPtr<rhi::ISampler> getSamplerState(const ImageSamplingProperties& samplingProperties);

    /// Unbinding image is not supported, the image stays bound until another image is bound,
    /// to the same cursor.
    bool unbindImage(ImagePtr image) override { return true; }

    /// Create rendering resources for the given image.
    bool createRenderResources(ImagePtr image, bool generateMipMaps, bool useAsRenderTarget = false) override;

    /// Release rendering resources for the given image, or for all cached images
    /// if no image pointer is specified.
    void releaseRenderResources(ImagePtr image = nullptr) override;

    SlangTexturePtr getAssociatedSlangTexture(ImagePtr image);

    /// Utility to map an address mode enumeration to an OpenGL address mode
    static rhi::TextureAddressingMode mapAddressModeToSlang(ImageSamplingProperties::AddressMode addressModeEnum);

    /// Utility to map a filter type enumeration to an OpenGL filter type
    static rhi::TextureFilteringMode mapFilterTypeToSlang(ImageSamplingProperties::FilterType filterTypeEnum);

    /// Utility to map generic texture properties to OpenGL texture formats.
    static rhi::Format mapTextureFormatToSlang(Image::BaseType baseType, unsigned int channelCount, bool srgb);

    /// Makes sure the image has CPU data. If force is set to true, it will overwrite the data from GPU even if it exists.
    ImagePtr uploadImageToCPU(ImagePtr image, bool force = false);

  protected:
    // Protected constructor
    SlangTextureHandler(SlangContextPtr context, ImageLoaderPtr imageLoader);

    rhi::ComPtr<rhi::ITexture> makeTexture(SlangCommandEncoderPtr& commandEncoder, Image* srcImage, bool generateMipMaps, bool useAsRenderTarget);
    rhi::ComPtr<rhi::IComputePipeline> getPipeline(Image* srcImage);
    rhi::ComPtr<rhi::IComputePipeline> getPipeline(unsigned channelCount, Image::BaseType baseType);
  protected:
    SlangContextPtr _context;
    rhi::ComPtr<rhi::IDevice> _device;

    std::map<unsigned, std::pair<ImagePtr, ImageSamplingProperties>> _imageBindingInfo;
    std::map<unsigned, SlangTexturePtr> _slangTextureMap;
    // std::map<ImagePtr, rhi::ComPtr<rhi::ISampler>> _boundSamplers;
    std::unordered_map<ImageSamplingProperties, rhi::ComPtr<rhi::ISampler>, ImageSamplingKeyHasher> _imageSamplerStateMap;

    std::map<std::pair<uint32_t, Image::BaseType>, rhi::ComPtr<rhi::IComputePipeline>> _pipelineCache;
    static const std::string CONVERSION_SHADER;
};

MATERIALX_NAMESPACE_END

#endif