//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLD3D12CONTEXT_H
#define MATERIALX_HLSLD3D12CONTEXT_H

/// @file
/// D3D12 device, command submission and descriptor management.

#include <MaterialXRenderHlsl/Export.h>

#include <MaterialXCore/Library.h>

#include <cstdint>
#include <functional>
#include <memory>

struct ID3D12CommandQueue;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

MATERIALX_NAMESPACE_BEGIN

class HlslD3D12Context;
using HlslD3D12ContextPtr = shared_ptr<class HlslD3D12Context>;

/// Descriptor heap categories managed by HlslD3D12Context.
enum class HlslD3D12DescriptorType
{
    Resource,     ///< Constant buffer, shader resource and unordered access views.
    Sampler,      ///< Samplers.
    RenderTarget, ///< Render target views.
    DepthStencil  ///< Depth stencil views.
};

/// Fixed-function state baked into a D3D12 pipeline state object.
struct HlslD3D12RenderState
{
    bool blend = false;          ///< Alpha blending with source alpha and one minus source alpha.
    bool depthTest = true;
    bool depthWrite = true;
    bool depthLessEqual = false; ///< Pass depth values equal to the stored depth.
    bool cullBackFaces = false;
    bool wireframe = false;
};

/// A range of CPU-writable, GPU-readable memory from the per-frame upload ring.
struct HlslD3D12UploadAllocation
{
    void* cpuAddress = nullptr;
    uint64_t gpuAddress = 0;
};

/// @class HlslD3D12Context
/// Owns a D3D12 device and a direct command queue for off-screen rendering,
/// along with the descriptor heaps and upload memory shared by the HLSL
/// D3D12 classes.
///
/// Submission is synchronous, which keeps resource lifetimes simple for
/// test and viewer rendering:
///   - beginFrame() and endFrame() bracket the recording of a frame command
///     list; endFrame() executes it and waits for the GPU.
///   - executeImmediate() records, executes and waits for a separate command
///     list. It is used for resource uploads and readback, and may be called
///     while a frame is being recorded, in which case its work completes
///     before the frame is submitted.
///
/// Descriptors live in two kinds of heaps:
///   - staging heaps (CPU only) hold long-lived descriptors such as texture
///     views, samplers, render target and depth views;
///   - shader-visible heaps receive per-frame copies of those descriptors
///     and are reset by beginFrame().
///
/// Descriptor handles are exchanged as the `ptr` values of
/// D3D12_CPU_DESCRIPTOR_HANDLE and D3D12_GPU_DESCRIPTOR_HANDLE, and resource
/// states as D3D12_RESOURCE_STATES values, so that this header does not
/// depend on d3d12.h.
class MX_RENDERHLSL_API HlslD3D12Context
{
  public:
    /// Create a device on the first hardware adapter supporting feature
    /// level 11.0, falling back to WARP. When preferWarp is true WARP is
    /// used directly. Throws ExceptionRenderError if no device can be created.
    explicit HlslD3D12Context(bool preferWarp = false);
    ~HlslD3D12Context();

    HlslD3D12Context(const HlslD3D12Context&) = delete;
    HlslD3D12Context& operator=(const HlslD3D12Context&) = delete;

    static HlslD3D12ContextPtr create(bool preferWarp = false)
    {
        return std::make_shared<HlslD3D12Context>(preferWarp);
    }

    /// True if the device wraps a hardware adapter, false if WARP.
    bool isHardware() const;

    /// Borrowed device and command queue.
    ID3D12Device* getDevice() const;
    ID3D12CommandQueue* getCommandQueue() const;

    /// Start recording a frame and return its command list, with the
    /// shader-visible descriptor heaps already set. Resets the per-frame
    /// descriptor heaps and the upload ring. Throws if a frame is already
    /// being recorded.
    ID3D12GraphicsCommandList* beginFrame();

    /// Close the frame command list, execute it and wait for completion.
    void endFrame();

    /// Return true between beginFrame() and endFrame().
    bool isRecordingFrame() const;

    /// Return the frame command list while a frame is being recorded, or
    /// nullptr otherwise.
    ID3D12GraphicsCommandList* getFrameCommandList() const;

    /// Record the DXGI formats of the render target and depth target bound
    /// on the frame command list. Called by HlslD3D12Framebuffer::bind().
    void setRenderTargetFormats(uint32_t renderTargetFormat, uint32_t depthFormat);
    uint32_t getRenderTargetFormat() const;
    uint32_t getDepthFormat() const;

    /// Fixed-function state for draws issued through HlslD3D12ShaderMaterial,
    /// which, like OpenGL materials, take their state from the context.
    void setRenderState(const HlslD3D12RenderState& state);
    const HlslD3D12RenderState& getRenderState() const;

    /// Record commands with `record` into a one-off command list, execute it
    /// and wait for completion.
    void executeImmediate(const std::function<void(ID3D12GraphicsCommandList*)>& record);

    /// Allocate `size` bytes of upload memory aligned to `alignment`. The
    /// memory stays valid until the next beginFrame().
    HlslD3D12UploadAllocation allocateUpload(std::size_t size, std::size_t alignment = 256);

    /// Allocate a descriptor from the staging heap of the given type.
    uint64_t allocateStagingDescriptor(HlslD3D12DescriptorType type);

    /// Return a staging descriptor to its heap.
    void freeStagingDescriptor(HlslD3D12DescriptorType type, uint64_t handle);

    /// Allocate `count` contiguous descriptors of the given type (Resource
    /// or Sampler) from the shader-visible heap for the current frame.
    /// Returns false if the heap is exhausted.
    bool allocateFrameDescriptors(HlslD3D12DescriptorType type, unsigned int count,
                                  uint64_t& cpuStart, uint64_t& gpuStart);

    /// Size in bytes of one descriptor of the given type.
    unsigned int getDescriptorIncrement(HlslD3D12DescriptorType type) const;

    /// Staging descriptors used for unbound table slots: a null 2D texture
    /// view, and a linear sampler with repeat addressing.
    uint64_t getNullTextureDescriptor() const;
    uint64_t getDefaultSamplerDescriptor() const;

    /// Create a buffer in the default heap holding `size` bytes of `data`.
    /// The caller owns the returned reference and must Release() it.
    ID3D12Resource* createBuffer(const void* data, std::size_t size);

    /// Create a persistently mappable buffer in the upload heap. The caller
    /// owns the returned reference.
    ID3D12Resource* createUploadBuffer(std::size_t size);

    /// Create a buffer in the readback heap. The caller owns the returned
    /// reference.
    ID3D12Resource* createReadbackBuffer(std::size_t size);

    /// Record a transition barrier for every subresource of `resource`.
    static void transitionResource(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
                                   uint32_t stateBefore, uint32_t stateAfter);

  private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

MATERIALX_NAMESPACE_END

#endif
