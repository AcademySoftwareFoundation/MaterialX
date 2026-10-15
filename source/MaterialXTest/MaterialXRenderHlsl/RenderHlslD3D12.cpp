//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
// Validates the D3D12 path of the HLSL renderer:
//   - shader reflection and the root signature layout derived from it,
//     which need no device;
//   - framebuffer clear and readback;
//   - HlslD3D12Renderer drawing generated materials, with and without
//     file textures and lighting, and agreeing with the D3D11 renderer;
//   - the texture bakers capturing into a caller-provided image.
// Device-dependent cases skip cleanly when no D3D12 device is available.
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXRenderHlsl/HlslD3D12Renderer.h>
#include <MaterialXRenderHlsl/HlslRenderer.h>
#include <MaterialXRenderHlsl/HlslRootSignature.h>
#include <MaterialXRenderHlsl/TextureBaker.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXRender/Camera.h>
#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/TinyObjLoader.h>

#include <cmath>

namespace mx = MaterialX;

namespace
{

mx::HlslD3D12RendererPtr tryCreateD3D12Renderer(unsigned int width, unsigned int height)
{
    mx::HlslD3D12RendererPtr renderer = mx::HlslD3D12Renderer::create(width, height, mx::Image::BaseType::UINT8);
    try
    {
        renderer->initialize();
    }
    catch (const std::exception&)
    {
        return nullptr;
    }
    return renderer;
}

mx::HlslRendererPtr tryCreateD3D11Renderer(unsigned int width, unsigned int height)
{
    mx::HlslRendererPtr renderer = mx::HlslRenderer::create(width, height, mx::Image::BaseType::UINT8);
    try
    {
        renderer->initialize();
    }
    catch (const std::exception&)
    {
        return nullptr;
    }
    return renderer;
}

// Load a document from the resources folder and generate HLSL for its first
// surfacematerial node.
mx::ShaderPtr generateSurfaceMaterial(const std::string& path)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, searchPath.find(path));
    doc->setDataLibrary(libraries);

    mx::ElementPtr element;
    for (mx::ElementPtr child : doc->getChildren())
    {
        if (child->isA<mx::Node>() && child->asA<mx::Node>()->getCategory() == "surfacematerial")
        {
            element = child;
            break;
        }
    }
    if (!element)
        return nullptr;

    mx::GenContext context(mx::HlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);
    context.getShaderGenerator().registerTypeDefs(doc);
    return context.getShaderGenerator().generate(element->getName(), element, context);
}

// Generate HLSL for a nodegraph output holding a constant color.
mx::ShaderPtr generateConstantColor(const mx::Color3& color)
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::DocumentPtr doc = mx::createDocument();
    doc->setDataLibrary(libraries);
    mx::NodeGraphPtr graph = doc->addNodeGraph("NG_constant");
    mx::NodePtr constant = graph->addNode("constant", "constant1", "color3");
    constant->setInputValue("value", color);
    mx::OutputPtr output = graph->addOutput("out", "color3");
    output->setConnectedNode(constant);

    mx::GenContext context(mx::HlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);
    return context.getShaderGenerator().generate("ConstantColor", output, context);
}

// Build a camera framing the first mesh of the geometry handler.
mx::CameraPtr createFramingCamera(mx::GeometryHandlerPtr geometry, float aspect)
{
    mx::MeshPtr mesh = geometry->getMeshes()[0];
    const mx::Vector3 center = mesh->getSphereCenter();
    const float radius = mesh->getSphereRadius();
    const float fovY = 30.0f * 3.14159265f / 180.0f;
    const float distance = radius / std::tan(fovY * 0.5f) * 1.2f;
    const float nearPlane = std::max(0.01f, distance - radius * 2.0f);
    const float farPlane = distance + radius * 2.0f;
    const float halfHeight = nearPlane * std::tan(fovY * 0.5f);

    mx::CameraPtr camera = mx::Camera::create();
    camera->setViewMatrix(mx::Camera::createViewMatrix(center + mx::Vector3(0.0f, 0.0f, distance), center,
                                                       mx::Vector3(0.0f, 1.0f, 0.0f)));
    camera->setProjectionMatrix(mx::Camera::createPerspectiveMatrix(-halfHeight * aspect, halfHeight * aspect,
                                                                    -halfHeight, halfHeight, nearPlane, farPlane));
    return camera;
}

struct PixelDelta
{
    int max = 0;
    double mean = 0.0;
};

PixelDelta compareImages(mx::ImagePtr a, mx::ImagePtr b)
{
    PixelDelta delta;
    const auto* pa = static_cast<const uint8_t*>(a->getResourceBuffer());
    const auto* pb = static_cast<const uint8_t*>(b->getResourceBuffer());
    const std::size_t count = static_cast<std::size_t>(a->getWidth()) * a->getHeight() * 4;
    double sum = 0.0;
    for (std::size_t i = 0; i < count; ++i)
    {
        const int d = std::abs(static_cast<int>(pa[i]) - static_cast<int>(pb[i]));
        delta.max = std::max(delta.max, d);
        sum += d;
    }
    delta.mean = sum / static_cast<double>(count);
    return delta;
}

} // namespace

TEST_CASE("Render: Hlsl D3D12 Root Signature", "[renderhlsl]")
{
    mx::ShaderPtr shader = generateSurfaceMaterial("resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx");
    REQUIRE(shader);

    mx::HlslProgramPtr program = mx::HlslProgram::create();
    if (mx::HlslProgram::isDxcAvailable())
    {
        program->setCompilerBackend(mx::HlslCompilerBackend::Dxc);
        program->setShaderModel("6_0");
    }
    REQUIRE(program->build(shader));

    const mx::HlslStageReflection vs = program->getVertexReflection();
    const mx::HlslStageReflection ps = program->getPixelReflection();
    REQUIRE(vs.isValid());
    REQUIRE(ps.isValid());

    // The vertex stage reads at least a position, and owns the matrices.
    bool hasPosition = false;
    for (const mx::HlslInputParameter& input : vs.getInputs())
        hasPosition = hasPosition || (input.semanticName == "POSITION" && input.componentCount == 3);
    CHECK(hasPosition);
    mx::HlslUniformLocation location;
    CHECK(vs.findVariable("u_worldMatrix", location));
    CHECK(!ps.findVariable("u_worldMatrix", location));

    // Brass binds the two environment maps and two file textures, each a
    // texture plus a sampler.
    CHECK(ps.getSlotCount(mx::HlslResourceType::Texture) == 4);
    CHECK(ps.getSlotCount(mx::HlslResourceType::Sampler) == 4);

    const mx::HlslRootSignatureDesc desc = mx::HlslRootSignatureDesc::create(vs, ps);
    CHECK(desc.findConstantBuffer(mx::HlslShaderVisibility::Vertex, 0) >= 0);
    CHECK(desc.findConstantBuffer(mx::HlslShaderVisibility::Pixel, 0) >= 0);
    const int textures = desc.findTable(mx::HlslRootParameterType::TextureTable, mx::HlslShaderVisibility::Pixel);
    const int samplers = desc.findTable(mx::HlslRootParameterType::SamplerTable, mx::HlslShaderVisibility::Pixel);
    REQUIRE(textures >= 0);
    REQUIRE(samplers >= 0);
    CHECK(desc.getParameters()[textures].registerCount == 4);
    CHECK(desc.getParameters()[samplers].registerCount == 4);
    CHECK(desc.findTable(mx::HlslRootParameterType::TextureTable, mx::HlslShaderVisibility::Vertex) < 0);
    CHECK(desc.getRootCost() <= 64);
}

TEST_CASE("Render: Hlsl D3D12 Light Array Layout", "[renderhlsl]")
{
    mx::ShaderPtr shader = generateSurfaceMaterial("resources/Materials/Examples/StandardSurface/standard_surface_carpaint.mtlx");
    REQUIRE(shader);

    mx::HlslProgramPtr program = mx::HlslProgram::create();
    REQUIRE(program->build(shader));
    const mx::HlslStageReflection ps = program->getPixelReflection();
    REQUIRE(ps.isValid());

    const mx::HlslReflectedVariable* lightData = nullptr;
    for (const mx::HlslReflectedCbuffer& cb : ps.getCbuffers())
    {
        for (const mx::HlslReflectedVariable& var : cb.variables)
        {
            if (var.name == mx::HW::LIGHT_DATA_INSTANCE)
                lightData = &var;
        }
    }
    REQUIRE(lightData);
    REQUIRE(lightData->elements > 1);
    REQUIRE(!lightData->members.empty());

    // Each element starts on a 16-byte boundary, and consecutive elements
    // of the same member are one stride apart.
    const unsigned int stride = lightData->getElementStride();
    CHECK(stride % 16 == 0);
    CHECK((lightData->elements - 1) * stride < lightData->size);
    CHECK(lightData->size <= lightData->elements * stride);

    mx::HlslUniformLocation first;
    mx::HlslUniformLocation second;
    REQUIRE(ps.findArrayMember(mx::HW::LIGHT_DATA_INSTANCE, 0, "type", first));
    REQUIRE(ps.findArrayMember(mx::HW::LIGHT_DATA_INSTANCE, 1, "type", second));
    CHECK(second.offset - first.offset == stride);
    CHECK(!ps.findArrayMember(mx::HW::LIGHT_DATA_INSTANCE, lightData->elements, "type", second));
}

TEST_CASE("Render: Hlsl D3D12 Framebuffer ClearAndReadback", "[renderhlsl]")
{
    mx::HlslD3D12ContextPtr context;
    try
    {
        context = mx::HlslD3D12Context::create();
    }
    catch (const std::exception&)
    {
        WARN("No D3D12 device available; skipping framebuffer test.");
        return;
    }

    mx::HlslD3D12FramebufferPtr fb = mx::HlslD3D12Framebuffer::create(context, 8, 4);
    auto clearTo = [&](const mx::Color4& color)
    {
        ID3D12GraphicsCommandList* list = context->beginFrame();
        fb->bind(list);
        fb->clear(list, color);
        context->endFrame();
    };

    // sRGB encoding (default): linear 0.5 encodes to 188.
    clearTo(mx::Color4(1.0f, 0.0f, 0.5f, 1.0f));
    mx::ImagePtr image = fb->readColor();
    REQUIRE(image);
    REQUIRE(image->getWidth() == 8);
    REQUIRE(image->getHeight() == 4);
    REQUIRE(image->getChannelCount() == 4);
    const auto* px = static_cast<const uint8_t*>(image->getResourceBuffer());
    CHECK(px[0] == 255);
    CHECK(px[1] == 0);
    CHECK(std::abs(static_cast<int>(px[2]) - 188) <= 1);
    CHECK(px[3] == 255);

    // Linear storage, read back into a caller-provided image.
    fb->setEncodeSrgb(false);
    clearTo(mx::Color4(1.0f, 0.0f, 0.5f, 1.0f));
    mx::ImagePtr target = mx::Image::create(8, 4, 4, mx::Image::BaseType::UINT8);
    target->createResourceBuffer();
    REQUIRE(fb->readColor(target) == target);
    px = static_cast<const uint8_t*>(target->getResourceBuffer());
    const std::size_t last = (3 * 8 + 7) * 4;
    CHECK(std::abs(static_cast<int>(px[last + 2]) - 128) <= 1);
}

TEST_CASE("Render: Hlsl D3D12 Renderer Carpaint", "[renderhlsl]")
{
    constexpr unsigned int W = 32;
    constexpr unsigned int H = 32;
    mx::HlslD3D12RendererPtr renderer = tryCreateD3D12Renderer(W, H);
    if (!renderer)
    {
        WARN("HlslD3D12Renderer could not be initialised; skipping renderer test.");
        return;
    }

    mx::CameraPtr camera = mx::Camera::create();
    camera->setWorldMatrix(mx::Matrix44::IDENTITY);
    camera->setViewMatrix(mx::Matrix44::IDENTITY);
    camera->setProjectionMatrix(mx::Matrix44::IDENTITY);
    renderer->setCamera(camera);
    renderer->setScreenColor(mx::Color4(0.0f, 0.0f, 0.0f, 0.5f));

    mx::ShaderPtr shader = generateSurfaceMaterial("resources/Materials/Examples/StandardSurface/standard_surface_carpaint.mtlx");
    REQUIRE(shader);
    REQUIRE_NOTHROW(renderer->createProgram(shader));
    REQUIRE_NOTHROW(renderer->validateInputs());
    REQUIRE_NOTHROW(renderer->render());

    mx::ImagePtr out = renderer->captureImage();
    REQUIRE(out);
    REQUIRE(out->getWidth() == W);
    REQUIRE(out->getHeight() == H);

    // The surface shader writes alpha = 1 over the clear alpha of 0.5, so
    // opaque samples across the viewport show the fullscreen draw covered it.
    const auto* px = static_cast<const uint8_t*>(out->getResourceBuffer());
    for (unsigned int y : { 0u, H / 2, H - 1 })
    {
        for (unsigned int x : { 0u, W / 2, W - 1 })
        {
            const std::size_t a = (y * W + x) * 4 + 3;
            INFO("alpha@(" << x << "," << y << ") = " << (unsigned) px[a]);
            REQUIRE(px[a] == 255);
        }
    }
}

TEST_CASE("Render: Hlsl D3D12 Renderer Resize", "[renderhlsl]")
{
    mx::HlslD3D12RendererPtr renderer = tryCreateD3D12Renderer(16, 16);
    if (!renderer)
    {
        WARN("HlslD3D12Renderer could not be initialised; skipping resize test.");
        return;
    }
    auto before = renderer->getFramebuffer();
    renderer->setSize(16, 16);
    CHECK(renderer->getFramebuffer() == before);
    renderer->setSize(64, 32);
    CHECK(renderer->getFramebuffer()->getWidth() == 64);
    CHECK(renderer->getFramebuffer()->getHeight() == 32);
}

TEST_CASE("Render: Hlsl D3D12 Renderer Image Binding", "[renderhlsl]")
{
    mx::HlslD3D12RendererPtr renderer = tryCreateD3D12Renderer(32, 32);
    if (!renderer)
    {
        WARN("HlslD3D12Renderer could not be initialised; skipping image-binding test.");
        return;
    }

    mx::ShaderPtr shader = generateSurfaceMaterial("resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx");
    REQUIRE(shader);
    REQUIRE_NOTHROW(renderer->createProgram(shader));

    mx::ImagePtr solid = mx::createUniformImage(4, 4, 4, mx::Image::BaseType::UINT8, mx::Color4(0.8f, 0.4f, 0.2f, 1.0f));
    CHECK(renderer->bindImage("image_color_file", solid));
    CHECK(!renderer->bindImage("not_a_texture", solid));

    // render() rebinds file textures through the image handler. The
    // material folder is on the search path so that the brass file prefix
    // resolves to the bundled images.
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    searchPath.append(searchPath.find("resources/Materials/Examples/StandardSurface"));
    mx::ImageHandlerPtr images = mx::ImageHandler::create(mx::StbImageLoader::create());
    images->setSearchPath(searchPath);
    renderer->setImageHandler(images);
    REQUIRE_NOTHROW(renderer->render());
    mx::ImagePtr out = renderer->captureImage();
    REQUIRE(out);
    const auto* px = static_cast<const uint8_t*>(out->getResourceBuffer());
    CHECK(px[(16 * 32 + 16) * 4 + 3] == 255);
}

TEST_CASE("Render: Hlsl D3D12 Matches D3D11", "[renderhlsl]")
{
    constexpr unsigned int W = 64;
    constexpr unsigned int H = 64;
    mx::HlslD3D12RendererPtr d3d12 = tryCreateD3D12Renderer(W, H);
    mx::HlslRendererPtr d3d11 = tryCreateD3D11Renderer(W, H);
    if (!d3d12 || !d3d11)
    {
        WARN("D3D11 or D3D12 renderer could not be initialised; skipping comparison test.");
        return;
    }

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    mx::GeometryHandlerPtr geometry = mx::GeometryHandler::create();
    geometry->addLoader(mx::TinyObjLoader::create());
    REQUIRE(geometry->loadGeometry(searchPath.find("resources/Geometry/sphere.obj")));
    mx::CameraPtr camera = createFramingCamera(geometry, static_cast<float>(W) / H);

    // One CPU image handler shared by both renderers; each renderer uploads
    // through its own texture handler. The material folder is on the search
    // path so that relative file prefixes resolve.
    mx::ImageHandlerPtr images = mx::ImageHandler::create(mx::StbImageLoader::create());
    mx::FileSearchPath imageSearchPath = searchPath;
    imageSearchPath.append(searchPath.find("resources/Materials/Examples/StandardSurface"));
    images->setSearchPath(imageSearchPath);

    mx::LightHandlerPtr lights = mx::LightHandler::create();
    lights->setEnvRadianceMap(images->acquireImage(searchPath.find("resources/Lights/san_giuseppe_bridge_split.hdr")));
    lights->setEnvIrradianceMap(images->acquireImage(searchPath.find("resources/Lights/irradiance/san_giuseppe_bridge_split.hdr")));
    lights->setEnvSampleCount(64);

    for (mx::ShaderRendererPtr renderer : { mx::ShaderRendererPtr(d3d12), mx::ShaderRendererPtr(d3d11) })
    {
        renderer->setGeometryHandler(geometry);
        renderer->setCamera(camera);
        renderer->setImageHandler(images);
        renderer->setLightHandler(lights);
    }

    for (const std::string& path : { std::string("resources/Materials/Examples/StandardSurface/standard_surface_carpaint.mtlx"),
                                     std::string("resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx") })
    {
        mx::ShaderPtr shader = generateSurfaceMaterial(path);
        REQUIRE(shader);
        REQUIRE_NOTHROW(d3d12->createProgram(shader));
        REQUIRE_NOTHROW(d3d11->createProgram(shader));
        REQUIRE_NOTHROW(d3d12->render());
        REQUIRE_NOTHROW(d3d11->render());

        const PixelDelta delta = compareImages(d3d12->captureImage(), d3d11->captureImage());
        INFO(path << ": max delta " << delta.max << ", mean delta " << delta.mean);
        CHECK(delta.max <= 16);
        CHECK(delta.mean <= 0.5);
    }
}

TEST_CASE("Render: Hlsl TextureBaker Capture", "[renderhlsl]")
{
    // Both bakers render a constant color into texture space and capture it
    // into the baker's own pre-allocated image, as TextureBaker does.
    const mx::Color3 color(0.5f, 0.25f, 1.0f);
    mx::ShaderPtr shader = generateConstantColor(color);
    REQUIRE(shader);

    auto checkCapture = [&](mx::ImagePtr target, mx::ImagePtr captured)
    {
        REQUIRE(captured == target);
        const auto* px = static_cast<const uint8_t*>(target->getResourceBuffer());
        const std::size_t center = (8 * 16 + 8) * 4;
        CHECK(std::abs(static_cast<int>(px[center + 0]) - 128) <= 1);
        CHECK(std::abs(static_cast<int>(px[center + 1]) - 64) <= 1);
        CHECK(px[center + 2] == 255);
        CHECK(px[center + 3] == 255);
    };

    mx::TextureBakerHlslD3D12Ptr d3d12Baker;
    try
    {
        d3d12Baker = mx::TextureBakerHlslD3D12::create(16, 16);
    }
    catch (const std::exception&)
    {
        WARN("No D3D12 device available; skipping D3D12 baker capture.");
    }
    if (d3d12Baker)
    {
        d3d12Baker->getFramebuffer()->setEncodeSrgb(false);
        REQUIRE_NOTHROW(d3d12Baker->createProgram(shader));
        d3d12Baker->renderTextureSpace(mx::Vector2(0.0f, 0.0f), mx::Vector2(1.0f, 1.0f));
        mx::ImagePtr target = mx::Image::create(16, 16, 4, mx::Image::BaseType::UINT8);
        target->createResourceBuffer();
        checkCapture(target, d3d12Baker->captureImage(target));
    }

    mx::TextureBakerHlslPtr d3d11Baker;
    try
    {
        d3d11Baker = mx::TextureBakerHlsl::create(16, 16);
    }
    catch (const std::exception&)
    {
        WARN("No D3D11 device available; skipping D3D11 baker capture.");
    }
    if (d3d11Baker)
    {
        d3d11Baker->getFramebuffer()->setEncodeSrgb(false);
        REQUIRE_NOTHROW(d3d11Baker->createProgram(shader));
        d3d11Baker->renderTextureSpace(mx::Vector2(0.0f, 0.0f), mx::Vector2(1.0f, 1.0f));
        mx::ImagePtr target = mx::Image::create(16, 16, 4, mx::Image::BaseType::UINT8);
        target->createResourceBuffer();
        checkCapture(target, d3d11Baker->captureImage(target));
    }
}
