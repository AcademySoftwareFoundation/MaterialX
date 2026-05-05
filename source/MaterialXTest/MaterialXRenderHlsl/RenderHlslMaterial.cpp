//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
// Validation that HlslMaterial wraps the bind-and-draw flow correctly.
// Renders a fullscreen triangle whose pixel shader reads its color from
// a single cbuffer and asserts that updating the cbuffer through the
// material is observable in the rendered output.
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXRenderHlsl/HlslContext.h>
#include <MaterialXRenderHlsl/HlslFramebuffer.h>
#include <MaterialXRenderHlsl/HlslMaterial.h>
#include <MaterialXRenderHlsl/HlslProgram.h>

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

namespace mx = MaterialX;
using Microsoft::WRL::ComPtr;

namespace
{

const char* kVs = R"(
struct VsIn  { float3 pos : POSITION; };
struct VsOut { float4 pos : SV_Position; };

VsOut VSMain(VsIn vin)
{
    VsOut vout;
    vout.pos = float4(vin.pos, 1.0);
    return vout;
}
)";

// Pixel shader: read one float4 from a cbuffer and emit it. The cbuffer
// has a single member so the byte layout is unambiguous.
const char* kPs = R"(
struct VsOut { float4 pos : SV_Position; };

cbuffer Tint_pixel
{
    float4 u_tint;
};

float4 PSMain(VsOut pin) : SV_Target
{
    return u_tint;
}
)";

mx::HlslContextPtr tryCreateContext()
{
    try { return mx::HlslContext::create(); }
    catch (const std::exception&) { return nullptr; }
}

} // namespace

TEST_CASE("Render: Hlsl Material BindCbufferAndDraw", "[renderhlsl]")
{
    mx::HlslContextPtr ctx = tryCreateContext();
    if (!ctx)
    {
        WARN("HlslContext could not be created; skipping material test.");
        return;
    }

    mx::HlslProgramPtr prog = mx::HlslProgram::create();
    REQUIRE(prog->build(std::string(kVs), std::string(kPs)));

    mx::HlslMaterialPtr mat = mx::HlslMaterial::create(ctx, prog);
    REQUIRE(mat->getVertexShader() != nullptr);
    REQUIRE(mat->getPixelShader()  != nullptr);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    REQUIRE(mat->createInputLayout(layoutDesc, 1));

    // Push the tint into the cbuffer via the material.
    const float tint[4] = { 0.1f, 0.6f, 0.3f, 1.0f };
    REQUIRE(mat->setCbufferDataByName("Tint_pixel", tint, sizeof(tint)));

    // Vertex buffer for an over-sized triangle covering the whole NDC.
    ID3D11Device* device = ctx->getDevice();
    ID3D11DeviceContext* dc = ctx->getDeviceContext();

    float verts[] = {
        -3.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  3.0f, 0.0f,
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

    constexpr unsigned int W = 32;
    constexpr unsigned int H = 32;
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

    mx::ImagePtr image = fb->readColor();
    REQUIRE(image);

    auto* pixels = static_cast<const uint8_t*>(image->getResourceBuffer());
    const std::size_t centre = ((H / 2) * W + W / 2) * 4;
    auto approxEq = [](unsigned int a, unsigned int b) {
        return (a > b ? a - b : b - a) <= 1;
    };
    REQUIRE(approxEq(pixels[centre + 0], 26));   // 0.1 * 255 = 25.5
    REQUIRE(approxEq(pixels[centre + 1], 153));  // 0.6 * 255 = 153
    REQUIRE(approxEq(pixels[centre + 2], 77));   // 0.3 * 255 = 76.5
    REQUIRE(pixels[centre + 3] == 255);
}
