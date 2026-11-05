//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLTEXTUREHANDLER_H
#define MATERIALX_HLSLTEXTUREHANDLER_H

/// @file
/// HLSL texture handler - uploads MaterialX Images to D3D11 SRVs and
/// caches sampler states keyed by their MaterialX sampling properties.

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslContext.h>

#include <MaterialXRender/ImageHandler.h>

#include <unordered_map>

struct ID3D11SamplerState;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to an HLSL texture handler.
using HlslTextureHandlerPtr = std::shared_ptr<class HlslTextureHandler>;

/// @class HlslTextureHandler
/// An ImageHandler subclass that owns the D3D11 GPU resources for every
/// MaterialX Image bound through it. Mirrors the role of GLTextureHandler
/// (OpenGL) / MetalTextureHandler (Metal) / SlangTextureHandler.
///
/// bindImage uploads the image's pixels into an immutable
/// ID3D11Texture2D, builds a matching SRV, and caches a sampler state
/// keyed by the supplied ImageSamplingProperties. unbindImage releases
/// the cached resources for the image. Callers retrieve the bound SRV
/// and sampler via getBoundSrv / getBoundSampler when assigning them
/// to t# / s# slots on the immediate context.
class MX_RENDERHLSL_API HlslTextureHandler : public ImageHandler
{
  public:
    static HlslTextureHandlerPtr create(HlslContextPtr context, ImageLoaderPtr imageLoader)
    {
        return HlslTextureHandlerPtr(new HlslTextureHandler(std::move(context), std::move(imageLoader)));
    }

    ~HlslTextureHandler() override;

    HlslTextureHandler(const HlslTextureHandler&) = delete;
    HlslTextureHandler& operator=(const HlslTextureHandler&) = delete;

    /// Upload `image` and bind a sampler state matching the supplied
    /// sampling properties. Subsequent calls for the same image are
    /// served from the in-memory cache. Returns false on a null /
    /// unsupported image or on D3D11 resource creation failure.
    bool bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties) override;

    /// Release the cached GPU resources for `image`.
    bool unbindImage(ImagePtr image) override;

    /// Release every cached GPU resource.
    void releaseRenderResources(ImagePtr image = nullptr) override;

    /// Return the cached SRV for an image's resource id, or nullptr if
    /// the image has not been bound.
    ID3D11ShaderResourceView* getBoundSrv(unsigned int resourceId) const;

    /// Return the cached sampler for an image's resource id, or nullptr.
    ID3D11SamplerState* getBoundSampler(unsigned int resourceId) const;

  protected:
    HlslTextureHandler(HlslContextPtr context, ImageLoaderPtr imageLoader);

  private:
    struct CacheEntry
    {
        ID3D11Texture2D*          texture = nullptr;
        ID3D11ShaderResourceView* srv = nullptr;
        ID3D11SamplerState*       sampler = nullptr;
    };

    HlslContextPtr _context;
    // Cache keyed by Image::getResourceId() so callers can retrieve the
    // bound resources without holding the original ImagePtr.
    std::unordered_map<unsigned int, CacheEntry> _cache;
};

MATERIALX_NAMESPACE_END

#endif
