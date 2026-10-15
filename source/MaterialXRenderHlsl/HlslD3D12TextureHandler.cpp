//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslD3D12TextureHandler.h>
#include <MaterialXRenderHlsl/HlslRenderUtil.h>

#include <MaterialXRender/Types.h>

#define NOMINMAX 1
#include <Windows.h>
#include <d3d12.h>
#include <wrl/client.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <unordered_map>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

namespace
{

using Microsoft::WRL::ComPtr;

static_assert(sizeof(Half) == sizeof(uint16_t), "Half must be 16 bits for texel conversion.");

D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle(uint64_t ptr)
{
    D3D12_CPU_DESCRIPTOR_HANDLE h;
    h.ptr = static_cast<SIZE_T>(ptr);
    return h;
}

unsigned int elementSize(Image::BaseType baseType)
{
    switch (baseType)
    {
        case Image::BaseType::UINT8: return 1;
        case Image::BaseType::HALF:  return 2;
        case Image::BaseType::FLOAT: return 4;
        default:                     return 0;
    }
}

// Map a channel count and base type to the DXGI format used for upload.
// Three-channel images are padded to four channels before upload, since
// D3D12 has no 24-bit RGB format and no filtering support for 96-bit RGB.
DXGI_FORMAT toDxgiFormat(unsigned int channels, Image::BaseType baseType)
{
    switch (baseType)
    {
        case Image::BaseType::UINT8:
            switch (channels)
            {
                case 1: return DXGI_FORMAT_R8_UNORM;
                case 2: return DXGI_FORMAT_R8G8_UNORM;
                case 4: return DXGI_FORMAT_R8G8B8A8_UNORM;
                default: break;
            }
            break;
        case Image::BaseType::HALF:
            switch (channels)
            {
                case 1: return DXGI_FORMAT_R16_FLOAT;
                case 2: return DXGI_FORMAT_R16G16_FLOAT;
                case 4: return DXGI_FORMAT_R16G16B16A16_FLOAT;
                default: break;
            }
            break;
        case Image::BaseType::FLOAT:
            switch (channels)
            {
                case 1: return DXGI_FORMAT_R32_FLOAT;
                case 2: return DXGI_FORMAT_R32G32_FLOAT;
                case 4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
                default: break;
            }
            break;
        default:
            break;
    }
    return DXGI_FORMAT_UNKNOWN;
}

float loadElement(const uint8_t* p, Image::BaseType baseType)
{
    switch (baseType)
    {
        case Image::BaseType::UINT8:
            return static_cast<float>(*p);
        case Image::BaseType::HALF:
        {
            Half h(0.0f);
            std::memcpy(&h, p, sizeof(Half));
            return static_cast<float>(h);
        }
        default:
        {
            float f;
            std::memcpy(&f, p, sizeof(float));
            return f;
        }
    }
}

void storeElement(uint8_t* p, Image::BaseType baseType, float value)
{
    switch (baseType)
    {
        case Image::BaseType::UINT8:
            *p = static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, std::floor(value + 0.5f))));
            break;
        case Image::BaseType::HALF:
        {
            const Half h(value);
            std::memcpy(p, &h, sizeof(Half));
            break;
        }
        default:
            std::memcpy(p, &value, sizeof(float));
            break;
    }
}

// Pad a three-channel image to four channels with an opaque alpha.
std::vector<uint8_t> padToFourChannels(const uint8_t* src, std::size_t texelCount, Image::BaseType baseType)
{
    const unsigned int es = elementSize(baseType);
    std::vector<uint8_t> padded(texelCount * 4 * es);
    for (std::size_t i = 0; i < texelCount; ++i)
    {
        std::memcpy(&padded[i * 4 * es], src + i * 3 * es, 3 * es);
        storeElement(&padded[i * 4 * es + 3 * es], baseType,
                     baseType == Image::BaseType::UINT8 ? 255.0f : 1.0f);
    }
    return padded;
}

// Build the next mip level with a 2x2 box filter. Odd source dimensions
// clamp the second tap to the last row or column.
std::vector<uint8_t> downsample(const uint8_t* src, unsigned int width, unsigned int height,
                                unsigned int channels, Image::BaseType baseType,
                                unsigned int& dstWidth, unsigned int& dstHeight)
{
    dstWidth = std::max(1u, width / 2);
    dstHeight = std::max(1u, height / 2);
    const unsigned int es = elementSize(baseType);
    std::vector<uint8_t> dst(static_cast<std::size_t>(dstWidth) * dstHeight * channels * es);
    auto texel = [&](unsigned int x, unsigned int y, unsigned int c)
    {
        return src + ((static_cast<std::size_t>(y) * width + x) * channels + c) * es;
    };
    for (unsigned int y = 0; y < dstHeight; ++y)
    {
        const unsigned int y0 = std::min(2 * y, height - 1);
        const unsigned int y1 = std::min(2 * y + 1, height - 1);
        for (unsigned int x = 0; x < dstWidth; ++x)
        {
            const unsigned int x0 = std::min(2 * x, width - 1);
            const unsigned int x1 = std::min(2 * x + 1, width - 1);
            for (unsigned int c = 0; c < channels; ++c)
            {
                const float sum = loadElement(texel(x0, y0, c), baseType) + loadElement(texel(x1, y0, c), baseType) +
                                  loadElement(texel(x0, y1, c), baseType) + loadElement(texel(x1, y1, c), baseType);
                storeElement(&dst[((static_cast<std::size_t>(y) * dstWidth + x) * channels + c) * es],
                             baseType, sum * 0.25f);
            }
        }
    }
    return dst;
}

D3D12_TEXTURE_ADDRESS_MODE toAddressMode(ImageSamplingProperties::AddressMode mode)
{
    // Unspecified defaults to repeat, matching HlslTextureHandler and
    // GLTextureHandler, so UV-tiled materials repeat out of the box.
    switch (mode)
    {
        case ImageSamplingProperties::AddressMode::CONSTANT: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case ImageSamplingProperties::AddressMode::CLAMP:    return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case ImageSamplingProperties::AddressMode::MIRROR:   return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case ImageSamplingProperties::AddressMode::PERIODIC:
        case ImageSamplingProperties::AddressMode::UNSPECIFIED:
        default:                                             return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    }
}

D3D12_FILTER toFilter(ImageSamplingProperties::FilterType filterType)
{
    switch (filterType)
    {
        case ImageSamplingProperties::FilterType::CLOSEST:
            return D3D12_FILTER_MIN_MAG_MIP_POINT;
        case ImageSamplingProperties::FilterType::CUBIC:
            // No fixed-function cubic filter; anisotropic is the closest
            // quality level available.
            return D3D12_FILTER_ANISOTROPIC;
        default:
            return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    }
}

void writeSampler(ID3D12Device* device, const ImageSamplingProperties& sp, uint64_t handle)
{
    D3D12_SAMPLER_DESC sd = {};
    sd.Filter = toFilter(sp.filterType);
    sd.AddressU = toAddressMode(sp.uaddressMode);
    sd.AddressV = toAddressMode(sp.vaddressMode);
    sd.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sd.MaxAnisotropy = (sd.Filter == D3D12_FILTER_ANISOTROPIC) ? 16u : 1u;
    sd.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sd.MinLOD = 0.0f;
    // Without mipmaps, sampling is limited to the base level.
    sd.MaxLOD = sp.enableMipmaps ? D3D12_FLOAT32_MAX : 0.0f;
    for (int i = 0; i < 4; ++i)
        sd.BorderColor[i] = sp.defaultColor[i];
    device->CreateSampler(&sd, cpuHandle(handle));
}

struct CacheEntry
{
    ComPtr<ID3D12Resource> texture;
    uint64_t srv = 0;
    uint64_t sampler = 0;
};

// Texel data and size of one mip level.
struct MipLevel
{
    const uint8_t* data = nullptr;
    unsigned int width = 0;
    unsigned int height = 0;
};

} // namespace

struct HlslD3D12TextureHandler::Impl
{
    HlslD3D12ContextPtr context;
    std::unordered_map<unsigned int, CacheEntry> cache;

    void release(CacheEntry& entry)
    {
        if (entry.srv)
            context->freeStagingDescriptor(HlslD3D12DescriptorType::Resource, entry.srv);
        if (entry.sampler)
            context->freeStagingDescriptor(HlslD3D12DescriptorType::Sampler, entry.sampler);
        entry = CacheEntry();
    }

    bool upload(ImagePtr image, const std::vector<MipLevel>& levels, unsigned int channels,
                Image::BaseType baseType, const ImageSamplingProperties& samplingProperties);
};

bool HlslD3D12TextureHandler::Impl::upload(ImagePtr image, const std::vector<MipLevel>& levels, unsigned int channels,
                                           Image::BaseType baseType, const ImageSamplingProperties& samplingProperties)
{
    const DXGI_FORMAT format = toDxgiFormat(channels, baseType);
    if (format == DXGI_FORMAT_UNKNOWN || levels.empty())
        return false;

    ID3D12Device* device = context->getDevice();
    const UINT16 mipCount = static_cast<UINT16>(levels.size());

    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC td = {};
    td.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    td.Width = levels[0].width;
    td.Height = levels[0].height;
    td.DepthOrArraySize = 1;
    td.MipLevels = mipCount;
    td.Format = format;
    td.SampleDesc.Count = 1;

    CacheEntry entry;
    if (FAILED(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &td,
                                               D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                               IID_PPV_ARGS(entry.texture.GetAddressOf()))))
    {
        return false;
    }

    std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprints(mipCount);
    std::vector<UINT> numRows(mipCount);
    std::vector<UINT64> rowSizes(mipCount);
    UINT64 totalSize = 0;
    device->GetCopyableFootprints(&td, 0, mipCount, 0, footprints.data(), numRows.data(),
                                  rowSizes.data(), &totalSize);

    ComPtr<ID3D12Resource> staging;
    staging.Attach(context->createUploadBuffer(static_cast<std::size_t>(totalSize)));
    D3D12_RANGE noRead = { 0, 0 };
    void* mapped = nullptr;
    if (FAILED(staging->Map(0, &noRead, &mapped)) || !mapped)
        return false;
    const std::size_t texelBytes = static_cast<std::size_t>(channels) * elementSize(baseType);
    for (UINT16 mip = 0; mip < mipCount; ++mip)
    {
        const std::size_t rowBytes = levels[mip].width * texelBytes;
        uint8_t* dst = static_cast<uint8_t*>(mapped) + footprints[mip].Offset;
        for (UINT row = 0; row < numRows[mip]; ++row)
            std::memcpy(dst + row * footprints[mip].Footprint.RowPitch, levels[mip].data + row * rowBytes, rowBytes);
    }
    staging->Unmap(0, nullptr);

    context->executeImmediate([&](ID3D12GraphicsCommandList* list)
    {
        for (UINT16 mip = 0; mip < mipCount; ++mip)
        {
            D3D12_TEXTURE_COPY_LOCATION dst = {};
            dst.pResource = entry.texture.Get();
            dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dst.SubresourceIndex = mip;
            D3D12_TEXTURE_COPY_LOCATION src = {};
            src.pResource = staging.Get();
            src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            src.PlacedFootprint = footprints[mip];
            list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
        }
        HlslD3D12Context::transitionResource(list, entry.texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                             D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    });

    entry.srv = context->allocateStagingDescriptor(HlslD3D12DescriptorType::Resource);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = format;
    srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvd.Texture2D.MipLevels = mipCount;
    device->CreateShaderResourceView(entry.texture.Get(), &srvd, cpuHandle(entry.srv));

    entry.sampler = context->allocateStagingDescriptor(HlslD3D12DescriptorType::Sampler);
    writeSampler(device, samplingProperties, entry.sampler);

    auto existing = cache.find(image->getResourceId());
    if (existing != cache.end())
        release(existing->second);
    cache[image->getResourceId()] = entry;
    return true;
}

HlslD3D12TextureHandler::HlslD3D12TextureHandler(HlslD3D12ContextPtr context, ImageLoaderPtr imageLoader) :
    ImageHandler(std::move(imageLoader)),
    _impl(new Impl())
{
    _impl->context = std::move(context);
}

HlslD3D12TextureHandler::~HlslD3D12TextureHandler()
{
    releaseRenderResources();
}

bool HlslD3D12TextureHandler::bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties)
{
    Impl& d = *_impl;
    if (!image || !d.context || !d.context->getDevice())
        return false;

    assignHlslImageResourceId(image);
    ID3D12Device* device = d.context->getDevice();

    // Resident image: only refresh the sampler, since the sampling
    // properties may differ between uniforms sharing the image. Images
    // uploaded with bindImageWithMips need no CPU data here.
    auto it = d.cache.find(image->getResourceId());
    if (it != d.cache.end() && it->second.texture)
    {
        writeSampler(device, samplingProperties, it->second.sampler);
        return true;
    }
    if (!image->getResourceBuffer())
        return false;

    const Image::BaseType baseType = image->getBaseType();
    unsigned int channels = image->getChannelCount();
    const unsigned int es = elementSize(baseType);
    if (es == 0)
        return false;

    const unsigned int width = image->getWidth();
    const unsigned int height = image->getHeight();
    const std::size_t texelCount = static_cast<std::size_t>(width) * height;

    std::vector<uint8_t> padded;
    const uint8_t* level0 = static_cast<const uint8_t*>(image->getResourceBuffer());
    if (channels == 3)
    {
        padded = padToFourChannels(level0, texelCount, baseType);
        level0 = padded.data();
        channels = 4;
    }
    // Full mip chain, as in HlslTextureHandler: environment maps are
    // sampled at roughness-derived levels, and minified material textures
    // need filtered levels.
    unsigned int mipCount = 1;
    for (unsigned int s = std::max(width, height); s > 1; s >>= 1)
        ++mipCount;

    std::vector<std::vector<uint8_t>> generated(mipCount);
    std::vector<MipLevel> levels(mipCount);
    levels[0] = { level0, width, height };
    for (unsigned int mip = 1; mip < mipCount; ++mip)
    {
        unsigned int w = 0;
        unsigned int h = 0;
        generated[mip] = downsample(levels[mip - 1].data, levels[mip - 1].width, levels[mip - 1].height,
                                    channels, baseType, w, h);
        levels[mip] = { generated[mip].data(), w, h };
    }
    return d.upload(image, levels, channels, baseType, samplingProperties);
}

bool HlslD3D12TextureHandler::bindImageWithMips(ImagePtr image, const std::vector<ImagePtr>& mipLevels,
                                                const ImageSamplingProperties& samplingProperties)
{
    Impl& d = *_impl;
    if (!image || mipLevels.empty() || !d.context)
        return false;
    assignHlslImageResourceId(image);

    const Image::BaseType baseType = mipLevels[0]->getBaseType();
    const unsigned int channels = mipLevels[0]->getChannelCount();
    if (elementSize(baseType) == 0)
        return false;

    std::vector<std::vector<uint8_t>> padded(mipLevels.size());
    std::vector<MipLevel> levels;
    for (std::size_t i = 0; i < mipLevels.size(); ++i)
    {
        ImagePtr level = mipLevels[i];
        if (!level || !level->getResourceBuffer() || level->getBaseType() != baseType ||
            level->getChannelCount() != channels)
        {
            return false;
        }
        if (i > 0 && (level->getWidth() != std::max(1u, levels.back().width / 2) ||
                      level->getHeight() != std::max(1u, levels.back().height / 2)))
        {
            return false;
        }
        const uint8_t* data = static_cast<const uint8_t*>(level->getResourceBuffer());
        if (channels == 3)
        {
            padded[i] = padToFourChannels(data, static_cast<std::size_t>(level->getWidth()) * level->getHeight(), baseType);
            data = padded[i].data();
        }
        levels.push_back({ data, level->getWidth(), level->getHeight() });
    }
    return d.upload(image, levels, channels == 3 ? 4 : channels, baseType, samplingProperties);
}

bool HlslD3D12TextureHandler::unbindImage(ImagePtr image)
{
    if (!image)
        return false;
    auto it = _impl->cache.find(image->getResourceId());
    if (it == _impl->cache.end())
        return false;
    _impl->release(it->second);
    _impl->cache.erase(it);
    return true;
}

void HlslD3D12TextureHandler::releaseRenderResources(ImagePtr image)
{
    if (image)
    {
        unbindImage(image);
        return;
    }
    for (auto& kv : _impl->cache)
        _impl->release(kv.second);
    _impl->cache.clear();
}

uint64_t HlslD3D12TextureHandler::getBoundSrv(unsigned int resourceId) const
{
    auto it = _impl->cache.find(resourceId);
    return it != _impl->cache.end() ? it->second.srv : 0;
}

uint64_t HlslD3D12TextureHandler::getBoundSampler(unsigned int resourceId) const
{
    auto it = _impl->cache.find(resourceId);
    return it != _impl->cache.end() ? it->second.sampler : 0;
}

ID3D12Resource* HlslD3D12TextureHandler::getBoundTexture(unsigned int resourceId) const
{
    auto it = _impl->cache.find(resourceId);
    return it != _impl->cache.end() ? it->second.texture.Get() : nullptr;
}

MATERIALX_NAMESPACE_END
