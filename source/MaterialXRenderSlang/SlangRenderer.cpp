//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderSlang/SlangRenderer.h>
#include <MaterialXRenderSlang/SlangContext.h>
#include <MaterialXRenderSlang/SlangTextureHandler.h>
#include <MaterialXRenderSlang/SlangFramebuffer.h>
#include <MaterialXRenderSlang/SlangTypeUtils.h>

#include <MaterialXGenSlang/SlangShaderGenerator.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <MaterialXRenderHw/SimpleWindow.h>
#include <MaterialXRender/TinyObjLoader.h>
#include <MaterialXRender/CgltfLoader.h>

#include <fstream>
#include <iostream>
#include <sstream>

MATERIALX_NAMESPACE_BEGIN

//
// SlangRenderer methods
//

SlangRendererPtr SlangRenderer::create(unsigned int width, unsigned int height, Image::BaseType baseType)
{
    return std::shared_ptr<SlangRenderer>(new SlangRenderer(width, height, baseType));
}

SlangRenderer::SlangRenderer(unsigned int width, unsigned int height, Image::BaseType baseType) :
    ShaderRenderer(width, height, baseType),
    _initialized(false),
    _screenColor(DEFAULT_SCREEN_COLOR_LIN_REC709)
{
    _geometryHandler = GeometryHandler::create();
    _geometryHandler->addLoader(TinyObjLoader::create());
    _geometryHandler->addLoader(CgltfLoader::create());
}

void SlangRenderer::initialize(RenderContextHandle renderContextHandle)
{
    if (!_initialized)
    {
        _initialized = true;

        // Create window
        _window = SimpleWindow::create();

        if (!_window->initialize("Renderer Window", _width, _height, nullptr))
        {
            throw ExceptionRenderError("Failed to initialize renderer window");
        }

        if (renderContextHandle)
            _context = SlangContext::create((const char*)renderContextHandle);
        else
            _context = SlangContext::create();
        _program = SlangProgram::create(_context);

        createFrameBuffer(true);
    }
}

SlangRenderer::~SlangRenderer() = default;

void SlangRenderer::reset()
{
    _program = SlangProgram::create(_context);
    _initialized = false;
    initialize({});
}

ImageHandlerPtr SlangRenderer::createImageHandler(ImageLoaderPtr imageLoader)
{
    if (!_context)
        throw ExceptionRenderError("SlangRenderer must first be initialized.");
    return SlangTextureHandler::create(_context, imageLoader);
}

void SlangRenderer::createFrameBuffer(bool encodeSrgb)
{
    _framebuffer = SlangFramebuffer::create(_context,
                                            _width, _height, 4,
                                            _baseType,
                                            nullptr, encodeSrgb);
}

void SlangRenderer::setSize(unsigned int width, unsigned int height)
{
    _width = width;
    _height = height;
    if (_framebuffer)
    {
        _framebuffer->resize(width, height);
    }
    else
    {
        createFrameBuffer(true);
    }
}

void SlangRenderer::createProgram(ShaderPtr shader)
{
    _program->setStages(shader);
    _program->build();
}

void SlangRenderer::createProgram(const StageMap& stages)
{
    for (const auto& it : stages)
        _program->addStage(it.first, it.second);

    _program->build();
}

void SlangRenderer::validateInputs()
{
    _program->getUniformsList();
    _program->getVertexInputsList();
}

void SlangRenderer::render()
{
    using namespace rhi;

    if (!_program)
    {
        return;
    }

    if (_program->getVertexInputsList().empty())
    {
        throw ExceptionRenderError("Program has no input vertex data");
    }

    // Bind shader properties.
    _program->bindViewInformation(_camera);
    _program->bindTextures(_imageHandler);
    _program->bindLighting(_lightHandler, _imageHandler);
    _program->bindTimeAndFrame();

    // Set blend state for the given material.
    bool isTransparent = _program->isTransparent();

    SlangCommandEncoderPtr commandEncoder = _context->createCommandEncoder();
    SlangRenderPassDesc passDesc;
    _framebuffer->bindRenderPassDesc(passDesc);
    passDesc.colorAttachments[0].setClearValue(Color4(_screenColor[0], _screenColor[1], _screenColor[2], 1.f));

    SlangRenderPassEncoder* passEncoder = beginRenderPass(commandEncoder, passDesc);

    rhi::ComPtr<rhi::IRenderPipeline> pipeline0;
    rhi::ComPtr<rhi::IRenderPipeline> pipeline1;

    if (isTransparent)
    {
        pipeline0 = _program->getPipeline(SlangProgram::PipelineKind2::CullFront, _framebuffer);
        pipeline1 = _program->getPipeline(SlangProgram::PipelineKind2::Default, _framebuffer);
        // TODO: Metal is doing CullBack, while OpenGL is doing no cull
        // pipeline1 = _program->getPipeline(SlangProgram::PipelineKind2::CullBack, _framebuffer);
    }
    else
    {
        pipeline0 = _program->getPipeline(SlangProgram::PipelineKind2::Default, _framebuffer);
    }

    // Bind each mesh and draw its partitions.
    SlangRenderState renderState;
    _framebuffer->bindRenderState(renderState);
    _program->bind(passEncoder, &renderState);
    for (MeshPtr mesh : _geometryHandler->getMeshes())
    {
        _program->bindMesh(mesh);

        for (size_t i = 0; i < mesh->getPartitionCount(); ++i)
        {
            auto partition = mesh->getPartition(i);
            _program->bindPartition(partition);

            /// With transparency we have two pipelines (front and back),
            /// and have bind back, draw, bind front, draw, for every object.
            if (pipeline0)
            {
                passEncoder->bindPipeline(pipeline0, _program->getRootObject());
                _program->drawPartition(partition);
            }

            if (pipeline1)
            {
                passEncoder->bindPipeline(pipeline1, _program->getRootObject());
                _program->drawPartition(partition);
            }
        }
    }
    _program->unbind();

    passEncoder->end();
    _context->submitCommandEncoder(commandEncoder);
    _context->waitOnHost();
}

ImagePtr SlangRenderer::captureImage(ImagePtr image)
{
    return _framebuffer->getColorImage(image);
}

void SlangRenderer::renderTextureSpace(const Vector2& uvMin, const Vector2& uvMax)
{
    MeshPtr mesh = _geometryHandler->createQuadMesh(uvMin, uvMax, true);
    auto partition = mesh->getPartition(0);

    _program->bindViewInformation(_camera);
    _program->bindTextures(_imageHandler);
    _program->bindLighting(_lightHandler, _imageHandler);
    _program->bindTimeAndFrame();

    SlangCommandEncoderPtr commandEncoder = _context->createCommandEncoder();
    SlangRenderPassDesc passDesc;
    _framebuffer->bindRenderPassDesc(passDesc);
    passDesc.colorAttachments[0].setClearValue(Color4(_screenColor[0], _screenColor[1], _screenColor[2], 1.f));
    SlangRenderPassEncoder* passEncoder = beginRenderPass(commandEncoder, passDesc);

    auto pipeline = _program->getPipeline(SlangProgram::PipelineKind2::Default, _framebuffer);

    rhi::RenderState renderState = {};
    _framebuffer->bindRenderState(renderState);
    _program->bind(passEncoder, &renderState);

    _program->bindMesh(mesh);
    _program->bindPartition(partition);
    passEncoder->bindPipeline(pipeline, _program->getRootObject());
    _program->drawPartition(partition);

    passEncoder->end();
    _context->submitCommandEncoder(commandEncoder);
    _context->waitOnHost();
}

MATERIALX_NAMESPACE_END
