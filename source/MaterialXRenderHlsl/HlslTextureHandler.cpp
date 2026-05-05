//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslTextureHandler.h>

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
            return D3D11_TEXTURE_ADDRESS_CLAMP;
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
    const DXGI_FORMAT format = toDxgiFormat(image->getChannelCount(),
                                            image->getBaseType(), bytesPerPixel);
    if (format == DXGI_FORMAT_UNKNOWN || bytesPerPixel == 0 || !image->getResourceBuffer())
        return false;

    ID3D11Device* device = _context->getDevice();

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = image->getWidth();
    td.Height = image->getHeight();
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = format;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_IMMUTABLE;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    td.CPUAccessFlags = 0;
    td.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = image->getResourceBuffer();
    sd.SysMemPitch = static_cast<UINT>(image->getWidth()) * bytesPerPixel;
    sd.SysMemSlicePitch = 0;

    CacheEntry entry;
    if (FAILED(device->CreateTexture2D(&td, &sd, &entry.texture)))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = format;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MostDetailedMip = 0;
    srvd.Texture2D.MipLevels = 1;
    if (FAILED(device->CreateShaderResourceView(entry.texture, &srvd, &entry.srv)))
    {
        entry.texture->Release();
        return false;
    }

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
