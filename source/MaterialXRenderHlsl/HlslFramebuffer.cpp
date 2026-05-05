//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslFramebuffer.h>

#include <MaterialXRender/ShaderRenderer.h>

#include <Windows.h>
#include <d3d11.h>

#include <cstring>

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

} // namespace

HlslFramebuffer::HlslFramebuffer(HlslContextPtr context, unsigned int width, unsigned int height) :
    _context(std::move(context)),
    _width(width),
    _height(height)
{
    if (!_context || !_context->getDevice())
        throw ExceptionRenderError("HlslFramebuffer: null device.");
    if (_width == 0 || _height == 0)
        throw ExceptionRenderError("HlslFramebuffer: zero-sized framebuffer.");

    ID3D11Device* device = _context->getDevice();

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = _width;
    td.Height = _height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.SampleDesc.Count = 1;
    td.SampleDesc.Quality = 0;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.CPUAccessFlags = 0;
    td.MiscFlags = 0;

    // Color target: render-target + shader-resource so a downstream pass
    // can sample the result.
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    if (FAILED(device->CreateTexture2D(&td, nullptr, &_colorTexture)))
        throw ExceptionRenderError("HlslFramebuffer: failed to create color texture.");

    // Render-target view over the color texture.
    D3D11_RENDER_TARGET_VIEW_DESC rtvd = {};
    rtvd.Format = td.Format;
    rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    if (FAILED(device->CreateRenderTargetView(_colorTexture, &rtvd, &_colorRtv)))
        throw ExceptionRenderError("HlslFramebuffer: failed to create RTV.");

    // Shader-resource view so a sampled pass can read the result.
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = td.Format;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MostDetailedMip = 0;
    srvd.Texture2D.MipLevels = 1;
    if (FAILED(device->CreateShaderResourceView(_colorTexture, &srvd, &_colorSrv)))
        throw ExceptionRenderError("HlslFramebuffer: failed to create SRV.");

    // Depth-stencil target.
    D3D11_TEXTURE2D_DESC dd = td;
    dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    if (FAILED(device->CreateTexture2D(&dd, nullptr, &_depthTexture)))
        throw ExceptionRenderError("HlslFramebuffer: failed to create depth texture.");

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {};
    dsvd.Format = dd.Format;
    dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    if (FAILED(device->CreateDepthStencilView(_depthTexture, &dsvd, &_depthDsv)))
        throw ExceptionRenderError("HlslFramebuffer: failed to create DSV.");

    // Staging texture used by readColor() to copy GPU pixels back to CPU.
    D3D11_TEXTURE2D_DESC sd = td;
    sd.Usage = D3D11_USAGE_STAGING;
    sd.BindFlags = 0;
    sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(device->CreateTexture2D(&sd, nullptr, &_stagingTexture)))
        throw ExceptionRenderError("HlslFramebuffer: failed to create staging texture.");
}

HlslFramebuffer::~HlslFramebuffer()
{
    releaseAndNull(reinterpret_cast<IUnknown**>(&_colorSrv));
    releaseAndNull(reinterpret_cast<IUnknown**>(&_depthDsv));
    releaseAndNull(reinterpret_cast<IUnknown**>(&_colorRtv));
    releaseAndNull(reinterpret_cast<IUnknown**>(&_stagingTexture));
    releaseAndNull(reinterpret_cast<IUnknown**>(&_depthTexture));
    releaseAndNull(reinterpret_cast<IUnknown**>(&_colorTexture));
}

void HlslFramebuffer::bind()
{
    ID3D11DeviceContext* ctx = _context->getDeviceContext();
    ctx->OMSetRenderTargets(1, &_colorRtv, _depthDsv);

    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width  = static_cast<float>(_width);
    vp.Height = static_cast<float>(_height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);
}

void HlslFramebuffer::unbind()
{
    ID3D11DeviceContext* ctx = _context->getDeviceContext();
    ID3D11RenderTargetView* nullRtv = nullptr;
    ctx->OMSetRenderTargets(1, &nullRtv, nullptr);
}

void HlslFramebuffer::clear(const Color4& color)
{
    ID3D11DeviceContext* ctx = _context->getDeviceContext();
    const float c[4] = { color[0], color[1], color[2], color[3] };
    ctx->ClearRenderTargetView(_colorRtv, c);
    ctx->ClearDepthStencilView(_depthDsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

ImagePtr HlslFramebuffer::readColor()
{
    ID3D11DeviceContext* ctx = _context->getDeviceContext();
    ctx->CopyResource(_stagingTexture, _colorTexture);

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (FAILED(ctx->Map(_stagingTexture, 0, D3D11_MAP_READ, 0, &mapped)) || !mapped.pData)
        throw ExceptionRenderError("HlslFramebuffer::readColor: failed to map staging texture.");

    ImagePtr image = Image::create(_width, _height, 4, Image::BaseType::UINT8);
    image->createResourceBuffer();

    auto* dst = static_cast<uint8_t*>(image->getResourceBuffer());
    const auto* src = static_cast<const uint8_t*>(mapped.pData);
    const size_t rowBytes = static_cast<size_t>(_width) * 4;
    for (unsigned int y = 0; y < _height; ++y)
    {
        std::memcpy(dst + y * rowBytes, src + y * mapped.RowPitch, rowBytes);
    }
    ctx->Unmap(_stagingTexture, 0);
    return image;
}

MATERIALX_NAMESPACE_END
