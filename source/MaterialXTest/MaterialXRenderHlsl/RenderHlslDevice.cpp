//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
// Smoke test for the D3D11 plumbing under MaterialXRenderHlsl: device
// creation, off-screen framebuffer, clear, CPU readback.
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXRenderHlsl/HlslContext.h>
#include <MaterialXRenderHlsl/HlslFramebuffer.h>

namespace mx = MaterialX;

namespace
{

// Wrapper that swallows the device-creation exception when no D3D11
// driver is present (rare on real Windows hosts; possible in stripped-
// down CI containers). Tests skip with a WARN in that case rather than
// failing.
mx::HlslContextPtr tryCreateContext(std::string& reason)
{
    try
    {
        return mx::HlslContext::create();
    }
    catch (const std::exception& e)
    {
        reason = e.what();
        return nullptr;
    }
}

} // namespace

TEST_CASE("Render: Hlsl D3D11 Context", "[renderhlsl]")
{
    std::string reason;
    mx::HlslContextPtr ctx = tryCreateContext(reason);
    if (!ctx)
    {
        WARN("HlslContext could not be created (" << reason
             << "); skipping device tests.");
        return;
    }

    REQUIRE(ctx->getDevice() != nullptr);
    REQUIRE(ctx->getDeviceContext() != nullptr);
}

TEST_CASE("Render: Hlsl Framebuffer ClearAndReadback", "[renderhlsl]")
{
    std::string reason;
    mx::HlslContextPtr ctx = tryCreateContext(reason);
    if (!ctx)
    {
        WARN("HlslContext could not be created; skipping framebuffer tests.");
        return;
    }

    constexpr unsigned int W = 16;
    constexpr unsigned int H = 8;
    mx::HlslFramebufferPtr fb = mx::HlslFramebuffer::create(ctx, W, H);
    REQUIRE(fb->getWidth() == W);
    REQUIRE(fb->getHeight() == H);

    // Test asserts raw byte values (0.25 -> 64, 0.5 -> 128, 0.75 -> 191),
    // i.e. linear-pass-through. Disable sRGB encoding so the framebuffer
    // uses the UNORM RTV instead of the default UNORM_SRGB one.
    fb->setEncodeSrgb(false);
    fb->bind();
    fb->clear(mx::Color4(0.25f, 0.5f, 0.75f, 1.0f));
    fb->unbind();

    mx::ImagePtr image = fb->readColor();
    REQUIRE(image);
    REQUIRE(image->getWidth() == W);
    REQUIRE(image->getHeight() == H);
    REQUIRE(image->getChannelCount() == 4);
    REQUIRE(image->getBaseType() == mx::Image::BaseType::UINT8);

    // R8G8B8A8_UNORM stores the cleared color as 8-bit ints. Verify the
    // first pixel matches what we cleared (within +/- 1 to allow for the
    // standard quantization).
    auto* pixels = static_cast<const uint8_t*>(image->getResourceBuffer());
    REQUIRE(pixels);
    auto approxEq = [](unsigned int a, unsigned int b) {
        return (a > b ? a - b : b - a) <= 1;
    };
    REQUIRE(approxEq(pixels[0], 64));   // 0.25 * 255 = 63.75 -> 64
    REQUIRE(approxEq(pixels[1], 128));  // 0.5  * 255 = 127.5 -> 128
    REQUIRE(approxEq(pixels[2], 191));  // 0.75 * 255 = 191.25 -> 191
    REQUIRE(pixels[3] == 255);

    // Last pixel of the framebuffer should hold the same color (uniform
    // clear).
    const std::size_t lastOffset = (static_cast<std::size_t>(H) * W - 1) * 4;
    REQUIRE(approxEq(pixels[lastOffset + 0], 64));
    REQUIRE(approxEq(pixels[lastOffset + 1], 128));
    REQUIRE(approxEq(pixels[lastOffset + 2], 191));
    REQUIRE(pixels[lastOffset + 3] == 255);
}
