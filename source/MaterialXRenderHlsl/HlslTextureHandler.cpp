//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslTextureHandler.h>

#include <atomic>
#include <vector>

#include <Windows.h>
#include <d3d11.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

void releaseAndNull(IUnknown** ptr)
{
    if (ptr && *ptr)
    {
        (*ptr)->Release();
        *ptr = nullptr;
    }
}

// Map a (channels, base type) pair to a DXGI format and the byte size of
// one pixel. Returns DXGI_FORMAT_UNKNOWN for unsupported combinations.
DXGI_FORMAT toDxgiFormat(unsigned int channels, Image::BaseType baseType, UINT& bytesPerPixel)
{
    bytesPerPixel = 0;
    switch (baseType)
    {
        case Image::BaseType::UINT8:
            switch (channels)
            {
                case 1: bytesPerPixel = 1; return DXGI_FORMAT_R8_UNORM;
                case 2: bytesPerPixel = 2; return DXGI_FORMAT_R8G8_UNORM;
                case 4: bytesPerPixel = 4; return DXGI_FORMAT_R8G8B8A8_UNORM;
                default: break;
            }
            break;
        case Image::BaseType::HALF:
            switch (channels)
            {
                case 1: bytesPerPixel = 2; return DXGI_FORMAT_R16_FLOAT;
                case 2: bytesPerPixel = 4; return DXGI_FORMAT_R16G16_FLOAT;
                case 4: bytesPerPixel = 8; return DXGI_FORMAT_R16G16B16A16_FLOAT;
                default: break;
            }
            break;
        case Image::BaseType::FLOAT:
            switch (channels)
            {
                case 1: bytesPerPixel = 4;  return DXGI_FORMAT_R32_FLOAT;
                case 2: bytesPerPixel = 8;  return DXGI_FORMAT_R32G32_FLOAT;
                case 3: bytesPerPixel = 12; return DXGI_FORMAT_R32G32B32_FLOAT;
                case 4: bytesPerPixel = 16; return DXGI_FORMAT_R32G32B32A32_FLOAT;
                default: break;
            }
            break;
        default:
            break;
    }
    return DXGI_FORMAT_UNKNOWN;
}

D3D11_TEXTURE_ADDRESS_MODE toAddressMode(ImageSamplingProperties::AddressMode mode)
{
    // Default to WRAP rather than CLAMP for unspecified - matches what
    // GLTextureHandler does (GL_REPEAT) so UV-tiled materials whose
    // shader doesn't emit explicit uaddressmode uniforms (wood_tiled,
    // brick_procedural, ...) repeat correctly out of the box.
    switch (mode)
    {
        case ImageSamplingProperties::AddressMode::CONSTANT:
            return D3D11_TEXTURE_ADDRESS_BORDER;
        case ImageSamplingProperties::AddressMode::CLAMP:
            return D3D11_TEXTURE_ADDRESS_CLAMP;
        case ImageSamplingProperties::AddressMode::PERIODIC:
            return D3D11_TEXTURE_ADDRESS_WRAP;
        case ImageSamplingProperties::AddressMode::MIRROR:
            return D3D11_TEXTURE_ADDRESS_MIRROR;
        case ImageSamplingProperties::AddressMode::UNSPECIFIED:
        default:
            return D3D11_TEXTURE_ADDRESS_WRAP;
    }
}

D3D11_FILTER toFilter(ImageSamplingProperties::FilterType filterType)
{
    switch (filterType)
    {
        case ImageSamplingProperties::FilterType::CLOSEST:
            return D3D11_FILTER_MIN_MAG_MIP_POINT;
        case ImageSamplingProperties::FilterType::CUBIC:
            // Cubic isn't a fixed-function D3D11 filter; degrade to
            // anisotropic so CUBIC at least gets the best available
            // sample quality the device offers.
            return D3D11_FILTER_ANISOTROPIC;
        case ImageSamplingProperties::FilterType::LINEAR:
        case ImageSamplingProperties::FilterType::UNSPECIFIED:
        default:
            return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    }
}

ID3D11SamplerState* createSampler(ID3D11Device* device,
                                  const ImageSamplingProperties& sp)
{
    D3D11_SAMPLER_DESC sd = {};
    sd.Filter = toFilter(sp.filterType);
    sd.AddressU = toAddressMode(sp.uaddressMode);
    sd.AddressV = toAddressMode(sp.vaddressMode);
    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.MipLODBias = 0.0f;
    sd.MaxAnisotropy = (sd.Filter == D3D11_FILTER_ANISOTROPIC) ? 16u : 1u;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.MinLOD = 0.0f;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    // Default border color matches the supplied default when CONSTANT
    // addressing is requested.
    sd.BorderColor[0] = sp.defaultColor[0];
    sd.BorderColor[1] = sp.defaultColor[1];
    sd.BorderColor[2] = sp.defaultColor[2];
    sd.BorderColor[3] = sp.defaultColor[3];

    ID3D11SamplerState* state = nullptr;
    if (FAILED(device->CreateSamplerState(&sd, &state)))
        return nullptr;
    return state;
}

} // namespace

HlslTextureHandler::HlslTextureHandler(HlslContextPtr context, ImageLoaderPtr imageLoader) :
    ImageHandler(std::move(imageLoader)),
    _context(std::move(context))
{
}

HlslTextureHandler::~HlslTextureHandler()
{
    releaseRenderResources();
}

bool HlslTextureHandler::bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties)
{
    if (!image || !_context || !_context->getDevice())
        return false;

    // Assign a unique resource id if the image does not already have one.
    // Image::_resourceId defaults to 0 for every newly-loaded Image, so
    // without this assignment the cache would alias every image to the
    // first one ever uploaded - producing the same texture (and sampler)
    // for every call after the first. We start at 1 to keep 0 reserved
    // as "no resource".
    if (image->getResourceId() == 0)
    {
        static std::atomic<unsigned int> s_nextId{1};
        image->setResourceId(s_nextId.fetch_add(1));
    }

    // Fast path: image already cached with a current SRV. Refresh only
    // the sampler if the supplied sampling properties might differ.
    auto it = _cache.find(image->getResourceId());
    if (it != _cache.end() && it->second.srv)
    {
        if (it->second.sampler)
        {
            it->second.sampler->Release();
            it->second.sampler = nullptr;
        }
        it->second.sampler = createSampler(_context->getDevice(), samplingProperties);
        return it->second.sampler != nullptr;
    }

    UINT bytesPerPixel = 0;
    UINT srcChannels = image->getChannelCount();
    DXGI_FORMAT format = toDxgiFormat(srcChannels,
                                      image->getBaseType(), bytesPerPixel);
    // D3D11 has no 24-bit RGB UNORM/HALF format. JPEGs (and any other
    // 3-channel UINT8/HALF source) need padding to 4 channels before
    // upload; with sRGB framebuffer encoding the alpha pad of 1.0 is
    // visually invisible and matches what the texture would have been
    // had the loader returned RGBA.
    std::vector<uint8_t> padded;
    const void* uploadData = image->getResourceBuffer();
    UINT uploadStride = 0;
    // Three-channel formats need padding because:
    //   - UINT8: D3D11 has no 24-bit RGB UNORM at all
    //   - HALF / FLOAT: 3-channel exists (R16G16B16/R32G32B32) but
    //     GenerateMips doesn't support them, so HDR env maps would
    //     produce garbage at non-zero mip levels and metals/glossy
    //     materials sampling at roughness>0 would render black.
    if (format == DXGI_FORMAT_UNKNOWN || (srcChannels == 3 &&
        (image->getBaseType() == Image::BaseType::HALF ||
         image->getBaseType() == Image::BaseType::FLOAT)))
    {
        if (srcChannels == 3 && (
            image->getBaseType() == Image::BaseType::UINT8 ||
            image->getBaseType() == Image::BaseType::HALF ||
            image->getBaseType() == Image::BaseType::FLOAT))
        {
            const UINT srcElemSize =
                (image->getBaseType() == Image::BaseType::UINT8) ? 1 :
                (image->getBaseType() == Image::BaseType::HALF)  ? 2 : 4;
            const UINT dstElemSize = srcElemSize;
            switch (image->getBaseType())
            {
                case Image::BaseType::UINT8:
                    format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
                case Image::BaseType::HALF:
                    format = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
                case Image::BaseType::FLOAT:
                    format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
                default: break;
            }
            bytesPerPixel = 4 * dstElemSize;
            const UINT w = image->getWidth();
            const UINT h = image->getHeight();
            padded.resize(static_cast<size_t>(w) * h * 4 * dstElemSize, 0);
            const uint8_t* src = static_cast<const uint8_t*>(image->getResourceBuffer());
            const uint16_t halfOne = 0x3C00;
            const float    floatOne = 1.0f;
            for (size_t i = 0; i < static_cast<size_t>(w) * h; ++i)
            {
                std::memcpy(&padded[i * 4 * dstElemSize],
                            src + i * 3 * srcElemSize,
                            3 * srcElemSize);
                if (image->getBaseType() == Image::BaseType::UINT8)
                    padded[i * 4 * dstElemSize + 3] = 0xFF;
                else if (image->getBaseType() == Image::BaseType::HALF)
                    std::memcpy(&padded[i * 4 * dstElemSize + 3 * dstElemSize],
                                &halfOne, sizeof(halfOne));
                else
                    std::memcpy(&padded[i * 4 * dstElemSize + 3 * dstElemSize],
                                &floatOne, sizeof(floatOne));
            }
            uploadData = padded.data();
        }
    }
    if (format == DXGI_FORMAT_UNKNOWN || bytesPerPixel == 0 || !uploadData)
        return false;

    ID3D11Device* device = _context->getDevice();

    const UINT w = image->getWidth();
    const UINT h = image->getHeight();

    // Always create a full mip chain. Two reasons:
    //   - Env radiance (HDR FLOAT/HALF) is sampled at roughness-derived
    //     LOD via SampleLevel - mips are required for IBL prefilter to
    //     work without banding on glossy materials.
    //   - Material UINT8 textures (e.g. wood with 4x4 UV tiling) have
    //     screen-space derivatives that drive LOD > 0 when minified.
    //     With only mip 0, trilinear sampling returns garbage at any
    //     LOD > 0 and the texture appears as a uniform average colour.
    // Sampler choice (trilinear with full LOD range vs MaxLOD=0)
    // determines how aggressively a material texture gets blurred -
    // see createSampler.
    const bool wantMips = true;
    UINT maxMips = 1;
    for (UINT s = (w > h ? w : h); s > 1; s >>= 1)
        ++maxMips;

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = w;
    td.Height = h;
    td.MipLevels = maxMips;
    td.ArraySize = 1;
    td.Format = format;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = wantMips
                 ? (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)
                 : D3D11_BIND_SHADER_RESOURCE;
    td.CPUAccessFlags = 0;
    td.MiscFlags = wantMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

    CacheEntry entry;
    if (FAILED(device->CreateTexture2D(&td, nullptr, &entry.texture)))
        return false;

    _context->getDeviceContext()->UpdateSubresource(
        entry.texture, 0, nullptr,
        uploadData, w * bytesPerPixel, 0);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = format;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MostDetailedMip = 0;
    srvd.Texture2D.MipLevels = maxMips;
    if (FAILED(device->CreateShaderResourceView(entry.texture, &srvd, &entry.srv)))
    {
        entry.texture->Release();
        return false;
    }
    if (wantMips)
        _context->getDeviceContext()->GenerateMips(entry.srv);

    entry.sampler = createSampler(device, samplingProperties);
    if (!entry.sampler)
    {
        entry.srv->Release();
        entry.texture->Release();
        return false;
    }

    _cache.emplace(image->getResourceId(), entry);
    return true;
}

bool HlslTextureHandler::unbindImage(ImagePtr image)
{
    if (!image)
        return false;
    auto it = _cache.find(image->getResourceId());
    if (it == _cache.end())
        return false;
    releaseAndNull(reinterpret_cast<IUnknown**>(&it->second.sampler));
    releaseAndNull(reinterpret_cast<IUnknown**>(&it->second.srv));
    releaseAndNull(reinterpret_cast<IUnknown**>(&it->second.texture));
    _cache.erase(it);
    return true;
}

void HlslTextureHandler::releaseRenderResources(ImagePtr image)
{
    if (image)
    {
        unbindImage(image);
        return;
    }
    for (auto& kv : _cache)
    {
        releaseAndNull(reinterpret_cast<IUnknown**>(&kv.second.sampler));
        releaseAndNull(reinterpret_cast<IUnknown**>(&kv.second.srv));
        releaseAndNull(reinterpret_cast<IUnknown**>(&kv.second.texture));
    }
    _cache.clear();
}

ID3D11ShaderResourceView* HlslTextureHandler::getBoundSrv(unsigned int resourceId) const
{
    auto it = _cache.find(resourceId);
    return it != _cache.end() ? it->second.srv : nullptr;
}

ID3D11SamplerState* HlslTextureHandler::getBoundSampler(unsigned int resourceId) const
{
    auto it = _cache.find(resourceId);
    return it != _cache.end() ? it->second.sampler : nullptr;
}

MATERIALX_NAMESPACE_END
