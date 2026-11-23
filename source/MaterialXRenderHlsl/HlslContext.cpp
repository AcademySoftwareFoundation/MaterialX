//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslContext.h>

#include <MaterialXRender/ShaderRenderer.h>

#include <Windows.h>
#include <d3d11.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

HRESULT createDevice(D3D_DRIVER_TYPE driverType,
                     ID3D11Device** device,
                     ID3D11DeviceContext** context)
{
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    // Debug layer requires the D3D11 SDK Layers; if not installed the
    // create call will fail and we'll retry without it.
    UINT debugFlags = flags | D3D11_CREATE_DEVICE_DEBUG;
#else
    UINT debugFlags = flags;
#endif

    static const D3D_FEATURE_LEVEL kLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    D3D_FEATURE_LEVEL got = D3D_FEATURE_LEVEL_10_0;
    HRESULT hr = ::D3D11CreateDevice(
        nullptr,
        driverType,
        nullptr,
        debugFlags,
        kLevels,
        ARRAYSIZE(kLevels),
        D3D11_SDK_VERSION,
        device,
        &got,
        context);
    if (FAILED(hr) && (debugFlags & D3D11_CREATE_DEVICE_DEBUG))
    {
        // Retry without the debug layer.
        hr = ::D3D11CreateDevice(
            nullptr,
            driverType,
            nullptr,
            flags,
            kLevels,
            ARRAYSIZE(kLevels),
            D3D11_SDK_VERSION,
            device,
            &got,
            context);
    }
    return hr;
}

} // namespace

HlslContext::HlslContext()
{
    HRESULT hr = createDevice(D3D_DRIVER_TYPE_HARDWARE, &_device, &_deviceContext);
    if (FAILED(hr))
    {
        _isHardware = false;
        hr = createDevice(D3D_DRIVER_TYPE_WARP, &_device, &_deviceContext);
    }
    if (FAILED(hr) || !_device || !_deviceContext)
    {
        throw ExceptionRenderError("HlslContext: D3D11CreateDevice failed (hardware and WARP).");
    }
}

HlslContext::~HlslContext()
{
    if (_deviceContext)
    {
        _deviceContext->ClearState();
        _deviceContext->Release();
        _deviceContext = nullptr;
    }
    if (_device)
    {
        _device->Release();
        _device = nullptr;
    }
}

MATERIALX_NAMESPACE_END
