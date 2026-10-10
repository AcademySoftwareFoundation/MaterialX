//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslD3D12Framebuffer.h>

#include <MaterialXRender/ShaderRenderer.h>

#define NOMINMAX 1
#include <Windows.h>
#include <d3d12.h>
#include <wrl/client.h>

#include <cstring>

MATERIALX_NAMESPACE_BEGIN

namespace
{

using Microsoft::WRL::ComPtr;

D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle(uint64_t ptr)
{
    D3D12_CPU_DESCRIPTOR_HANDLE h;
    h.ptr = static_cast<SIZE_T>(ptr);
    return h;
}

struct ColorFormat
{
    DXGI_FORMAT storage;
    DXGI_FORMAT srgbView;
    DXGI_FORMAT linearView;
    unsigned int bytesPerPixel;
};

ColorFormat colorFormatFor(Image::BaseType baseType)
{
    switch (baseType)
    {
        case Image::BaseType::HALF:
            return { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT,
                     DXGI_FORMAT_R16G16B16A16_FLOAT, 8 };
        case Image::BaseType::FLOAT:
            return { DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,
                     DXGI_FORMAT_R32G32B32A32_FLOAT, 16 };
        default:
            // Typeless storage so that an sRGB-encoding view and a linear
            // view can both target the same texture.
            return { DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                     DXGI_FORMAT_R8G8B8A8_UNORM, 4 };
    }
}

const DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;

} // namespace

struct HlslD3D12Framebuffer::Impl
{
    HlslD3D12ContextPtr context;
    unsigned int width = 0;
    unsigned int height = 0;
    Image::BaseType baseType = Image::BaseType::UINT8;
    ColorFormat format = {};
    bool encodeSrgb = true;

    ComPtr<ID3D12Resource> color;
    ComPtr<ID3D12Resource> depth;
    D3D12_RESOURCE_STATES colorState = D3D12_RESOURCE_STATE_RENDER_TARGET;

    uint64_t rtvSrgb = 0;
    uint64_t rtvLinear = 0;
    uint64_t dsv = 0;

    uint64_t currentRtv() const
    {
        return (encodeSrgb && baseType == Image::BaseType::UINT8) ? rtvSrgb : rtvLinear;
    }
};

HlslD3D12Framebuffer::HlslD3D12Framebuffer(HlslD3D12ContextPtr context, unsigned int width, unsigned int height,
                                           Image::BaseType baseType) :
    _impl(new Impl())
{
    Impl& d = *_impl;
    d.context = std::move(context);
    d.width = width;
    d.height = height;
    d.baseType = baseType;
    d.format = colorFormatFor(baseType);

    if (!d.context || !d.context->getDevice())
        throw ExceptionRenderError("HlslD3D12Framebuffer: null device.");
    if (width == 0 || height == 0)
        throw ExceptionRenderError("HlslD3D12Framebuffer: zero-sized framebuffer.");

    ID3D12Device* device = d.context->getDevice();

    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC td = {};
    td.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    td.Width = width;
    td.Height = height;
    td.DepthOrArraySize = 1;
    td.MipLevels = 1;
    td.Format = d.format.storage;
    td.SampleDesc.Count = 1;
    td.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if (FAILED(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &td,
                                               D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr,
                                               IID_PPV_ARGS(d.color.GetAddressOf()))))
    {
        throw ExceptionRenderError("HlslD3D12Framebuffer: failed to create color target.");
    }

    D3D12_RENDER_TARGET_VIEW_DESC rtvd = {};
    rtvd.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    d.rtvSrgb = d.context->allocateStagingDescriptor(HlslD3D12DescriptorType::RenderTarget);
    rtvd.Format = d.format.srgbView;
    device->CreateRenderTargetView(d.color.Get(), &rtvd, cpuHandle(d.rtvSrgb));
    d.rtvLinear = d.context->allocateStagingDescriptor(HlslD3D12DescriptorType::RenderTarget);
    rtvd.Format = d.format.linearView;
    device->CreateRenderTargetView(d.color.Get(), &rtvd, cpuHandle(d.rtvLinear));

    D3D12_RESOURCE_DESC dd = td;
    dd.Format = DEPTH_FORMAT;
    dd.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    D3D12_CLEAR_VALUE depthClear = {};
    depthClear.Format = DEPTH_FORMAT;
    depthClear.DepthStencil.Depth = 1.0f;
    if (FAILED(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &dd,
                                               D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClear,
                                               IID_PPV_ARGS(d.depth.GetAddressOf()))))
    {
        throw ExceptionRenderError("HlslD3D12Framebuffer: failed to create depth target.");
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvd = {};
    dsvd.Format = DEPTH_FORMAT;
    dsvd.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    d.dsv = d.context->allocateStagingDescriptor(HlslD3D12DescriptorType::DepthStencil);
    device->CreateDepthStencilView(d.depth.Get(), &dsvd, cpuHandle(d.dsv));
}

HlslD3D12Framebuffer::~HlslD3D12Framebuffer()
{
    Impl& d = *_impl;
    if (d.context)
    {
        d.context->freeStagingDescriptor(HlslD3D12DescriptorType::RenderTarget, d.rtvSrgb);
        d.context->freeStagingDescriptor(HlslD3D12DescriptorType::RenderTarget, d.rtvLinear);
        d.context->freeStagingDescriptor(HlslD3D12DescriptorType::DepthStencil, d.dsv);
    }
}

unsigned int HlslD3D12Framebuffer::getWidth() const
{
    return _impl->width;
}

unsigned int HlslD3D12Framebuffer::getHeight() const
{
    return _impl->height;
}

Image::BaseType HlslD3D12Framebuffer::getBaseType() const
{
    return _impl->baseType;
}

void HlslD3D12Framebuffer::setEncodeSrgb(bool encode)
{
    _impl->encodeSrgb = encode;
}

bool HlslD3D12Framebuffer::getEncodeSrgb() const
{
    return _impl->encodeSrgb;
}

uint32_t HlslD3D12Framebuffer::getRenderTargetFormat() const
{
    const Impl& d = *_impl;
    return (d.encodeSrgb && d.baseType == Image::BaseType::UINT8) ? d.format.srgbView : d.format.linearView;
}

uint32_t HlslD3D12Framebuffer::getDepthFormat() const
{
    return DEPTH_FORMAT;
}

void HlslD3D12Framebuffer::bind(ID3D12GraphicsCommandList* commandList)
{
    Impl& d = *_impl;
    HlslD3D12Context::transitionResource(commandList, d.color.Get(), d.colorState,
                                         D3D12_RESOURCE_STATE_RENDER_TARGET);
    d.colorState = D3D12_RESOURCE_STATE_RENDER_TARGET;

    const D3D12_CPU_DESCRIPTOR_HANDLE rtv = cpuHandle(d.currentRtv());
    const D3D12_CPU_DESCRIPTOR_HANDLE dsv = cpuHandle(d.dsv);
    commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
    d.context->setRenderTargetFormats(getRenderTargetFormat(), DEPTH_FORMAT);

    D3D12_VIEWPORT vp = {};
    vp.Width = static_cast<float>(d.width);
    vp.Height = static_cast<float>(d.height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    commandList->RSSetViewports(1, &vp);

    D3D12_RECT scissor = { 0, 0, static_cast<LONG>(d.width), static_cast<LONG>(d.height) };
    commandList->RSSetScissorRects(1, &scissor);
}

void HlslD3D12Framebuffer::clear(ID3D12GraphicsCommandList* commandList, const Color4& color)
{
    Impl& d = *_impl;
    const float c[4] = { color[0], color[1], color[2], color[3] };
    commandList->ClearRenderTargetView(cpuHandle(d.currentRtv()), c, 0, nullptr);
    clearDepth(commandList);
}

void HlslD3D12Framebuffer::clearDepth(ID3D12GraphicsCommandList* commandList)
{
    commandList->ClearDepthStencilView(cpuHandle(_impl->dsv),
                                       D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                                       1.0f, 0, 0, nullptr);
}

ImagePtr HlslD3D12Framebuffer::readColor(ImagePtr image)
{
    Impl& d = *_impl;
    if (d.context->isRecordingFrame())
        throw ExceptionRenderError("HlslD3D12Framebuffer::readColor: called while a frame is being recorded.");

    ID3D12Device* device = d.context->getDevice();
    const D3D12_RESOURCE_DESC desc = d.color->GetDesc();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout = {};
    UINT numRows = 0;
    UINT64 rowSize = 0;
    UINT64 totalSize = 0;
    device->GetCopyableFootprints(&desc, 0, 1, 0, &layout, &numRows, &rowSize, &totalSize);

    ComPtr<ID3D12Resource> readback;
    readback.Attach(d.context->createReadbackBuffer(static_cast<std::size_t>(totalSize)));

    d.context->executeImmediate([&](ID3D12GraphicsCommandList* list)
    {
        HlslD3D12Context::transitionResource(list, d.color.Get(), d.colorState, D3D12_RESOURCE_STATE_COPY_SOURCE);

        D3D12_TEXTURE_COPY_LOCATION dst = {};
        dst.pResource = readback.Get();
        dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        dst.PlacedFootprint = layout;
        D3D12_TEXTURE_COPY_LOCATION src = {};
        src.pResource = d.color.Get();
        src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        src.SubresourceIndex = 0;
        list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

        HlslD3D12Context::transitionResource(list, d.color.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE,
                                             D3D12_RESOURCE_STATE_RENDER_TARGET);
    });
    d.colorState = D3D12_RESOURCE_STATE_RENDER_TARGET;

    const bool reuse = image && image->getResourceBuffer() &&
                       image->getWidth() == d.width && image->getHeight() == d.height &&
                       image->getChannelCount() == 4 && image->getBaseType() == d.baseType;
    if (!reuse)
    {
        image = Image::create(d.width, d.height, 4, d.baseType);
        image->createResourceBuffer();
    }

    D3D12_RANGE readRange = { 0, static_cast<SIZE_T>(totalSize) };
    void* mapped = nullptr;
    if (FAILED(readback->Map(0, &readRange, &mapped)) || !mapped)
        throw ExceptionRenderError("HlslD3D12Framebuffer::readColor: failed to map readback buffer.");

    auto* dstBytes = static_cast<uint8_t*>(image->getResourceBuffer());
    const auto* srcBytes = static_cast<const uint8_t*>(mapped) + layout.Offset;
    const std::size_t rowBytes = static_cast<std::size_t>(d.width) * d.format.bytesPerPixel;
    for (unsigned int y = 0; y < d.height; ++y)
    {
        std::memcpy(dstBytes + y * rowBytes, srcBytes + y * layout.Footprint.RowPitch, rowBytes);
    }
    D3D12_RANGE noWrite = { 0, 0 };
    readback->Unmap(0, &noWrite);
    return image;
}

ID3D12Resource* HlslD3D12Framebuffer::getColorResource() const
{
    return _impl->color.Get();
}

MATERIALX_NAMESPACE_END
