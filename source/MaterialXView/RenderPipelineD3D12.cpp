//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXView/RenderPipelineD3D12.h>
#include <MaterialXView/Viewer.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/Util.h>
#include <MaterialXRenderHlsl/TextureBaker.h>

#include <nanogui/messagedialog.h>
#include <nanogui/opengl.h>

namespace
{

const float PI = std::acos(-1.0f);


mx::ImageSamplingProperties createSamplingProperties(mx::ImageSamplingProperties::AddressMode uMode,
                                                     mx::ImageSamplingProperties::AddressMode vMode,
                                                     mx::ImageSamplingProperties::FilterType filter)
{
    mx::ImageSamplingProperties sp;
    sp.uaddressMode = uMode;
    sp.vaddressMode = vMode;
    sp.filterType = filter;
    return sp;
}

} // anonymous namespace

D3D12RenderPipeline::D3D12RenderPipeline(Viewer* viewerPtr) :
    RenderPipeline(viewerPtr),
    _glTexture(0),
    _glFramebuffer(0),
    _glTextureWidth(0),
    _glTextureHeight(0)
{
}

D3D12RenderPipeline::~D3D12RenderPipeline()
{
    if (_glFramebuffer)
    {
        glDeleteFramebuffers(1, &_glFramebuffer);
    }
    if (_glTexture)
    {
        glDeleteTextures(1, &_glTexture);
    }
}

void D3D12RenderPipeline::initialize(void*, void*)
{
    if (!_context)
    {
        _context = mx::HlslD3D12Context::create();
        _geometryCache = mx::HlslD3D12GeometryCache::create(_context);
    }
}

void D3D12RenderPipeline::initFramebuffer(int, int, void*)
{
}

void D3D12RenderPipeline::resizeFramebuffer(int, int, void*)
{
}

mx::ImageHandlerPtr D3D12RenderPipeline::createImageHandler()
{
    initialize(nullptr, nullptr);
    return mx::HlslD3D12TextureHandler::create(_context, mx::StbImageLoader::create());
}

mx::MaterialPtr D3D12RenderPipeline::createMaterial()
{
    initialize(nullptr, nullptr);
    return mx::HlslD3D12ShaderMaterial::create(_context, _geometryCache);
}

std::shared_ptr<void> D3D12RenderPipeline::createTextureBaker(unsigned int width,
                                                              unsigned int height,
                                                              mx::Image::BaseType baseType)
{
    return std::static_pointer_cast<void>(mx::TextureBakerHlslD3D12::create(width, height, baseType));
}

mx::HlslD3D12TextureHandlerPtr D3D12RenderPipeline::getTextureHandler() const
{
    return std::dynamic_pointer_cast<mx::HlslD3D12TextureHandler>(_viewer->_imageHandler);
}

mx::HlslD3D12ShaderMaterialPtr D3D12RenderPipeline::createSupportMaterial(mx::ShaderPtr hwShader)
{
    mx::HlslD3D12ShaderMaterialPtr material = mx::HlslD3D12ShaderMaterial::create(_context, _geometryCache);
    material->generateShader(hwShader);
    material->bindShader();
    return material;
}

mx::ImagePtr D3D12RenderPipeline::renderQuad(mx::HlslD3D12ShaderMaterialPtr material,
                                             mx::HlslD3D12FramebufferPtr framebuffer,
                                             const mx::Color4& clearColor)
{
    ID3D12GraphicsCommandList* commandList = _context->beginFrame();
    try
    {
        framebuffer->bind(commandList);
        framebuffer->clear(commandList, clearColor);
        mx::HlslD3D12RenderState state;
        state.depthTest = false;
        state.depthWrite = false;
        _context->setRenderState(state);
        _viewer->renderScreenSpaceQuad(material);
    }
    catch (...)
    {
        _context->endFrame();
        throw;
    }
    _context->endFrame();
    return framebuffer->readColor();
}

void D3D12RenderPipeline::updateAlbedoTable(int tableSize)
{
    auto& genContext    = _viewer->_genContext;
    auto& stdLib        = _viewer->_stdLib;
    auto& lightHandler  = _viewer->_lightHandler;
    auto& imageHandler  = _viewer->_imageHandler;

    if (lightHandler->getAlbedoTable())
    {
        return;
    }

    mx::HlslD3D12ShaderMaterialPtr material;
    try
    {
        material = createSupportMaterial(mx::createAlbedoTableShader(genContext, stdLib, "__ALBEDO_TABLE_SHADER__"));
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(_viewer, ng::MessageDialog::Type::Warning, "Failed to generate albedo table shader", e.what());
        return;
    }
    material->bindUniform(mx::HW::ALBEDO_TABLE_SIZE, mx::Value::createValue(tableSize));

    mx::HlslD3D12FramebufferPtr framebuffer =
        mx::HlslD3D12Framebuffer::create(_context, tableSize, tableSize, mx::Image::BaseType::FLOAT);
    mx::ImagePtr table = renderQuad(material, framebuffer, mx::Color4(1.0f));

    imageHandler->releaseRenderResources(lightHandler->getAlbedoTable());
    lightHandler->setAlbedoTable(table);
    if (_viewer->_saveGeneratedLights)
    {
        imageHandler->saveImage("AlbedoTable.exr", lightHandler->getAlbedoTable());
    }
}

void D3D12RenderPipeline::updatePrefilteredMap()
{
    auto& genContext    = _viewer->_genContext;
    auto& lightHandler  = _viewer->_lightHandler;

    if (lightHandler->getEnvPrefilteredMap())
    {
        return;
    }

    mx::ImagePtr srcTex = lightHandler->getEnvRadianceMap();
    mx::HlslD3D12TextureHandlerPtr textureHandler = getTextureHandler();
    if (!srcTex || !textureHandler)
    {
        return;
    }

    mx::HlslD3D12ShaderMaterialPtr material;
    try
    {
        material = createSupportMaterial(mx::createEnvPrefilterShader(genContext, _viewer->_stdLib, "__ENV_PREFILTER__"));
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(_viewer, ng::MessageDialog::Type::Warning, "Failed to generate prefilter shader", e.what());
        return;
    }

    // Render each mip level of the prefiltered environment into its own
    // framebuffer, then upload the levels as one mip chain.
    const mx::ImageSamplingProperties envSampling =
        createSamplingProperties(mx::ImageSamplingProperties::AddressMode::PERIODIC,
                                 mx::ImageSamplingProperties::AddressMode::CLAMP,
                                 mx::ImageSamplingProperties::FilterType::LINEAR);
    material->bindTexture(mx::HW::ENV_RADIANCE, srcTex, textureHandler, envSampling);
    material->bindUniform(mx::HW::ENV_MATRIX, mx::Value::createValue(mx::Matrix44::createScale(mx::Vector3(-1, 1, -1))));
    material->bindUniform(mx::HW::ENV_RADIANCE_MIPS, mx::Value::createValue<int>(srcTex->getMaxMipCount()));

    std::vector<mx::ImagePtr> levels;
    try
    {
        unsigned int w = srcTex->getWidth();
        unsigned int h = srcTex->getHeight();
        for (int mip = 0; w > 0 && h > 0; mip++, w /= 2, h /= 2)
        {
            material->bindUniform(mx::HW::ENV_PREFILTER_MIP, mx::Value::createValue(mip));
            mx::HlslD3D12FramebufferPtr framebuffer =
                mx::HlslD3D12Framebuffer::create(_context, w, h, mx::Image::BaseType::HALF);
            levels.push_back(renderQuad(material, framebuffer, mx::Color4(0.0f)));
        }
    }
    catch (mx::ExceptionRenderError& e)
    {
        for (const std::string& error : e.errorLog())
        {
            std::cerr << error << std::endl;
        }
        new ng::MessageDialog(_viewer, ng::MessageDialog::Type::Warning, "Failed to render prefiltered environment", e.what());
        return;
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(_viewer, ng::MessageDialog::Type::Warning, "Failed to render prefiltered environment", e.what());
        return;
    }

    // The base level identifies the prefiltered texture for later binds.
    mx::ImagePtr outTex = levels[0];
    textureHandler->bindImageWithMips(outTex, levels, envSampling);
    lightHandler->setEnvPrefilteredMap(outTex);
}

mx::ImagePtr D3D12RenderPipeline::getShadowMap(int shadowMapSize)
{
    auto& genContext      = _viewer->_genContext;
    auto& imageHandler    = _viewer->_imageHandler;
    auto& shadowCamera    = _viewer->_shadowCamera;
    auto& stdLib          = _viewer->_stdLib;
    auto& geometryHandler = _viewer->_geometryHandler;

    if (!_viewer->_shadowMap)
    {
        // Generate shaders for shadow rendering.
        if (!_viewer->_shadowMaterial)
        {
            try
            {
                _viewer->_shadowMaterial = createSupportMaterial(mx::createDepthShader(genContext, stdLib, "__SHADOW_SHADER__"));
            }
            catch (std::exception& e)
            {
                std::cerr << "Failed to generate shadow shader: " << e.what() << std::endl;
                _viewer->_shadowMaterial = nullptr;
            }
        }
        if (!_viewer->_shadowBlurMaterial)
        {
            try
            {
                _viewer->_shadowBlurMaterial = createSupportMaterial(mx::createBlurShader(genContext, stdLib, "__SHADOW_BLUR_SHADER__", "gaussian", 1.0f));
            }
            catch (std::exception& e)
            {
                std::cerr << "Failed to generate shadow blur shader: " << e.what() << std::endl;
                _viewer->_shadowBlurMaterial = nullptr;
            }
        }

        mx::HlslD3D12TextureHandlerPtr textureHandler = getTextureHandler();
        if (_viewer->_shadowMaterial && _viewer->_shadowBlurMaterial && textureHandler)
        {
            mx::HlslD3D12FramebufferPtr framebuffer =
                mx::HlslD3D12Framebuffer::create(_context, shadowMapSize, shadowMapSize, mx::Image::BaseType::FLOAT);

            // Render shadow geometry.
            ID3D12GraphicsCommandList* commandList = _context->beginFrame();
            try
            {
                framebuffer->bind(commandList);
                framebuffer->clear(commandList, mx::Color4(1.0f));
                mx::HlslD3D12RenderState state;
                state.depthLessEqual = true;
                _context->setRenderState(state);
                _viewer->_shadowMaterial->bindShader();
                for (auto mesh : geometryHandler->getMeshes())
                {
                    _viewer->_shadowMaterial->bindMesh(mesh);
                    _viewer->_shadowMaterial->bindViewInformation(shadowCamera);
                    for (size_t i = 0; i < mesh->getPartitionCount(); i++)
                    {
                        _viewer->_shadowMaterial->drawPartition(mesh->getPartition(i));
                    }
                }
            }
            catch (...)
            {
                _context->endFrame();
                throw;
            }
            _context->endFrame();
            _viewer->_shadowMap = framebuffer->readColor();

            // Apply Gaussian blurring.
            const mx::ImageSamplingProperties blurSampling =
                createSamplingProperties(mx::ImageSamplingProperties::AddressMode::CLAMP,
                                         mx::ImageSamplingProperties::AddressMode::CLAMP,
                                         mx::ImageSamplingProperties::FilterType::CLOSEST);
            auto blurMaterial = std::static_pointer_cast<mx::HlslD3D12ShaderMaterial>(_viewer->_shadowBlurMaterial);
            for (unsigned int i = 0; i < _viewer->_shadowSoftness; i++)
            {
                blurMaterial->bindTexture("image_file", _viewer->_shadowMap, textureHandler, blurSampling);
                mx::ImagePtr blurred = renderQuad(blurMaterial, framebuffer, mx::Color4(1.0f));
                imageHandler->releaseRenderResources(_viewer->_shadowMap);
                _viewer->_shadowMap = blurred;
            }
        }

        // Reset frame timing after shadow generation.
        _viewer->resetFrameTiming();
    }

    return _viewer->_shadowMap;
}

void D3D12RenderPipeline::renderFrame(void*, int shadowMapSize, const char* dirLightNodeCat)
{
    auto& genContext      = _viewer->_genContext;
    auto& lightHandler    = _viewer->_lightHandler;
    auto& imageHandler    = _viewer->_imageHandler;
    auto& viewCamera      = _viewer->_viewCamera;
    auto& envCamera       = _viewer->_envCamera;
    auto& shadowCamera    = _viewer->_shadowCamera;
    float lightRotation   = _viewer->_lightRotation;
    auto& searchPath      = _viewer->_searchPath;
    auto& geometryHandler = _viewer->_geometryHandler;

    // Update prefiltered environment.
    if (lightHandler->getUsePrefilteredMap() && !_viewer->_materialAssignments.empty())
    {
        updatePrefilteredMap();
    }

    // Update lighting state.
    lightHandler->setLightTransform(mx::Matrix44::createRotationY(lightRotation / 180.0f * PI));

    // Update shadow state.
    mx::ShadowState shadowState;
    shadowState.ambientOcclusionGain = _viewer->_ambientOcclusionGain;
    mx::NodePtr dirLight = lightHandler->getFirstLightOfCategory(dirLightNodeCat);
    if (genContext.getOptions().hwShadowMap && dirLight)
    {
        mx::ImagePtr shadowMap = getShadowMap(shadowMapSize);
        if (shadowMap)
        {
            shadowState.shadowMap = shadowMap;
            shadowState.shadowMatrix = viewCamera->getWorldMatrix().getInverse() *
                shadowCamera->getWorldViewProjMatrix();
        }
        else
        {
            genContext.getOptions().hwShadowMap = false;
        }
    }

    // Match the framebuffer to the window's pixel size.
    const unsigned int width = (unsigned int) std::max(1, _viewer->m_fbsize[0]);
    const unsigned int height = (unsigned int) std::max(1, _viewer->m_fbsize[1]);
    if (!_framebuffer || _framebuffer->getWidth() != width || _framebuffer->getHeight() != height)
    {
        _framebuffer = mx::HlslD3D12Framebuffer::create(_context, width, height);
        _frameImage = nullptr;
    }

    // The OpenGL path clears with the sRGB background color while sRGB
    // encoding is disabled; linearize it so that the sRGB-encoding target
    // stores the same values.
    const ng::Color& background = _viewer->background();
    const mx::Color3 clearColor = mx::Color3(background.r(), background.g(), background.b()).srgbToLinear();

    ID3D12GraphicsCommandList* commandList = _context->beginFrame();
    try
    {
        _framebuffer->bind(commandList);
        _framebuffer->clear(commandList, mx::Color4(clearColor[0], clearColor[1], clearColor[2], 1.0f));

        mx::HlslD3D12RenderState state;
        state.depthLessEqual = true;

        // Environment background
        if (_viewer->_drawEnvironment)
        {
            mx::MaterialPtr envMaterial = _viewer->getEnvironmentMaterial();
            if (envMaterial)
            {
                const mx::MeshList& meshes = _viewer->_envGeometryHandler->getMeshes();
                mx::MeshPartitionPtr envPart = !meshes.empty() ? meshes[0]->getPartition(0) : nullptr;
                if (envPart)
                {
                    // Apply rotation and light intensity to the environment shader.
                    envMaterial->modifyUniform("envImage/rotation", mx::Value::createValue(lightRotation));
                    envMaterial->modifyUniform("envImageAdjusted/in2", mx::Value::createValue(lightHandler->getEnvLightIntensity()));

                    // Render the environment mesh without writing depth.
                    mx::HlslD3D12RenderState envState = state;
                    envState.depthWrite = false;
                    _context->setRenderState(envState);
                    envMaterial->bindShader();
                    envMaterial->bindMesh(meshes[0]);
                    envMaterial->bindViewInformation(envCamera);
                    envMaterial->bindImages(imageHandler, searchPath, false);
                    envMaterial->drawPartition(envPart);
                }
            }
            else
            {
                _viewer->_drawEnvironment = false;
            }
        }

        // Enable backface culling if requested.
        state.cullBackFaces = !_viewer->_renderDoubleSided;

        // Opaque pass
        _context->setRenderState(state);
        for (const auto& assignment : _viewer->_materialAssignments)
        {
            mx::MeshPartitionPtr geom = assignment.first;
            auto material = std::dynamic_pointer_cast<mx::HlslD3D12ShaderMaterial>(assignment.second);
            shadowState.ambientOcclusionMap = _viewer->getAmbientOcclusionImage(material);
            if (!material)
            {
                continue;
            }

            material->bindShader();
            material->bindMesh(geometryHandler->findParentMesh(geom));
            material->bindUniform(mx::HW::ALPHA_THRESHOLD, mx::Value::createValue(0.99f));
            material->bindTimeAndFrame((float) _timer.elapsedTime(), (float) _frame);
            material->bindViewInformation(viewCamera);
            material->bindLighting(lightHandler, imageHandler, shadowState);
            material->bindImages(imageHandler, searchPath);
            material->drawPartition(geom);
            material->unbindImages(imageHandler);
        }

        // Transparent pass
        if (_viewer->_renderTransparency)
        {
            mx::HlslD3D12RenderState blendState = state;
            blendState.blend = true;
            _context->setRenderState(blendState);
            for (const auto& assignment : _viewer->_materialAssignments)
            {
                mx::MeshPartitionPtr geom = assignment.first;
                auto material = std::dynamic_pointer_cast<mx::HlslD3D12ShaderMaterial>(assignment.second);
                shadowState.ambientOcclusionMap = _viewer->getAmbientOcclusionImage(material);
                if (!material || !material->hasTransparency())
                {
                    continue;
                }

                material->bindShader();
                material->bindMesh(geometryHandler->findParentMesh(geom));
                material->bindUniform(mx::HW::ALPHA_THRESHOLD, mx::Value::createValue(0.001f));
                material->bindTimeAndFrame((float) _timer.elapsedTime(), (float) _frame);
                material->bindViewInformation(viewCamera);
                material->bindLighting(lightHandler, imageHandler, shadowState);
                material->bindImages(imageHandler, searchPath);
                material->drawPartition(geom);
                material->unbindImages(imageHandler);
            }
        }

        // Wireframe pass
        if (_viewer->_outlineSelection)
        {
            mx::MaterialPtr wireMaterial = _viewer->getWireframeMaterial();
            if (wireMaterial)
            {
                mx::HlslD3D12RenderState wireState;
                wireState.depthLessEqual = true;
                wireState.wireframe = true;
                _context->setRenderState(wireState);
                wireMaterial->bindShader();
                wireMaterial->bindMesh(geometryHandler->findParentMesh(_viewer->getSelectedGeometry()));
                wireMaterial->bindViewInformation(viewCamera);
                wireMaterial->drawPartition(_viewer->getSelectedGeometry());
            }
            else
            {
                _viewer->_outlineSelection = false;
            }
        }
    }
    catch (...)
    {
        _context->endFrame();
        throw;
    }
    _context->endFrame();

    _frameImage = _framebuffer->readColor(_frameImage);
    presentToWindow(_frameImage);
}

void D3D12RenderPipeline::presentToWindow(mx::ImagePtr image)
{
    if (!image)
    {
        return;
    }
    const int width = (int) image->getWidth();
    const int height = (int) image->getHeight();

    if (!_glTexture)
    {
        glGenTextures(1, &_glTexture);
        glGenFramebuffers(1, &_glFramebuffer);
    }
    glBindTexture(GL_TEXTURE_2D, _glTexture);
    if (width != _glTextureWidth || height != _glTextureHeight)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        _glTextureWidth = width;
        _glTextureHeight = height;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image->getResourceBuffer());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);

    // The first row of the D3D12 image is the top of the frame, while the
    // first row of an OpenGL texture is its bottom, so the copy flips
    // vertically.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _glFramebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _glTexture, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, height, width, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void D3D12RenderPipeline::bakeTextures()
{
    auto& imageHandler = _viewer->_imageHandler;

    mx::MaterialPtr material = _viewer->getSelectedMaterial();
    mx::DocumentPtr doc = material ? material->getDocument() : nullptr;
    if (!doc)
    {
        return;
    }

    {
        // Construct a texture baker.
        mx::Image::BaseType baseType = _viewer->_bakeHdr ? mx::Image::BaseType::FLOAT : mx::Image::BaseType::UINT8;
        mx::UnsignedIntPair bakingRes = _viewer->computeBakingResolution(doc);
        mx::TextureBakerHlslD3D12Ptr baker = std::static_pointer_cast<mx::TextureBakerHlslD3D12>(
            createTextureBaker(bakingRes.first, bakingRes.second, baseType));
        baker->setupUnitSystem(_viewer->_stdLib);
        baker->setDistanceUnit(_viewer->_genContext.getOptions().targetDistanceUnit);
        baker->setAverageImages(_viewer->_bakeAverage);
        baker->setOptimizeConstants(_viewer->_bakeOptimize);
        baker->writeDocumentPerMaterial(_viewer->_bakeDocumentPerMaterial);

        // Share the viewer's image handler for image loading; the baker
        // uploads through its own device.
        imageHandler->releaseRenderResources();
        baker->setImageHandler(imageHandler);

        // Extend the image search path to include material source folders.
        mx::FileSearchPath extendedSearchPath = _viewer->_searchPath;
        extendedSearchPath.append(_viewer->_materialSearchPath);

        // Bake all materials in the active document.
        try
        {
            baker->bakeAllMaterials(doc, extendedSearchPath, _viewer->_bakeFilename);
        }
        catch (std::exception& e)
        {
            std::cerr << "Error in texture baking: " << e.what() << std::endl;
        }

        // Release any render resources generated by the baking process.
        imageHandler->releaseRenderResources();
    }

    // The prefiltered environment was uploaded with rendered mip levels,
    // which releasing the render resources discarded; regenerate it.
    _viewer->_lightHandler->setEnvPrefilteredMap(nullptr);
}

mx::ImagePtr D3D12RenderPipeline::getFrameImage()
{
    // Return the frame rendered by D3D12 rather than reading the back
    // buffer, so the capture does not depend on OpenGL pack state. Rows are
    // stored bottom-up, matching the OpenGL pipeline, since the viewer
    // saves frame images with a vertical flip.
    if (!_frameImage)
    {
        return nullptr;
    }
    const unsigned int width = _frameImage->getWidth();
    const unsigned int height = _frameImage->getHeight();
    mx::ImagePtr image = mx::Image::create(width, height, 3);
    image->createResourceBuffer();

    const uint8_t* src = static_cast<const uint8_t*>(_frameImage->getResourceBuffer());
    uint8_t* dst = static_cast<uint8_t*>(image->getResourceBuffer());
    for (unsigned int y = 0; y < height; y++)
    {
        const uint8_t* srcRow = src + (size_t) y * width * 4;
        uint8_t* dstRow = dst + (size_t) (height - 1 - y) * width * 3;
        for (unsigned int x = 0; x < width; x++)
        {
            dstRow[x * 3 + 0] = srcRow[x * 4 + 0];
            dstRow[x * 3 + 1] = srcRow[x * 4 + 1];
            dstRow[x * 3 + 2] = srcRow[x * 4 + 2];
        }
    }
    return image;
}
