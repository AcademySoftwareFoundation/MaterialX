//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslD3D12Context.h>

#include <MaterialXRender/ShaderRenderer.h>

#define NOMINMAX 1
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cstdio>
#include <cstring>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

namespace
{

using Microsoft::WRL::ComPtr;

// Size of each page of the per-frame upload ring. Pages are added on
// demand and reused every frame.
const std::size_t UPLOAD_PAGE_SIZE = 4 * 1024 * 1024;

// Staging heap capacities, indexed by HlslD3D12DescriptorType.
const UINT STAGING_HEAP_SIZES[] = { 4096, 2048, 256, 64 };

// Shader-visible heap capacities. 2048 is the D3D12 maximum for samplers.
const UINT FRAME_RESOURCE_HEAP_SIZE = 65536;
const UINT FRAME_SAMPLER_HEAP_SIZE = 2048;

D3D12_DESCRIPTOR_HEAP_TYPE toHeapType(HlslD3D12DescriptorType type)
{
    switch (type)
    {
        case HlslD3D12DescriptorType::Resource:     return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        case HlslD3D12DescriptorType::Sampler:      return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        case HlslD3D12DescriptorType::RenderTarget: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        default:                                    return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    }
}

std::size_t alignUp(std::size_t value, std::size_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

void throwIfFailed(HRESULT hr, const char* what)
{
    if (FAILED(hr))
    {
        char code[16];
        std::snprintf(code, sizeof(code), "0x%08X", static_cast<unsigned int>(hr));
        throw ExceptionRenderError(std::string("HlslD3D12Context: ") + what + " failed (" + code + ").");
    }
}

D3D12_RESOURCE_DESC bufferDesc(std::size_t size)
{
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    rd.Width = size;
    rd.Height = 1;
    rd.DepthOrArraySize = 1;
    rd.MipLevels = 1;
    rd.Format = DXGI_FORMAT_UNKNOWN;
    rd.SampleDesc.Count = 1;
    rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    return rd;
}

D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle(uint64_t ptr)
{
    D3D12_CPU_DESCRIPTOR_HANDLE h;
    h.ptr = static_cast<SIZE_T>(ptr);
    return h;
}

struct StagingHeap
{
    ComPtr<ID3D12DescriptorHeap> heap;
    SIZE_T start = 0;
    UINT increment = 0;
    UINT capacity = 0;
    UINT next = 0;
    std::vector<UINT> freeSlots;
};

struct FrameHeap
{
    ComPtr<ID3D12DescriptorHeap> heap;
    SIZE_T cpuStart = 0;
    UINT64 gpuStart = 0;
    UINT increment = 0;
    UINT capacity = 0;
    UINT next = 0;
};

struct UploadPage
{
    ComPtr<ID3D12Resource> buffer;
    uint8_t* cpu = nullptr;
    D3D12_GPU_VIRTUAL_ADDRESS gpu = 0;
    std::size_t size = 0;
    std::size_t offset = 0;
};

} // namespace

struct HlslD3D12Context::Impl
{
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> queue;
    ComPtr<ID3D12Fence> fence;
    HANDLE fenceEvent = nullptr;
    UINT64 fenceValue = 0;

    ComPtr<ID3D12CommandAllocator> frameAllocator;
    ComPtr<ID3D12GraphicsCommandList> frameList;
    bool recording = false;

    ComPtr<ID3D12CommandAllocator> immediateAllocator;
    ComPtr<ID3D12GraphicsCommandList> immediateList;
    bool executingImmediate = false;

    StagingHeap staging[4];
    FrameHeap frameResources;
    FrameHeap frameSamplers;

    std::vector<UploadPage> uploadPages;
    std::size_t uploadPage = 0;

    uint64_t nullTexture = 0;
    uint64_t defaultSampler = 0;
    bool isHardware = true;

    uint32_t renderTargetFormat = DXGI_FORMAT_UNKNOWN;
    uint32_t depthFormat = DXGI_FORMAT_UNKNOWN;
    HlslD3D12RenderState renderState;

    void waitForGpu()
    {
        const UINT64 value = ++fenceValue;
        throwIfFailed(queue->Signal(fence.Get(), value), "ID3D12CommandQueue::Signal");
        if (fence->GetCompletedValue() < value)
        {
            throwIfFailed(fence->SetEventOnCompletion(value, fenceEvent), "ID3D12Fence::SetEventOnCompletion");
            ::WaitForSingleObject(fenceEvent, INFINITE);
        }
    }

    void submit(ID3D12GraphicsCommandList* list)
    {
        throwIfFailed(list->Close(), "ID3D12GraphicsCommandList::Close");
        ID3D12CommandList* lists[] = { list };
        queue->ExecuteCommandLists(1, lists);
        waitForGpu();
        throwIfFailed(device->GetDeviceRemovedReason(), "command list execution");
    }

    FrameHeap* frameHeap(HlslD3D12DescriptorType type)
    {
        if (type == HlslD3D12DescriptorType::Resource)
            return &frameResources;
        if (type == HlslD3D12DescriptorType::Sampler)
            return &frameSamplers;
        return nullptr;
    }
};

HlslD3D12Context::HlslD3D12Context(bool preferWarp) :
    _impl(new Impl())
{
    Impl& d = *_impl;

#ifdef _DEBUG
    // The debug layer is only present when the Graphics Tools optional
    // feature is installed, so a failure here is not an error.
    ComPtr<ID3D12Debug> debug;
    if (SUCCEEDED(::D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()))))
        debug->EnableDebugLayer();
#endif

    ComPtr<IDXGIFactory4> factory;
    throwIfFailed(::CreateDXGIFactory2(0, IID_PPV_ARGS(factory.GetAddressOf())), "CreateDXGIFactory2");

    if (!preferWarp)
    {
        ComPtr<IDXGIAdapter1> adapter;
        for (UINT i = 0; factory->EnumAdapters1(i, adapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            DXGI_ADAPTER_DESC1 desc = {};
            adapter->GetDesc1(&desc);
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                continue;
            if (SUCCEEDED(::D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                              IID_PPV_ARGS(d.device.GetAddressOf()))))
            {
                break;
            }
        }
    }
    if (!d.device)
    {
        d.isHardware = false;
        ComPtr<IDXGIAdapter> warp;
        if (SUCCEEDED(factory->EnumWarpAdapter(IID_PPV_ARGS(warp.GetAddressOf()))))
        {
            ::D3D12CreateDevice(warp.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(d.device.GetAddressOf()));
        }
    }
    if (!d.device)
        throw ExceptionRenderError("HlslD3D12Context: D3D12CreateDevice failed (hardware and WARP).");

    D3D12_COMMAND_QUEUE_DESC qd = {};
    qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    throwIfFailed(d.device->CreateCommandQueue(&qd, IID_PPV_ARGS(d.queue.GetAddressOf())), "CreateCommandQueue");
    throwIfFailed(d.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(d.fence.GetAddressOf())), "CreateFence");
    d.fenceEvent = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!d.fenceEvent)
        throw ExceptionRenderError("HlslD3D12Context: CreateEvent failed.");

    auto createList = [&d](ComPtr<ID3D12CommandAllocator>& allocator, ComPtr<ID3D12GraphicsCommandList>& list)
    {
        throwIfFailed(d.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                       IID_PPV_ARGS(allocator.GetAddressOf())),
                      "CreateCommandAllocator");
        throwIfFailed(d.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr,
                                                  IID_PPV_ARGS(list.GetAddressOf())),
                      "CreateCommandList");
        // Command lists are created open; close them so every use starts
        // with a Reset.
        list->Close();
    };
    createList(d.frameAllocator, d.frameList);
    createList(d.immediateAllocator, d.immediateList);

    for (int i = 0; i < 4; ++i)
    {
        StagingHeap& h = d.staging[i];
        D3D12_DESCRIPTOR_HEAP_DESC hd = {};
        hd.Type = toHeapType(static_cast<HlslD3D12DescriptorType>(i));
        hd.NumDescriptors = STAGING_HEAP_SIZES[i];
        hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        throwIfFailed(d.device->CreateDescriptorHeap(&hd, IID_PPV_ARGS(h.heap.GetAddressOf())), "CreateDescriptorHeap");
        h.start = h.heap->GetCPUDescriptorHandleForHeapStart().ptr;
        h.increment = d.device->GetDescriptorHandleIncrementSize(hd.Type);
        h.capacity = hd.NumDescriptors;
    }

    auto createFrameHeap = [&d](FrameHeap& h, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count)
    {
        D3D12_DESCRIPTOR_HEAP_DESC hd = {};
        hd.Type = type;
        hd.NumDescriptors = count;
        hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        throwIfFailed(d.device->CreateDescriptorHeap(&hd, IID_PPV_ARGS(h.heap.GetAddressOf())), "CreateDescriptorHeap");
        h.cpuStart = h.heap->GetCPUDescriptorHandleForHeapStart().ptr;
        h.gpuStart = h.heap->GetGPUDescriptorHandleForHeapStart().ptr;
        h.increment = d.device->GetDescriptorHandleIncrementSize(type);
        h.capacity = count;
    };
    createFrameHeap(d.frameResources, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, FRAME_RESOURCE_HEAP_SIZE);
    createFrameHeap(d.frameSamplers, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, FRAME_SAMPLER_HEAP_SIZE);

    d.nullTexture = allocateStagingDescriptor(HlslD3D12DescriptorType::Resource);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvd.Texture2D.MipLevels = 1;
    d.device->CreateShaderResourceView(nullptr, &srvd, cpuHandle(d.nullTexture));

    d.defaultSampler = allocateStagingDescriptor(HlslD3D12DescriptorType::Sampler);
    D3D12_SAMPLER_DESC sd = {};
    sd.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sd.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sd.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sd.MaxAnisotropy = 1;
    sd.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sd.MaxLOD = D3D12_FLOAT32_MAX;
    d.device->CreateSampler(&sd, cpuHandle(d.defaultSampler));
}

HlslD3D12Context::~HlslD3D12Context()
{
    if (_impl->recording)
        _impl->frameList->Close();
    try
    {
        _impl->waitForGpu();
    }
    catch (const std::exception&)
    {
        // The device may have been removed; nothing left to wait for.
    }
    if (_impl->fenceEvent)
        ::CloseHandle(_impl->fenceEvent);
}

bool HlslD3D12Context::isHardware() const
{
    return _impl->isHardware;
}

ID3D12Device* HlslD3D12Context::getDevice() const
{
    return _impl->device.Get();
}

ID3D12CommandQueue* HlslD3D12Context::getCommandQueue() const
{
    return _impl->queue.Get();
}

ID3D12GraphicsCommandList* HlslD3D12Context::beginFrame()
{
    Impl& d = *_impl;
    if (d.recording)
        throw ExceptionRenderError("HlslD3D12Context::beginFrame: a frame is already being recorded.");

    throwIfFailed(d.frameAllocator->Reset(), "ID3D12CommandAllocator::Reset");
    throwIfFailed(d.frameList->Reset(d.frameAllocator.Get(), nullptr), "ID3D12GraphicsCommandList::Reset");

    d.frameResources.next = 0;
    d.frameSamplers.next = 0;
    for (UploadPage& page : d.uploadPages)
        page.offset = 0;
    d.uploadPage = 0;

    ID3D12DescriptorHeap* heaps[] = { d.frameResources.heap.Get(), d.frameSamplers.heap.Get() };
    d.frameList->SetDescriptorHeaps(2, heaps);
    d.recording = true;
    return d.frameList.Get();
}

void HlslD3D12Context::endFrame()
{
    Impl& d = *_impl;
    if (!d.recording)
        throw ExceptionRenderError("HlslD3D12Context::endFrame: no frame is being recorded.");
    d.recording = false;
    d.submit(d.frameList.Get());
}

bool HlslD3D12Context::isRecordingFrame() const
{
    return _impl->recording;
}

ID3D12GraphicsCommandList* HlslD3D12Context::getFrameCommandList() const
{
    return _impl->recording ? _impl->frameList.Get() : nullptr;
}

void HlslD3D12Context::setRenderTargetFormats(uint32_t renderTargetFormat, uint32_t depthFormat)
{
    _impl->renderTargetFormat = renderTargetFormat;
    _impl->depthFormat = depthFormat;
}

uint32_t HlslD3D12Context::getRenderTargetFormat() const
{
    return _impl->renderTargetFormat;
}

uint32_t HlslD3D12Context::getDepthFormat() const
{
    return _impl->depthFormat;
}

void HlslD3D12Context::setRenderState(const HlslD3D12RenderState& state)
{
    _impl->renderState = state;
}

const HlslD3D12RenderState& HlslD3D12Context::getRenderState() const
{
    return _impl->renderState;
}

void HlslD3D12Context::executeImmediate(const std::function<void(ID3D12GraphicsCommandList*)>& record)
{
    Impl& d = *_impl;
    if (d.executingImmediate)
        throw ExceptionRenderError("HlslD3D12Context::executeImmediate: nested call.");
    d.executingImmediate = true;
    try
    {
        throwIfFailed(d.immediateAllocator->Reset(), "ID3D12CommandAllocator::Reset");
        throwIfFailed(d.immediateList->Reset(d.immediateAllocator.Get(), nullptr), "ID3D12GraphicsCommandList::Reset");
        record(d.immediateList.Get());
        d.submit(d.immediateList.Get());
    }
    catch (...)
    {
        // Leave the list closed so that the next call can reset it.
        d.immediateList->Close();
        d.executingImmediate = false;
        throw;
    }
    d.executingImmediate = false;
}

HlslD3D12UploadAllocation HlslD3D12Context::allocateUpload(std::size_t size, std::size_t alignment)
{
    Impl& d = *_impl;
    size = std::max<std::size_t>(size, 1);
    for (; d.uploadPage < d.uploadPages.size(); ++d.uploadPage)
    {
        UploadPage& page = d.uploadPages[d.uploadPage];
        const std::size_t offset = alignUp(page.offset, alignment);
        if (offset + size <= page.size)
        {
            page.offset = offset + size;
            return { page.cpu + offset, page.gpu + offset };
        }
    }

    UploadPage page;
    page.size = std::max(UPLOAD_PAGE_SIZE, alignUp(size, 65536));
    page.buffer.Attach(createUploadBuffer(page.size));
    D3D12_RANGE noRead = { 0, 0 };
    void* mapped = nullptr;
    throwIfFailed(page.buffer->Map(0, &noRead, &mapped), "ID3D12Resource::Map");
    page.cpu = static_cast<uint8_t*>(mapped);
    page.gpu = page.buffer->GetGPUVirtualAddress();
    page.offset = size;
    d.uploadPages.push_back(page);
    d.uploadPage = d.uploadPages.size() - 1;
    return { page.cpu, page.gpu };
}

uint64_t HlslD3D12Context::allocateStagingDescriptor(HlslD3D12DescriptorType type)
{
    StagingHeap& h = _impl->staging[static_cast<int>(type)];
    UINT slot = 0;
    if (!h.freeSlots.empty())
    {
        slot = h.freeSlots.back();
        h.freeSlots.pop_back();
    }
    else if (h.next < h.capacity)
    {
        slot = h.next++;
    }
    else
    {
        throw ExceptionRenderError("HlslD3D12Context: staging descriptor heap exhausted.");
    }
    return static_cast<uint64_t>(h.start) + static_cast<uint64_t>(slot) * h.increment;
}

void HlslD3D12Context::freeStagingDescriptor(HlslD3D12DescriptorType type, uint64_t handle)
{
    StagingHeap& h = _impl->staging[static_cast<int>(type)];
    if (handle < h.start)
        return;
    const UINT slot = static_cast<UINT>((handle - h.start) / h.increment);
    if (slot < h.next)
        h.freeSlots.push_back(slot);
}

bool HlslD3D12Context::allocateFrameDescriptors(HlslD3D12DescriptorType type, unsigned int count,
                                                uint64_t& cpuStart, uint64_t& gpuStart)
{
    FrameHeap* h = _impl->frameHeap(type);
    if (!h || h->next + count > h->capacity)
        return false;
    cpuStart = static_cast<uint64_t>(h->cpuStart) + static_cast<uint64_t>(h->next) * h->increment;
    gpuStart = h->gpuStart + static_cast<uint64_t>(h->next) * h->increment;
    h->next += count;
    return true;
}

unsigned int HlslD3D12Context::getDescriptorIncrement(HlslD3D12DescriptorType type) const
{
    return _impl->staging[static_cast<int>(type)].increment;
}

uint64_t HlslD3D12Context::getNullTextureDescriptor() const
{
    return _impl->nullTexture;
}

uint64_t HlslD3D12Context::getDefaultSamplerDescriptor() const
{
    return _impl->defaultSampler;
}

ID3D12Resource* HlslD3D12Context::createBuffer(const void* data, std::size_t size)
{
    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = D3D12_HEAP_TYPE_DEFAULT;
    const D3D12_RESOURCE_DESC rd = bufferDesc(std::max<std::size_t>(size, 1));
    ComPtr<ID3D12Resource> buffer;
    // Buffers are created in the common state; copies and later reads as
    // vertex or index data rely on implicit state promotion.
    throwIfFailed(_impl->device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &rd,
                                                         D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                         IID_PPV_ARGS(buffer.GetAddressOf())),
                  "CreateCommittedResource (buffer)");
    if (data && size)
    {
        ComPtr<ID3D12Resource> staging;
        staging.Attach(createUploadBuffer(size));
        D3D12_RANGE noRead = { 0, 0 };
        void* mapped = nullptr;
        throwIfFailed(staging->Map(0, &noRead, &mapped), "ID3D12Resource::Map");
        std::memcpy(mapped, data, size);
        staging->Unmap(0, nullptr);
        executeImmediate([&](ID3D12GraphicsCommandList* list)
        {
            list->CopyBufferRegion(buffer.Get(), 0, staging.Get(), 0, size);
        });
    }
    return buffer.Detach();
}

ID3D12Resource* HlslD3D12Context::createUploadBuffer(std::size_t size)
{
    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = D3D12_HEAP_TYPE_UPLOAD;
    const D3D12_RESOURCE_DESC rd = bufferDesc(std::max<std::size_t>(size, 1));
    ID3D12Resource* buffer = nullptr;
    throwIfFailed(_impl->device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &rd,
                                                         D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                         IID_PPV_ARGS(&buffer)),
                  "CreateCommittedResource (upload buffer)");
    return buffer;
}

ID3D12Resource* HlslD3D12Context::createReadbackBuffer(std::size_t size)
{
    D3D12_HEAP_PROPERTIES heap = {};
    heap.Type = D3D12_HEAP_TYPE_READBACK;
    const D3D12_RESOURCE_DESC rd = bufferDesc(std::max<std::size_t>(size, 1));
    ID3D12Resource* buffer = nullptr;
    throwIfFailed(_impl->device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &rd,
                                                         D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                                         IID_PPV_ARGS(&buffer)),
                  "CreateCommittedResource (readback buffer)");
    return buffer;
}

void HlslD3D12Context::transitionResource(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
                                          uint32_t stateBefore, uint32_t stateAfter)
{
    if (!commandList || !resource || stateBefore == stateAfter)
        return;
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = resource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(stateBefore);
    barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(stateAfter);
    commandList->ResourceBarrier(1, &barrier);
}

MATERIALX_NAMESPACE_END
