//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLD3D12TEXTUREHANDLER_H
#define MATERIALX_HLSLD3D12TEXTUREHANDLER_H

/// @file
/// D3D12 texture handler - uploads MaterialX Images to D3D12 textures and
/// keeps staging descriptors for their views and samplers.

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslD3D12Context.h>

#include <MaterialXRender/ImageHandler.h>

struct ID3D12Resource;

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a D3D12 texture handler.
using HlslD3D12TextureHandlerPtr = std::shared_ptr<class HlslD3D12TextureHandler>;

/// @class HlslD3D12TextureHandler
/// An ImageHandler subclass that owns the D3D12 GPU resources for every
/// MaterialX Image bound through it; the D3D12 counterpart of
/// HlslTextureHandler.
///
/// bindImage uploads the image with a full mip chain, generated on the CPU
/// with a 2x2 box filter since D3D12 has no GenerateMips, and writes a
/// shader resource view and a sampler into staging descriptors. Callers
/// copy those descriptors into shader-visible tables at draw time.
class MX_RENDERHLSL_API HlslD3D12TextureHandler : public ImageHandler
{
  public:
    static HlslD3D12TextureHandlerPtr create(HlslD3D12ContextPtr context, ImageLoaderPtr imageLoader)
    {
        return HlslD3D12TextureHandlerPtr(new HlslD3D12TextureHandler(std::move(context), std::move(imageLoader)));
    }

    ~HlslD3D12TextureHandler() override;

    HlslD3D12TextureHandler(const HlslD3D12TextureHandler&) = delete;
    HlslD3D12TextureHandler& operator=(const HlslD3D12TextureHandler&) = delete;

    /// Upload `image` if it is not already resident, and write a sampler
    /// matching `samplingProperties`. Returns false on a null or
    /// unsupported image or on resource creation failure.
    bool bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties) override;

    /// Upload `image` with an explicit mip chain, for textures whose levels
    /// are not a box-filtered reduction of the base level, such as
    /// prefiltered environment maps. `mipLevels[0]` holds the base level;
    /// each following image is half the size of the previous one, with the
    /// same channel count and base type. `image` identifies the texture in
    /// later calls to bindImage. Returns false on invalid input.
    bool bindImageWithMips(ImagePtr image, const std::vector<ImagePtr>& mipLevels,
                           const ImageSamplingProperties& samplingProperties);

    /// Release the GPU resources of `image`.
    bool unbindImage(ImagePtr image) override;

    /// Release the GPU resources of `image`, or of every image if null.
    void releaseRenderResources(ImagePtr image = nullptr) override;

    /// Staging descriptor (D3D12_CPU_DESCRIPTOR_HANDLE::ptr) of the shader
    /// resource view of a bound image, or 0 if the image is not bound.
    uint64_t getBoundSrv(unsigned int resourceId) const;

    /// Staging descriptor of the sampler of a bound image, or 0.
    uint64_t getBoundSampler(unsigned int resourceId) const;

    /// Borrowed texture resource of a bound image, or nullptr.
    ID3D12Resource* getBoundTexture(unsigned int resourceId) const;

  protected:
    HlslD3D12TextureHandler(HlslD3D12ContextPtr context, ImageLoaderPtr imageLoader);

  private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

MATERIALX_NAMESPACE_END

#endif
