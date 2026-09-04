//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLCONTEXT_H
#define MATERIALX_HLSLCONTEXT_H

/// @file
/// D3D11 device + immediate context wrapper.

#include <MaterialXRenderHlsl/Export.h>

#include <MaterialXCore/Library.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

MATERIALX_NAMESPACE_BEGIN

class HlslContext;
using HlslContextPtr = shared_ptr<class HlslContext>;

/// @class HlslContext
/// Owns a single D3D11 device and immediate context for headless rendering.
///
/// The context is intentionally minimal: no swap chain, no window. It is
/// created with BGRA support and the default feature level (10.0+), and
/// is suitable for off-screen render-to-texture work where the result is
/// read back via Map / staging textures.
///
/// Construction may throw if D3D11CreateDevice fails (driver missing,
/// reference rasterizer requested but unavailable, etc.). Callers that
/// want a non-throwing path should catch ExceptionRenderError.
class MX_RENDERHLSL_API HlslContext
{
  public:
    /// Construct a hardware D3D11 device. If hardware creation fails the
    /// constructor falls back to D3D_DRIVER_TYPE_WARP (the high-quality
    /// software rasterizer included with Windows). Throws if both fail.
    HlslContext();
    ~HlslContext();

    HlslContext(const HlslContext&) = delete;
    HlslContext& operator=(const HlslContext&) = delete;

    static HlslContextPtr create()
    {
        return std::make_shared<HlslContext>();
    }

    /// True if the device wraps a hardware adapter, false if WARP.
    bool isHardware() const { return _isHardware; }

    /// Direct access to the wrapped device. Borrowed reference; the
    /// HlslContext retains ownership.
    ID3D11Device* getDevice() const { return _device; }

    /// Direct access to the wrapped immediate context. Borrowed.
    ID3D11DeviceContext* getDeviceContext() const { return _deviceContext; }

  private:
    ID3D11Device* _device = nullptr;
    ID3D11DeviceContext* _deviceContext = nullptr;
    bool _isHardware = true;
};

MATERIALX_NAMESPACE_END

#endif
