//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderMsl/MslRenderer.h>
#include <MaterialXRenderHw/SimpleWindow.h>
#include <MaterialXRender/TinyObjLoader.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

#include <iostream>

#import <Metal/Metal.h>

MATERIALX_NAMESPACE_BEGIN

const float PI = std::acos(-1.0f);

// View information
const float FOV_PERSP = 45.0f; // degrees
const float NEAR_PLANE_PERSP = 0.05f;
const float FAR_PLANE_PERSP = 100.0f;

//
// MslRenderer methods
//

MslRendererPtr MslRenderer::create(unsigned int width, unsigned int height, Image::BaseType baseType)
{
    return MslRendererPtr(new MslRenderer(width, height, baseType));
}

id<MTLDevice> MslRenderer::getMetalDevice() const
{
    return _device;
}

MslRenderer::MslRenderer(unsigned int width, unsigned int height, Image::BaseType baseType) :
    ShaderRenderer(width, height, baseType),
    _initialized(false),
    _eye(0.0f, 0.0f, 3.0f),
    _center(0.0f, 0.0f, 0.0f),
    _up(0.0f, 1.0f, 0.0f),
    _objectScale(1.0f),
    _screenColor(DEFAULT_SCREEN_COLOR_LIN_REC709)
{
    _program = MslProgram::create();

    _geometryHandler = GeometryHandler::create();
    _geometryHandler->addLoader(TinyObjLoader::create());

    _camera = Camera::create();
}

void MslRenderer::initialize(RenderContextHandle)
{
    if (!_initialized)
    {
        // Create window
        _window = SimpleWindow::create();

        if (!_window->initialize("Renderer Window", _width, _height, nullptr))
        {
            throw ExceptionRenderError("Failed to initialize renderer window");
        }

        _device = MTLCreateSystemDefaultDevice();
        _cmdQueue = [_device newCommandQueue];
        createFrameBuffer(true);
        
        _initialized = true;
    }
}

void MslRenderer::createProgram(ShaderPtr shader)
{
    _program = MslProgram::create();
    _program->setStages(shader);
    _program->build(_device, _framebuffer);
}

void MslRenderer::createProgram(const StageMap& stages)
{
    for (const auto& it : stages)
    {
        _program->addStage(it.first, it.second);
    }
    _program->build(_device, _framebuffer);
}

void MslRenderer::renderTextureSpace(const Vector2& uvMin, const Vector2& uvMax)
{
    bool captureRenderTextureSpace = false;
    if(captureRenderTextureSpace)
        triggerProgrammaticCapture();
    
    MTLRenderPassDescriptor* desc = [MTLRenderPassDescriptor new];
    _framebuffer->bind(desc);
    
    _cmdBuffer = [_cmdQueue commandBuffer];
    
    id<MTLRenderCommandEncoder> rendercmdEncoder = [_cmdBuffer renderCommandEncoderWithDescriptor:desc];
    _program->bind(rendercmdEncoder);
    _program->prepareUsedResources(rendercmdEncoder,
                         _camera,
                         _geometryHandler,
                         _imageHandler,
                         _lightHandler);
    
    MeshPtr mesh = _geometryHandler->createQuadMesh(uvMin, uvMax, true);
    _program->bindMesh(rendercmdEncoder, mesh);
    MeshPartitionPtr part = mesh->getPartition(0);
    _program->bindPartition(part);
    MeshIndexBuffer& indexData = part->getIndices();
    [rendercmdEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                 indexCount:indexData.size()
                                  indexType:MTLIndexTypeUInt32
                                indexBuffer:_program->getIndexBuffer(part)
                          indexBufferOffset:0];
    
    _framebuffer->unbind();
    [rendercmdEncoder endEncoding];
    
    [_cmdBuffer commit];
    [_cmdBuffer waitUntilCompleted];
    
    [desc release];
    
    if(captureRenderTextureSpace)
        stopProgrammaticCapture();
}

void MslRenderer::validateInputs()
{
    // Check that the generated uniforms and attributes are valid
    _program->getUniformsList();
    _program->getAttributesList();
}

void MslRenderer::createFrameBuffer(bool encodeSrgb)
{
    _framebuffer = MetalFramebuffer::create(_device,
                                            _width, _height, 4,
                                            _baseType,
                                            nil, encodeSrgb);
}

void MslRenderer::setSize(unsigned int width, unsigned int height)
{
    if (_framebuffer)
    {
        _framebuffer->resize(width, height);
    }
    else
    {
        _width = width;
        _height = height;
        createFrameBuffer(true);
    }
    
}

void MslRenderer::updateViewInformation()
{
    float fH = std::tan(FOV_PERSP / 360.0f * PI) * NEAR_PLANE_PERSP;
    float fW = fH * 1.0f;

    _camera->setViewMatrix(Camera::createViewMatrix(_eye, _center, _up));
    _camera->setProjectionMatrix(Camera::createPerspectiveMatrixZP(-fW, fW, -fH, fH, NEAR_PLANE_PERSP, FAR_PLANE_PERSP));
}

void MslRenderer::updateWorldInformation()
{
     _camera->setWorldMatrix(Matrix44::createScale(Vector3(_objectScale)));
}

void MslRenderer::triggerProgrammaticCapture()
{
    MTLCaptureManager*    captureManager    = [MTLCaptureManager sharedCaptureManager];
    MTLCaptureDescriptor* captureDescriptor = [MTLCaptureDescriptor new];
    captureDescriptor.captureObject = _device;
    
    NSError* error = nil;
    if(![captureManager startCaptureWithDescriptor:captureDescriptor error:&error])
    {
        NSLog(@"Failed to start capture, error %@", error);
    }
}

void MslRenderer::stopProgrammaticCapture()
{
    MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
    [captureManager stopCapture];
}

void MslRenderer::render()
{
    bool captureFrame = false;
    if(captureFrame)
        triggerProgrammaticCapture();
    
    _cmdBuffer = [_cmdQueue commandBuffer];
    MTLRenderPassDescriptor* renderpassDesc = [MTLRenderPassDescriptor new];
    
    _framebuffer->bind(renderpassDesc);
    [renderpassDesc.colorAttachments[0] setClearColor:
        MTLClearColorMake(_screenColor[0], _screenColor[1], _screenColor[2], 1.0f)];
    
    id<MTLRenderCommandEncoder> renderCmdEncoder = [_cmdBuffer renderCommandEncoderWithDescriptor:renderpassDesc];

    MTLDepthStencilDescriptor* depthStencilDesc = [MTLDepthStencilDescriptor new];
    depthStencilDesc.depthWriteEnabled    = !(_program->isTransparent());
    depthStencilDesc.depthCompareFunction = MTLCompareFunctionLess;
    
    id<MTLDepthStencilState> depthStencilState = [_device newDepthStencilStateWithDescriptor:depthStencilDesc];
    [renderCmdEncoder setDepthStencilState:depthStencilState];

    
    [renderCmdEncoder setCullMode:MTLCullModeBack];
    
    updateViewInformation();
    updateWorldInformation();

    try
    {
        // Bind program and input parameters
        if (_program)
        {
            // Bind the program to use
            _program->bind(renderCmdEncoder);
            _program->prepareUsedResources(renderCmdEncoder, _camera, _geometryHandler, _imageHandler, _lightHandler);

            // Draw all the partitions of all the meshes in the handler
            for (const auto& mesh : _geometryHandler->getMeshes())
            {
                _program->bindMesh(renderCmdEncoder, mesh);

                for (size_t i = 0; i < mesh->getPartitionCount(); i++)
                {
                    auto part = mesh->getPartition(i);
                    _program->bindPartition(part);
                    MeshIndexBuffer& indexData = part->getIndices();
                    
                    if(_program->isTransparent())
                    {
                        [renderCmdEncoder setCullMode:MTLCullModeFront];
                        [renderCmdEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:(int)indexData.size() indexType:MTLIndexTypeUInt32 indexBuffer:_program->getIndexBuffer(part) indexBufferOffset:0];
                        [renderCmdEncoder setCullMode:MTLCullModeBack];
                    }
                    
                    [renderCmdEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:(int)indexData.size() indexType:MTLIndexTypeUInt32 indexBuffer:_program->getIndexBuffer(part) indexBufferOffset:0];
                }
            }
        }
    }
    catch (ExceptionRenderError& e)
    {
        _framebuffer->unbind();
        throw e;
    }
    
    [renderCmdEncoder endEncoding];
    
    _framebuffer->unbind();
    
    [_cmdBuffer commit];
    [_cmdBuffer waitUntilCompleted];
    
    [_cmdBuffer release];
    _cmdBuffer = nil;
    
    if(captureFrame)
        stopProgrammaticCapture();
}

ImagePtr MslRenderer::captureImage(ImagePtr image)
{
    return _framebuffer->getColorImage(_cmdQueue, image);
}

MATERIALX_NAMESPACE_END
