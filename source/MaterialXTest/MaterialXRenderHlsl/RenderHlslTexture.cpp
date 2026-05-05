//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
// End-to-end test for HlslTextureHandler. Builds a procedural 2x2 RGBA8
// image with four distinct colors, uploads it to D3D11, samples it
// through a fullscreen quad's pixel shader using HlslMaterial's t#/s#
// binding, and verifies that the four corners of the framebuffer carry
// the four input colors.
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXRenderHlsl/HlslContext.h>
#include <MaterialXRenderHlsl/HlslFramebuffer.h>
#include <MaterialXRenderHlsl/HlslMaterial.h>
#include <MaterialXRenderHlsl/HlslProgram.h>
#include <MaterialXRenderHlsl/HlslTextureHandler.h>

#include <MaterialXRender/Image.h>

#define NOMINMAX 1
#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

#include <cstdint>
#include <cstring>

namespace mx = MaterialX;
using Microsoft::WRL::ComPtr;

namespace
{

// VS: pass position through unchanged (vertices are already in NDC),
// emit UV from the input position so the PS can sample the texture.
const char* kVs = R"(
struct VsIn  { float3 pos : POSITION; };
struct VsOut { float4 pos : SV_Position; float2 uv : TEXCOORD0; };

VsOut VSMain(VsIn vin)
{
    VsOut vout;
    vout.pos = float4(vin.pos, 1.0);
    // NDC (-1..1, -1..1) -> UV (0..1, 0..1) with V flipped because D3D's
    // Y goes up in NDC and down in pixel coords.
    vout.uv = float2(vin.pos.x * 0.5 + 0.5, 0.5 - vin.pos.y * 0.5);
    return vout;
}
)";

const char* kPs = R"(
struct VsOut { float4 pos : SV_Position; float2 uv : TEXCOORD0; };

Texture2D    u_tex     : register(t0);
SamplerState u_samp    : register(s0);

float4 PSMain(VsOut pin) : SV_Target
{
    return u_tex.Sample(u_samp, pin.uv);
}
)";

mx::HlslContextPtr tryCreateContext()
{
    try { return mx::HlslContext::create(); }
    catch (const std::exception&) { return nullptr; }
}

} // namespace

TEST_CASE("Render: Hlsl Texture SampleAndDraw", "[renderhlsl]")
{
    mx::HlslContextPtr ctx = tryCreateContext();
    if (!ctx)
    {
        WARN("HlslContext could not be created; skipping texture test.");
        return;
    }

    // Build a 2x2 RGBA8 procedural image. Pixel layout (top-left first):
    //   (255, 0,   0,   255)  red       (0,   255, 0,   255)  green
    //   (0,   0,   255, 255)  blue      (255, 255, 0,   255)  yellow
    mx::ImagePtr image = mx::Image::create(2, 2, 4, mx::Image::BaseType::UINT8);
    image->createResourceBuffer();
    auto* px = static_cast<uint8_t*>(image->getResourceBuffer());
    const uint8_t pixels[16] = {
        255, 0,   0,   255,
        0,   255, 0,   255,
        0,   0,   255, 255,
        255, 255, 0,   255,
    };
    std::memcpy(px, pixels, sizeof(pixels));

    // Compile + wrap.
    mx::HlslProgramPtr prog = mx::HlslProgram::create();
    REQUIRE(prog->build(std::string(kVs), std::string(kPs)));
    mx::HlslMaterialPtr mat = mx::HlslMaterial::create(ctx, prog);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    REQUIRE(mat->createInputLayout(layoutDesc, 1));

    // Upload the texture and bind to slot 0. The handler-as-ImageHandler
    // subclass takes the bind through bindImage(image, samplingProps),
    // and exposes the underlying SRV / sampler via accessors.
    mx::HlslTextureHandlerPtr texHandler = mx::HlslTextureHandler::create(ctx, nullptr);
    mx::ImageSamplingProperties sp;
    REQUIRE(texHandler->bindImage(image, sp));
    ID3D11ShaderResourceView* srv     = texHandler->getBoundSrv(image->getResourceId());
    ID3D11SamplerState*       sampler = texHandler->getBoundSampler(image->getResourceId());
    REQUIRE(srv     != nullptr);
    REQUIRE(sampler != nullptr);
    mat->setTexture(0, srv);
    mat->setSampler(0, sampler);

    // Fullscreen triangle covering all of NDC.
    ID3D11Device* device = ctx->getDevice();
    ID3D11DeviceContext* dc = ctx->getDeviceContext();
    const float verts[] = {
        -1.0f, -1.0f, 0.0f,
         3.0f, -1.0f, 0.0f,
        -1.0f,  3.0f, 0.0f,
    };
    D3D11_BUFFER_DESC vbd = {};
    vbd.ByteWidth = sizeof(verts);
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbInit = { verts, 0, 0 };
    ComPtr<ID3D11Buffer> vbuf;
    REQUIRE(SUCCEEDED(device->CreateBuffer(&vbd, &vbInit, vbuf.GetAddressOf())));

    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    rd.DepthClipEnable = TRUE;
    ComPtr<ID3D11RasterizerState> rs;
    REQUIRE(SUCCEEDED(device->CreateRasterizerState(&rd, rs.GetAddressOf())));
    dc->RSSetState(rs.Get());

    // 64x64 framebuffer is large enough that the four corners sit well
    // inside the four texel cells (with linear filtering on a 2x2 image,
    // pixels at the very edges are unfiltered samples of the corner texels).
    constexpr unsigned int W = 64;
    constexpr unsigned int H = 64;
    mx::HlslFramebufferPtr fb = mx::HlslFramebuffer::create(ctx, W, H);
    fb->bind();
    fb->clear(mx::Color4(0.0f, 0.0f, 0.0f, 1.0f));

    UINT stride = 3 * sizeof(float);
    UINT offset = 0;
    ID3D11Buffer* vbufRaw = vbuf.Get();
    dc->IASetVertexBuffers(0, 1, &vbufRaw, &stride, &offset);
    dc->IASetInputLayout(mat->getInputLayout());
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mat->bind();
    dc->Draw(3, 0);

    fb->unbind();

    mx::ImagePtr out = fb->readColor();
    REQUIRE(out);

    // Sample each of the four corners and verify the right input texel
    // landed there. With CLAMP addressing and linear filtering, the
    // pixel at (0, 0) sees uv ~= (0.5/64, 0.5/64) which falls into the
    // top-left texel (red); analogously for the other three corners.
    auto* outPx = static_cast<const uint8_t*>(out->getResourceBuffer());
    auto sample = [&](unsigned int x, unsigned int y) -> std::array<unsigned int, 4>
    {
        const std::size_t i = (y * W + x) * 4;
        return { outPx[i], outPx[i + 1], outPx[i + 2], outPx[i + 3] };
    };
    auto approxEq = [](unsigned int a, unsigned int b) {
        return (a > b ? a - b : b - a) <= 4;
    };

    auto tl = sample(0,     0);
    auto tr = sample(W - 1, 0);
    auto bl = sample(0,     H - 1);
    auto br = sample(W - 1, H - 1);

    REQUIRE(approxEq(tl[0], 255)); REQUIRE(approxEq(tl[1], 0));   REQUIRE(approxEq(tl[2], 0));   // red
    REQUIRE(approxEq(tr[0], 0));   REQUIRE(approxEq(tr[1], 255)); REQUIRE(approxEq(tr[2], 0));   // green
    REQUIRE(approxEq(bl[0], 0));   REQUIRE(approxEq(bl[1], 0));   REQUIRE(approxEq(bl[2], 255)); // blue
    REQUIRE(approxEq(br[0], 255)); REQUIRE(approxEq(br[1], 255)); REQUIRE(approxEq(br[2], 0));   // yellow
}
