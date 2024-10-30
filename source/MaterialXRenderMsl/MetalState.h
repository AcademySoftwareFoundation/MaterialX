//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALXVIEW_METALSTATE_H
#define MATERIALXVIEW_METALSTATE_H

#include <stack>
#include <memory>
#include <mutex>
#include <condition_variable>

#import <Metal/Metal.h>

#include <MaterialXCore/Generated.h>
#include <MaterialXRenderMsl/Export.h>

MATERIALX_NAMESPACE_BEGIN
class MetalFramebuffer;
// Shared pointer to a MetalFramebuffer
using MetalFramebufferPtr = std::shared_ptr<class MetalFramebuffer>;

struct MX_RENDERMSL_API MetalState
{
    static MetalState* getSingleton()
    {
        if (!singleton)
        {
            singleton = std::unique_ptr<MetalState>(new MetalState());
        }
        return singleton.get();
    }

    MetalState();

    void initialize(id<MTLDevice> mtlDevice, id<MTLCommandQueue> mtlCmdQueue);
    void initLinearToSRGBKernel();
    void triggerProgrammaticCapture();
    void stopProgrammaticCapture();
    void beginCommandBuffer();
    void beginEncoder(MTLRenderPassDescriptor* renderpassDesc);
    void endEncoder();
    void endCommandBuffer();

    void waitForCompletion();

    MaterialX::MetalFramebufferPtr currentFramebuffer();

    static std::unique_ptr<MetalState> singleton;

    id<MTLDevice> device = nil;
    id<MTLCommandQueue> cmdQueue = nil;
    id<MTLCommandBuffer> cmdBuffer = nil;
    id<MTLRenderPipelineState> linearToSRGB_pso = nil;
    id<MTLRenderCommandEncoder> renderCmdEncoder = nil;
    std::stack<MaterialX::MetalFramebufferPtr> framebufferStack;

    bool supportsTiledPipeline;

    id<MTLDepthStencilState> opaqueDepthStencilState = nil;
    id<MTLDepthStencilState> transparentDepthStencilState = nil;
    id<MTLDepthStencilState> envMapDepthStencilState = nil;

    std::condition_variable inFlightCV;
    std::mutex inFlightMutex;
    std::atomic<int> inFlightCommandBuffers;
};

MATERIALX_NAMESPACE_END

#define MTL(a) (MaterialX::MetalState::getSingleton()->a)
#define MTL_DEPTHSTENCIL_STATE(a) (MaterialX::MetalState::getSingleton()->a##DepthStencilState)
#define MTL_TRIGGER_CAPTURE MaterialX::MetalState::getSingleton()->triggerProgrammaticCapture()
#define MTL_STOP_CAPTURE MaterialX::MetalState::getSingleton()->stopProgrammaticCapture()
#define MTL_PUSH_FRAMEBUFFER(a) MaterialX::MetalState::getSingleton()->framebufferStack.push(a)
#define MTL_POP_FRAMEBUFFER(a) MaterialX::MetalState::getSingleton()->framebufferStack.pop()

#endif // MATERIALXVIEW_METALSTATE_H
