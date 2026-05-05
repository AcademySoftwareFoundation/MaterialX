//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
// End-to-end smoke test for the HLSL render pipeline. Compiles a trivial
// shader pair via HlslProgram, instantiates VS/PS objects on a D3D11
// device, draws a fullscreen triangle into an HlslFramebuffer, and reads
// the framebuffer back to confirm the triangle actually executed on the
// GPU. This is the first test that exercises the GPU end of the pipeline,
// not just compile / reflect.
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXRenderHlsl/HlslContext.h>
#include <MaterialXRenderHlsl/HlslFramebuffer.h>
#include <MaterialXRenderHlsl/HlslProgram.h>

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

namespace mx = MaterialX;
using Microsoft::WRL::ComPtr;

namespace
{

const char* kSolidVs = R"(
struct VsIn  { float3 pos : POSITION; };
struct VsOut { float4 pos : SV_Position; };

VsOut VSMain(VsIn vin)
{
    VsOut vout;
    vout.pos = float4(vin.pos, 1.0);
    return vout;
}
)";

// Pixel shader emits a fixed RGB so the readback is deterministic.
const char* kSolidPs = R"(
struct VsOut { float4 pos : SV_Position; };

float4 PSMain(VsOut pin) : SV_Target
{
    return float4(0.2, 0.4, 0.8, 1.0);
}
)";

mx::HlslContextPtr tryCreateContext()
{
    try { return mx::HlslContext::create(); }
    catch (const std::exception&) { return nullptr; }
}

} // namespace

TEST_CASE("Render: Hlsl Draw Triangle", "[renderhlsl]")
{
    mx::HlslContextPtr ctx = tryCreateContext();
    if (!ctx)
    {
        WARN("HlslContext could not be created; skipping draw test.");
        return;
    }

    // Compile the trivial shader pair.
    mx::HlslProgramPtr prog = mx::HlslProgram::create();
    std::vector<uint8_t> vsBc, psBc;
    std::string log;
    REQUIRE(prog->buildStage(kSolidVs, "VSMain", "vs_5_0", vsBc, log));
    REQUIRE(prog->buildStage(kSolidPs, "PSMain", "ps_5_0", psBc, log));

    ID3D11Device* device = ctx->getDevice();
    ID3D11DeviceContext* dc = ctx->getDeviceContext();

    ComPtr<ID3D11VertexShader> vs;
    REQUIRE(SUCCEEDED(device->CreateVertexShader(vsBc.data(), vsBc.size(), nullptr, vs.GetAddressOf())));
    ComPtr<ID3D11PixelShader> ps;
    REQUIRE(SUCCEEDED(device->CreatePixelShader(psBc.data(), psBc.size(), nullptr, ps.GetAddressOf())));

    // Input layout: a single POSITION (float3).
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ComPtr<ID3D11InputLayout> layout;
    REQUIRE(SUCCEEDED(device->CreateInputLayout(layoutDesc, 1, vsBc.data(), vsBc.size(), layout.GetAddressOf())));

    // Triangle covering the entire NDC clip space.
    float verts[] = {
        -3.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  3.0f, 0.0f,
    };
    D3D11_BUFFER_DESC vbd = {};
    vbd.ByteWidth = sizeof(verts);
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbInit = {};
    vbInit.pSysMem = verts;
    ComPtr<ID3D11Buffer> vbuf;
    REQUIRE(SUCCEEDED(device->CreateBuffer(&vbd, &vbInit, vbuf.GetAddressOf())));

    constexpr unsigned int W = 32;
    constexpr unsigned int H = 32;
    mx::HlslFramebufferPtr fb = mx::HlslFramebuffer::create(ctx, W, H);
    fb->bind();
    fb->clear(mx::Color4(0.0f, 0.0f, 0.0f, 1.0f));

    // Disable back-face culling - the default D3D11 rasterizer uses
    // FrontCounterClockwise=FALSE + CullMode=BACK, which would cull our
    // CCW-wound fullscreen triangle.
    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    rd.FrontCounterClockwise = FALSE;
    rd.DepthClipEnable = TRUE;
    ComPtr<ID3D11RasterizerState> rs;
    REQUIRE(SUCCEEDED(device->CreateRasterizerState(&rd, rs.GetAddressOf())));
    dc->RSSetState(rs.Get());

    UINT stride = 3 * sizeof(float);
    UINT offset = 0;
    ID3D11Buffer* vbufRaw = vbuf.Get();
    dc->IASetVertexBuffers(0, 1, &vbufRaw, &stride, &offset);
    dc->IASetInputLayout(layout.Get());
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dc->VSSetShader(vs.Get(), nullptr, 0);
    dc->PSSetShader(ps.Get(), nullptr, 0);
    dc->Draw(3, 0);

    fb->unbind();

    mx::ImagePtr image = fb->readColor();
    REQUIRE(image);
    REQUIRE(image->getWidth() == W);
    REQUIRE(image->getHeight() == H);

    // Center pixel: triangle covers the whole framebuffer, so the centre
    // must be the shader's solid color (within ULP-1 quantisation).
    auto* pixels = static_cast<const uint8_t*>(image->getResourceBuffer());
    REQUIRE(pixels);
    const std::size_t cx = W / 2;
    const std::size_t cy = H / 2;
    const std::size_t centre = (cy * W + cx) * 4;
    auto approxEq = [](unsigned int a, unsigned int b) {
        return (a > b ? a - b : b - a) <= 1;
    };
    REQUIRE(approxEq(pixels[centre + 0], 51));   // 0.2 * 255 = 51
    REQUIRE(approxEq(pixels[centre + 1], 102));  // 0.4 * 255 = 102
    REQUIRE(approxEq(pixels[centre + 2], 204));  // 0.8 * 255 = 204
    REQUIRE(pixels[centre + 3] == 255);
}
