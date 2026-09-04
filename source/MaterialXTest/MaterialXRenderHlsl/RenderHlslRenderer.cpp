//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
// Drives a generated MaterialX shader through the public ShaderRenderer
// interface (HlslRenderer override). Validates that the renderer:
//   - initialises a D3D11 device + framebuffer + texture handler;
//   - compiles a generated shader and creates a usable pipeline;
//   - writes camera matrices into vertexCB without the caller having
//     to know reflected offsets;
//   - rasterises and reads back the framebuffer through captureImage().
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXGenHlsl/HlslResourceBindingContext.h>
#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXRenderHlsl/HlslRenderer.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXRender/Camera.h>
#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/Mesh.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/TinyObjLoader.h>
#include <MaterialXCore/Document.h>

namespace mx = MaterialX;

namespace
{

mx::HlslRendererPtr tryCreateRenderer(unsigned int W, unsigned int H)
{
    mx::HlslRendererPtr r = mx::HlslRenderer::create(W, H, mx::Image::BaseType::UINT8);
    try
    {
        r->initialize();
    }
    catch (const std::exception&)
    {
        return nullptr;
    }
    return r;
}

} // namespace

TEST_CASE("Render: Hlsl Renderer Carpaint", "[renderhlsl]")
{
    constexpr unsigned int W = 32;
    constexpr unsigned int H = 32;
    mx::HlslRendererPtr renderer = tryCreateRenderer(W, H);
    if (!renderer)
    {
        WARN("HlslRenderer could not be initialised; skipping renderer test.");
        return;
    }

    // Standard handlers - only the camera is wired up here, since this
    // test exercises the renderer-internal cbuffer-from-camera plumbing
    // and not the not-yet-implemented mesh / light / image integration.
    mx::CameraPtr camera = mx::Camera::create();
    camera->setWorldMatrix(mx::Matrix44::IDENTITY);
    camera->setViewMatrix(mx::Matrix44::IDENTITY);
    camera->setProjectionMatrix(mx::Matrix44::IDENTITY);
    renderer->setCamera(camera);
    renderer->setScreenColor(mx::Color4(0.0f, 0.0f, 0.0f, 0.5f));

    // Generate the HLSL pair for standard_surface_carpaint.
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

    REQUIRE_NOTHROW(renderer->createProgram(shader));
    REQUIRE_NOTHROW(renderer->validateInputs());
    REQUIRE_NOTHROW(renderer->render());

    mx::ImagePtr out = renderer->captureImage();
    REQUIRE(out);
    REQUIRE(out->getWidth() == W);
    REQUIRE(out->getHeight() == H);
    REQUIRE(out->getChannelCount() == 4);

    // The MaterialX surface shader unconditionally writes alpha = 1.0
    // (clear was 0.5 -> 128). Sampling 9 pixels confirms the entire
    // viewport rasterised through the public render() entry point.
    auto* px = static_cast<const uint8_t*>(out->getResourceBuffer());
    REQUIRE(px);
    for (unsigned int y : { 0u, H / 2, H - 1 })
        for (unsigned int x : { 0u, W / 2, W - 1 })
        {
            const std::size_t a = (y * W + x) * 4 + 3;
            INFO("alpha@(" << x << "," << y << ") = " << (unsigned)px[a]);
            REQUIRE(px[a] == 255);
        }
}

TEST_CASE("Render: Hlsl Renderer Resize", "[renderhlsl]")
{
    mx::HlslRendererPtr renderer = tryCreateRenderer(16, 16);
    if (!renderer)
    {
        WARN("HlslRenderer could not be initialised; skipping resize test.");
        return;
    }
    REQUIRE(renderer->getFramebuffer());
    REQUIRE(renderer->getFramebuffer()->getWidth() == 16);

    renderer->setSize(64, 32);
    REQUIRE(renderer->getFramebuffer()->getWidth() == 64);
    REQUIRE(renderer->getFramebuffer()->getHeight() == 32);

    // No-op resize must not reallocate the framebuffer (pointer-stable).
    auto fbBefore = renderer->getFramebuffer();
    renderer->setSize(64, 32);
    REQUIRE(renderer->getFramebuffer().get() == fbBefore.get());
}

TEST_CASE("Render: Hlsl Renderer BindImagesToTextureSlots", "[renderhlsl]")
{
    constexpr unsigned int W = 32;
    constexpr unsigned int H = 32;
    mx::HlslRendererPtr renderer = tryCreateRenderer(W, H);
    if (!renderer)
    {
        WARN("HlslRenderer could not be initialised; skipping image-binding test.");
        return;
    }

    mx::CameraPtr camera = mx::Camera::create();
    camera->setWorldMatrix(mx::Matrix44::IDENTITY);
    camera->setViewMatrix(mx::Matrix44::IDENTITY);
    camera->setProjectionMatrix(mx::Matrix44::IDENTITY);
    renderer->setCamera(camera);

    // standard_surface_brass_tiled has FILENAME inputs that the
    // generator emits as SamplerTexture2D handles named
    // image_color_file and image_roughness_file. Render the material;
    // bindImage must successfully route each procedurally-built Image
    // into the matching t# / s# slot pair.
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::FilePath testFile = searchPath.find("resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx");
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, testFile);
    doc->setDataLibrary(libraries);

    mx::ElementPtr element;
    for (auto child : doc->getChildren())
    {
        if (child->isA<mx::Node>() && child->asA<mx::Node>()->getCategory() == "surfacematerial")
        {
            element = child;
            break;
        }
    }
    REQUIRE(element);

    mx::GenContext genContext(mx::HlslShaderGenerator::create());
    genContext.registerSourceCodeSearchPath(searchPath);
    genContext.getShaderGenerator().registerTypeDefs(doc);
    mx::ShaderPtr shader = genContext.getShaderGenerator().generate(element->getName(), element, genContext);
    REQUIRE(shader);

    REQUIRE_NOTHROW(renderer->createProgram(shader));

    auto makeSolid = [](uint8_t r, uint8_t g, uint8_t b) -> mx::ImagePtr {
        mx::ImagePtr img = mx::Image::create(4, 4, 4, mx::Image::BaseType::UINT8);
        img->createResourceBuffer();
        auto* px = static_cast<uint8_t*>(img->getResourceBuffer());
        for (unsigned int i = 0; i < 16; ++i)
        {
            px[i * 4 + 0] = r;
            px[i * 4 + 1] = g;
            px[i * 4 + 2] = b;
            px[i * 4 + 3] = 255;
        }
        return img;
    };

    REQUIRE(renderer->bindImage("image_color_file",     makeSolid(200, 100,  50)));
    REQUIRE(renderer->bindImage("image_roughness_file", makeSolid(128, 128, 128)));
    // The env radiance / irradiance SRVs are auto-bound in render()
    // by bindEnvironmentImagesFromLightHandler() - providing an
    // ImageHandler is enough; no manual env binds needed.
    mx::ImageHandlerPtr ih = mx::ImageHandler::create(mx::StbImageLoader::create());
    renderer->setImageHandler(ih);

    REQUIRE_NOTHROW(renderer->render());

    mx::ImagePtr out = renderer->captureImage();
    REQUIRE(out);
    REQUIRE(out->getWidth() == W);
    REQUIRE(out->getHeight() == H);
    auto* px = static_cast<const uint8_t*>(out->getResourceBuffer());
    REQUIRE(px);
    const std::size_t centre = ((H / 2) * W + W / 2) * 4;
    REQUIRE(px[centre + 3] == 255);
}

TEST_CASE("Render: Hlsl Renderer AutoBindFromImageHandler", "[renderhlsl]")
{
    constexpr unsigned int W = 32;
    constexpr unsigned int H = 32;
    mx::HlslRendererPtr renderer = tryCreateRenderer(W, H);
    if (!renderer)
    {
        WARN("HlslRenderer could not be initialised; skipping image-handler test.");
        return;
    }

    mx::CameraPtr camera = mx::Camera::create();
    camera->setWorldMatrix(mx::Matrix44::IDENTITY);
    camera->setViewMatrix(mx::Matrix44::IDENTITY);
    camera->setProjectionMatrix(mx::Matrix44::IDENTITY);
    renderer->setCamera(camera);

    // Standard MaterialX image handler with the stb loader. Provides a
    // search path so the FILENAME default values in brass_tiled.mtlx
    // resolve to the bundled texture files under
    // resources/Images.
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    // Image handler with both the default data search path AND the
    // bundled resources/Images directory, since the brass material's
    // FILENAME defaults are bare basenames (brass_color.jpg, etc.).
    mx::FileSearchPath imageSearchPath = searchPath;
    imageSearchPath.append(searchPath.find("resources/Images"));
    mx::ImageHandlerPtr imageHandler = mx::ImageHandler::create(mx::StbImageLoader::create());
    imageHandler->setSearchPath(imageSearchPath);
    renderer->setImageHandler(imageHandler);

    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::FilePath testFile = searchPath.find("resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx");
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, testFile);
    doc->setDataLibrary(libraries);

    mx::ElementPtr element;
    for (auto child : doc->getChildren())
    {
        if (child->isA<mx::Node>() && child->asA<mx::Node>()->getCategory() == "surfacematerial")
        {
            element = child;
            break;
        }
    }
    REQUIRE(element);

    mx::GenContext genContext(mx::HlslShaderGenerator::create());
    genContext.registerSourceCodeSearchPath(searchPath);
    genContext.getShaderGenerator().registerTypeDefs(doc);
    mx::ShaderPtr shader = genContext.getShaderGenerator().generate(element->getName(), element, genContext);
    REQUIRE(shader);

    REQUIRE_NOTHROW(renderer->createProgram(shader));
    // No manual bindImage calls - render() must walk PUBLIC_UNIFORMS,
    // resolve the FILENAME inputs through the image handler, and bind
    // each one automatically.
    REQUIRE_NOTHROW(renderer->render());

    mx::ImagePtr out = renderer->captureImage();
    REQUIRE(out);
    REQUIRE(out->getWidth() == W);
    REQUIRE(out->getHeight() == H);
    auto* px = static_cast<const uint8_t*>(out->getResourceBuffer());
    REQUIRE(px);
    const std::size_t centre = ((H / 2) * W + W / 2) * 4;
    REQUIRE(px[centre + 3] == 255);
}

TEST_CASE("Render: Hlsl Renderer LightingScalarsLandInCbuffer", "[renderhlsl]")
{
    // After render() the env-related scalars on the LightHandler should
    // appear in the pixel cbuffer at the offsets reflection reports.
    // Read them back via the staging-texture-style cbuffer mirror is
    // overkill for a test; instead we re-do the same writePixelCbufferMember
    // logic ourselves via reflection to confirm the offsets are non-empty
    // (i.e. the cbuffer contains the expected uniforms) and that the
    // renderer's call ran without crashing.
    constexpr unsigned int W = 32;
    constexpr unsigned int H = 32;
    mx::HlslRendererPtr renderer = tryCreateRenderer(W, H);
    if (!renderer)
    {
        WARN("HlslRenderer could not be initialised; skipping lighting-scalars test.");
        return;
    }

    mx::CameraPtr camera = mx::Camera::create();
    camera->setWorldMatrix(mx::Matrix44::IDENTITY);
    camera->setViewMatrix(mx::Matrix44::IDENTITY);
    camera->setProjectionMatrix(mx::Matrix44::IDENTITY);
    renderer->setCamera(camera);

    mx::ImageHandlerPtr ih = mx::ImageHandler::create(mx::StbImageLoader::create());
    renderer->setImageHandler(ih);

    mx::LightHandlerPtr lh = mx::LightHandler::create();
    lh->setEnvLightIntensity(2.5f);
    lh->setEnvSampleCount(8);
    lh->setRefractionTwoSided(true);
    renderer->setLightHandler(lh);

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

    REQUIRE_NOTHROW(renderer->createProgram(shader));

    // Verify the pixel cbuffer has the uniforms we expect to write to.
    mx::HlslMaterialPtr mat = renderer->getMaterial();
    REQUIRE(mat);
    REQUIRE(mat->lookupVariableOffset(mx::HlslMaterial::Stage::Pixel, "pixelCB",
                                       "u_envMatrix") != std::size_t(-1));
    REQUIRE(mat->lookupVariableOffset(mx::HlslMaterial::Stage::Pixel, "pixelCB",
                                       "u_envLightIntensity") != std::size_t(-1));
    REQUIRE(mat->lookupVariableOffset(mx::HlslMaterial::Stage::Pixel, "pixelCB",
                                       "u_numActiveLightSources") != std::size_t(-1));

    REQUIRE_NOTHROW(renderer->render());

    mx::ImagePtr out = renderer->captureImage();
    REQUIRE(out);
    auto* px = static_cast<const uint8_t*>(out->getResourceBuffer());
    REQUIRE(px);
    const std::size_t centre = ((H / 2) * W + W / 2) * 4;
    REQUIRE(px[centre + 3] == 255);
}

TEST_CASE("Render: Hlsl Renderer BindsRealPointLight", "[renderhlsl]")
{
    // Wires a real MaterialX point_light through the renderer:
    //   - Generate a surface shader with point_light bound as light type 1.
    //   - Stand up a LightHandler with one synthesised point_light node
    //     and identity light transform.
    //   - Render. The per-light binding pass walks light sources, looks
    //     up u_lightData[0].type / .position / .color / .intensity /
    //     .decay_rate via reflection, and writes them into pixelCB
    //     through HlslMaterial::setCbufferRange.
    //   - Verify the framebuffer has alpha=255 (surface shader ran).
    constexpr unsigned int W = 32;
    constexpr unsigned int H = 32;
    mx::HlslRendererPtr renderer = tryCreateRenderer(W, H);
    if (!renderer)
    {
        WARN("HlslRenderer could not be initialised; skipping point-light test.");
        return;
    }

    mx::CameraPtr camera = mx::Camera::create();
    camera->setWorldMatrix(mx::Matrix44::IDENTITY);
    camera->setViewMatrix(mx::Matrix44::IDENTITY);
    camera->setProjectionMatrix(mx::Matrix44::IDENTITY);
    renderer->setCamera(camera);

    mx::ImageHandlerPtr ih = mx::ImageHandler::create(mx::StbImageLoader::create());
    renderer->setImageHandler(ih);

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

    // Bind the point_light shader so the generated surface code emits a
    // dispatch case for it. Light type id 1 is what computeLightIdMap
    // will assign to the only point_light below.
    mx::NodeDefPtr pointLightDef = libraries->getNodeDef("ND_point_light");
    REQUIRE(pointLightDef);
    mx::HwShaderGenerator::bindLightShader(*pointLightDef, 1, genContext);

    mx::ShaderPtr shader = genContext.getShaderGenerator().generate(element->getName(), element, genContext);
    REQUIRE(shader);

    mx::LightHandlerPtr lh = mx::LightHandler::create();
    lh->setEnvLightIntensity(0.0f);
    {
        // Synthesise a single point_light node directly on the document
        // so LightHandler::registerLights can resolve its NodeDef.
        mx::NodePtr light = doc->addNode("point_light", "test_point_light", "lightshader");
        light->setInputValue("position",   mx::Vector3(0.0f, 0.0f, 1.0f));
        light->setInputValue("color",      mx::Color3(1.0f, 0.5f, 0.25f));
        light->setInputValue("intensity",  1.0f);
        light->setInputValue("decay_rate", 0.0f);
        std::vector<mx::NodePtr> lights = { light };
        lh->registerLights(doc, lights, genContext);
        lh->setLightSources(lights);
    }
    renderer->setLightHandler(lh);

    REQUIRE_NOTHROW(renderer->createProgram(shader));
    REQUIRE_NOTHROW(renderer->render());

    mx::ImagePtr out = renderer->captureImage();
    REQUIRE(out);
    auto* px = static_cast<const uint8_t*>(out->getResourceBuffer());
    REQUIRE(px);
    const std::size_t centre = ((H / 2) * W + W / 2) * 4;
    REQUIRE(px[centre + 3] == 255);
}

TEST_CASE("Render: Hlsl Renderer WithBindingContext", "[renderhlsl]")
{
    // Verifies the renderer's auto-binding paths still find every
    // uniform when an HlslResourceBindingContext is attached. With the
    // binding context, the generator splits uniforms across separate
    // PrivateUniforms_pixel / PublicUniforms_pixel / LightData_pixel
    // cbuffers instead of packing them into a single pixelCB. The
    // renderer must locate each uniform regardless of which cbuffer
    // owns it (HlslMaterial::patchVariable handles the search).
    constexpr unsigned int W = 32;
    constexpr unsigned int H = 32;
    mx::HlslRendererPtr renderer = tryCreateRenderer(W, H);
    if (!renderer)
    {
        WARN("HlslRenderer could not be initialised; skipping binding-context renderer test.");
        return;
    }

    mx::CameraPtr camera = mx::Camera::create();
    camera->setWorldMatrix(mx::Matrix44::IDENTITY);
    camera->setViewMatrix(mx::Matrix44::IDENTITY);
    camera->setProjectionMatrix(mx::Matrix44::IDENTITY);
    renderer->setCamera(camera);

    mx::ImageHandlerPtr ih = mx::ImageHandler::create(mx::StbImageLoader::create());
    renderer->setImageHandler(ih);
    renderer->setLightHandler(mx::LightHandler::create());

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::FilePath testFile = searchPath.find("resources/Materials/Examples/StandardSurface/standard_surface_carpaint.mtlx");
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, testFile);
    doc->setDataLibrary(libraries);

    mx::ElementPtr element = doc->getChild("Car_Paint");
    REQUIRE(element);

    // Activate the binding context BEFORE generating the shader so the
    // generator splits the cbuffers.
    mx::GenContext genContext(mx::HlslShaderGenerator::create());
    genContext.registerSourceCodeSearchPath(searchPath);
    genContext.getShaderGenerator().registerTypeDefs(doc);
    genContext.pushUserData(mx::HW::USER_DATA_BINDING_CONTEXT,
                            mx::HlslResourceBindingContext::create(0, 0, 0));

    mx::ShaderPtr shader = genContext.getShaderGenerator().generate(element->getName(), element, genContext);
    REQUIRE(shader);

    REQUIRE_NOTHROW(renderer->createProgram(shader));

    // Confirm the generator actually emitted multiple cbuffers in this
    // mode (not a single pixelCB), so we know patchVariable is being
    // exercised across them.
    mx::HlslMaterialPtr mat = renderer->getMaterial();
    REQUIRE(mat);
    int psCbufferCount = 0;
    for (const auto& b : mat->getPixelBindings())
    {
        if (b.type == mx::HlslResourceType::CBuffer)
            ++psCbufferCount;
    }
    REQUIRE(psCbufferCount >= 2);

    REQUIRE_NOTHROW(renderer->render());

    mx::ImagePtr out = renderer->captureImage();
    REQUIRE(out);
    auto* px = static_cast<const uint8_t*>(out->getResourceBuffer());
    REQUIRE(px);
    const std::size_t centre = ((H / 2) * W + W / 2) * 4;
    REQUIRE(px[centre + 3] == 255);
}

TEST_CASE("Render: Hlsl Renderer DrawsMeshWhenAvailable", "[renderhlsl]")
{
    constexpr unsigned int W = 32;
    constexpr unsigned int H = 32;
    mx::HlslRendererPtr renderer = tryCreateRenderer(W, H);
    if (!renderer)
    {
        WARN("HlslRenderer could not be initialised; skipping mesh-render test.");
        return;
    }

    // Load resources/Geometry/plane.obj - a textbook flat plane that
    // GeometryHandler will parse via TinyObjLoader. The flat plane has
    // POSITION + NORMAL + TANGENT streams, which is exactly the input
    // layout HlslRenderer expects.
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::GeometryHandlerPtr geom = mx::GeometryHandler::create();
    geom->addLoader(mx::TinyObjLoader::create());
    REQUIRE(geom->loadGeometry(searchPath.find("resources/Geometry/plane.obj")));
    REQUIRE_FALSE(geom->getMeshes().empty());
    renderer->setGeometryHandler(geom);

    // Identity camera so the quad's built-in NDC positions pass through.
    mx::CameraPtr camera = mx::Camera::create();
    camera->setWorldMatrix(mx::Matrix44::IDENTITY);
    camera->setViewMatrix(mx::Matrix44::IDENTITY);
    camera->setProjectionMatrix(mx::Matrix44::IDENTITY);
    renderer->setCamera(camera);
    renderer->setScreenColor(mx::Color4(0.0f, 0.0f, 0.0f, 0.0f));

    // Trivial shader pair: PS emits a fixed color so we can detect that
    // the mesh path actually rasterised pixels.
    mx::ShaderRenderer::StageMap stages;
    stages[mx::Stage::VERTEX] = R"(
        struct VsIn  { float3 pos : POSITION; };
        struct VsOut { float4 pos : SV_Position; };
        VsOut VSMain(VsIn vin)
        {
            VsOut o;
            o.pos = float4(vin.pos, 1.0);
            return o;
        }
    )";
    stages[mx::Stage::PIXEL] = R"(
        struct VsOut { float4 pos : SV_Position; };
        float4 PSMain(VsOut pin) : SV_Target
        {
            return float4(0.4, 0.7, 0.2, 1.0);
        }
    )";
    REQUIRE_NOTHROW(renderer->createProgram(stages));
    // Test asserts raw PS-output bytes (0.4 * 255 = 102 etc.); opt out
    // of the framebuffer's default sRGB encoding so the linear RTV is
    // bound for this draw.
    renderer->getFramebuffer()->setEncodeSrgb(false);
    REQUIRE_NOTHROW(renderer->render());

    mx::ImagePtr out = renderer->captureImage();
    REQUIRE(out);
    auto* px = static_cast<const uint8_t*>(out->getResourceBuffer());
    REQUIRE(px);

    // Centre pixel must come from the PS, not the clear (clear was
    // alpha=0; PS emits alpha=1).
    const std::size_t c = ((H / 2) * W + W / 2) * 4;
    auto approxEq = [](unsigned int a, unsigned int b) {
        return (a > b ? a - b : b - a) <= 1;
    };
    REQUIRE(approxEq(px[c + 0], 102));   // 0.4 * 255 = 102
    REQUIRE(approxEq(px[c + 1], 178));   // 0.7 * 255 = 178.5
    REQUIRE(approxEq(px[c + 2], 51));    // 0.2 * 255 = 51
    REQUIRE(px[c + 3] == 255);
}
