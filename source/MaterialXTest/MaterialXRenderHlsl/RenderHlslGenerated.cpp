//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
// Render a real generated MaterialX shader through HlslMaterial. We do
// not yet plumb MaterialX uniform values into the cbuffers (that is the
// next layer up - a ShaderRenderer subclass), so the surface evaluates
// with zero-initialised inputs. The aim of this test is structural: the
// reflection-driven cbuffer allocation, the bind, and the draw must all
// succeed on a production-scale shader, and the framebuffer must come
// back with a valid alpha channel proving the pixel shader actually ran.
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXRenderHlsl/HlslContext.h>
#include <MaterialXRenderHlsl/HlslFramebuffer.h>
#include <MaterialXRenderHlsl/HlslMaterial.h>
#include <MaterialXRenderHlsl/HlslProgram.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXCore/Document.h>

#include <algorithm>
#include <array>
#include <cstring>

#define NOMINMAX 1
#include <Windows.h>
#include <d3d11.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

namespace mx = MaterialX;
using Microsoft::WRL::ComPtr;

namespace
{

mx::HlslContextPtr tryCreateContext()
{
    try { return mx::HlslContext::create(); }
    catch (const std::exception&) { return nullptr; }
}

} // namespace

TEST_CASE("Render: Hlsl Generated Carpaint", "[renderhlsl]")
{
    mx::HlslContextPtr ctx = tryCreateContext();
    if (!ctx)
    {
        WARN("HlslContext could not be created; skipping generated-shader render test.");
        return;
    }

    // Generate the HLSL pair for standard_surface_carpaint (procedural, no
    // file textures, exercises lighting/pbr cbuffers but no t#/s# slots).
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::FilePath testFile = searchPath.find("resources/Materials/Examples/StandardSurface/standard_surface_carpaint.mtlx");
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, testFile);
    doc->setDataLibrary(libraries);

    mx::ElementPtr element = doc->getChild("Car_Paint");
    REQUIRE(element);

    mx::GenContext genContext(mx::HlslShaderGenerator::create());
    genContext.registerSourceCodeSearchPath(searchPath);
    genContext.getShaderGenerator().registerTypeDefs(doc);
    mx::ShaderPtr shader = genContext.getShaderGenerator().generate(element->getName(), element, genContext);
    REQUIRE(shader);

    // Compile via the FXC SM 5.0 path, default for HlslProgram.
    mx::HlslProgramPtr prog = mx::HlslProgram::create();
    bool built = prog->build(shader);
    INFO("HlslProgram log:\n" << prog->getCompileLog());
    REQUIRE(built);

    // Wrap with HlslMaterial. Reflection-driven cbuffer allocation walks
    // every reflected b# slot in the production shader, on both stages.
    mx::HlslMaterialPtr mat = mx::HlslMaterial::create(ctx, prog);

    // The pixel-stage reflection should report at least one cbuffer; if
    // the binding-context path were active there would be three (Private,
    // Public, LightData), but the default path packs all value uniforms
    // into one stage-named cbuffer.
    REQUIRE_FALSE(mat->getPixelBindings().empty());

    // Push identity matrices into the vertex-stage cbuffer so the VS
    // doesn't collapse every vertex to (0, 0, 0, 0). The generator names
    // the per-stage cbuffer "<stage>CB". We look up the offsets via
    // reflection, build a single payload large enough to cover all of
    // them, splat identity in place, and push the whole thing once
    // (UpdateSubresource overwrites the full subresource, so multiple
    // partial writes would clobber each other).
    const float identity[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    const std::array<const char*, 3> matrixNames = {
        "u_worldMatrix",
        "u_viewProjectionMatrix",
        "u_worldInverseTransposeMatrix",
    };
    std::size_t maxEnd = 0;
    std::array<std::size_t, 3> offsets = { std::size_t(-1), std::size_t(-1), std::size_t(-1) };
    for (std::size_t i = 0; i < matrixNames.size(); ++i)
    {
        offsets[i] = mat->lookupVariableOffset(
            mx::HlslMaterial::Stage::Vertex, "vertexCB", matrixNames[i]);
        if (offsets[i] != std::size_t(-1))
            maxEnd = std::max(maxEnd, offsets[i] + sizeof(identity));
    }
    if (maxEnd > 0)
    {
        std::vector<uint8_t> payload(maxEnd, 0);
        for (std::size_t i = 0; i < matrixNames.size(); ++i)
        {
            if (offsets[i] != std::size_t(-1))
                std::memcpy(payload.data() + offsets[i], identity, sizeof(identity));
        }
        REQUIRE(mat->setCbufferDataByName(mx::HlslMaterial::Stage::Vertex,
                                          "vertexCB", payload.data(), payload.size()));
    }

    // The MaterialX HLSL VS for a surface shader pulls POSITION, NORMAL,
    // and TANGENT from the input stream. We don't actually drive any of
    // these meaningfully (the test renders an over-sized triangle whose
    // attributes are unused for the alpha-only check below); they just
    // need to be declared so the input layout matches the VS signature.
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    REQUIRE(mat->createInputLayout(layoutDesc, 3));

    // Classic fullscreen triangle: covers all of NDC ([-1,1]^2). Vertices
    // are intentionally past the unit square so rasterisation hits every
    // pixel exactly once with no seams. Each vertex carries (POSITION,
    // NORMAL, TANGENT) = 9 floats; normals point at +Z and tangents at
    // +X (arbitrary, just to keep the stream well-defined).
    ID3D11Device* device = ctx->getDevice();
    ID3D11DeviceContext* dc = ctx->getDeviceContext();
    const float verts[] = {
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
         3.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        -1.0f,  3.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
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
    // Clear to magenta so we can tell whether the shader actually wrote
    // any pixels (the HLSL gen for a surface shader emits alpha = 1
    // unconditionally for the final SV_Target write).
    fb->clear(mx::Color4(1.0f, 0.0f, 1.0f, 0.5f));

    UINT stride = 9 * sizeof(float);
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

    // Every pixel should now have alpha = 255 (the surface shader writes
    // a.a = 1.0 unconditionally; clear was 0.5). Sample several pixels.
    auto* pixels = static_cast<const uint8_t*>(image->getResourceBuffer());
    REQUIRE(pixels);
    for (unsigned int y : { 0u, H / 2, H - 1 })
    {
        for (unsigned int x : { 0u, W / 2, W - 1 })
        {
            const std::size_t a = (y * W + x) * 4 + 3;
            INFO("alpha@(" << x << "," << y << ") = " << (unsigned)pixels[a]);
            REQUIRE(pixels[a] == 255);
        }
    }
}
