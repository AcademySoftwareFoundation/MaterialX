//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslD3D12GeometryCache.h>

#define NOMINMAX 1
#include <Windows.h>
#include <d3d12.h>
#include <wrl/client.h>

#include <map>

MATERIALX_NAMESPACE_BEGIN

namespace
{

using Microsoft::WRL::ComPtr;

// Key identifying a vertex layout.
std::string layoutKey(const std::vector<HlslVertexElement>& layout)
{
    std::string key;
    for (const HlslVertexElement& e : layout)
    {
        key += e.semanticName;
        key += std::to_string(e.semanticIndex);
        key += ':';
        key += std::to_string(e.componentCount);
        key += ';';
    }
    return key;
}

struct VertexBuffer
{
    ComPtr<ID3D12Resource> buffer;
    unsigned int size = 0;
};

struct IndexBuffer
{
    ComPtr<ID3D12Resource> buffer;
    unsigned int count = 0;
};

} // namespace

struct HlslD3D12GeometryCache::Impl
{
    HlslD3D12ContextPtr context;
    std::map<std::pair<const Mesh*, std::string>, VertexBuffer> vertexBuffers;
    std::map<const MeshPartition*, IndexBuffer> indexBuffers;

    // Meshes are kept alive while cached, so that a freed mesh address
    // cannot alias a new one. Partitions are owned by their mesh.
    std::map<const Mesh*, MeshPtr> meshes;
};

HlslD3D12GeometryCache::HlslD3D12GeometryCache(HlslD3D12ContextPtr context) :
    _impl(new Impl())
{
    _impl->context = std::move(context);
}

HlslD3D12GeometryCache::~HlslD3D12GeometryCache() = default;

bool HlslD3D12GeometryCache::getDrawBuffers(MeshPtr mesh, MeshPartitionPtr partition,
                                            const std::vector<HlslVertexElement>& layout, unsigned int stride,
                                            HlslD3D12DrawBuffers& buffers)
{
    Impl& d = *_impl;
    if (!mesh || !partition || mesh->getVertexCount() == 0 || stride == 0 || partition->getIndices().empty())
        return false;

    const auto vbKey = std::make_pair(static_cast<const Mesh*>(mesh.get()), layoutKey(layout));
    auto vbIt = d.vertexBuffers.find(vbKey);
    if (vbIt == d.vertexBuffers.end())
    {
        const std::vector<float> vertices = interleaveHlslVertices(mesh, layout, stride);
        VertexBuffer vb;
        vb.size = static_cast<unsigned int>(vertices.size() * sizeof(float));
        vb.buffer.Attach(d.context->createBuffer(vertices.data(), vb.size));
        vbIt = d.vertexBuffers.emplace(vbKey, vb).first;
        d.meshes[mesh.get()] = mesh;
    }

    auto ibIt = d.indexBuffers.find(partition.get());
    if (ibIt == d.indexBuffers.end())
    {
        const MeshIndexBuffer& indices = partition->getIndices();
        IndexBuffer ib;
        ib.count = static_cast<unsigned int>(indices.size());
        ib.buffer.Attach(d.context->createBuffer(indices.data(), indices.size() * sizeof(uint32_t)));
        ibIt = d.indexBuffers.emplace(partition.get(), ib).first;
        d.meshes[mesh.get()] = mesh;
    }

    buffers.vertexBufferAddress = vbIt->second.buffer->GetGPUVirtualAddress();
    buffers.vertexBufferSize = vbIt->second.size;
    buffers.vertexStride = stride;
    buffers.indexBufferAddress = ibIt->second.buffer->GetGPUVirtualAddress();
    buffers.indexCount = ibIt->second.count;
    return true;
}

void HlslD3D12GeometryCache::release(MeshPtr mesh)
{
    Impl& d = *_impl;
    if (!mesh)
    {
        d.vertexBuffers.clear();
        d.indexBuffers.clear();
        d.meshes.clear();
        return;
    }
    for (auto it = d.vertexBuffers.begin(); it != d.vertexBuffers.end();)
        it = (it->first.first == mesh.get()) ? d.vertexBuffers.erase(it) : std::next(it);
    for (size_t i = 0; i < mesh->getPartitionCount(); ++i)
        d.indexBuffers.erase(mesh->getPartition(i).get());
    d.meshes.erase(mesh.get());
}

void HlslD3D12GeometryCache::draw(ID3D12GraphicsCommandList* commandList, const HlslD3D12DrawBuffers& buffers)
{
    D3D12_VERTEX_BUFFER_VIEW vbv = {};
    vbv.BufferLocation = buffers.vertexBufferAddress;
    vbv.SizeInBytes = buffers.vertexBufferSize;
    vbv.StrideInBytes = buffers.vertexStride;
    D3D12_INDEX_BUFFER_VIEW ibv = {};
    ibv.BufferLocation = buffers.indexBufferAddress;
    ibv.SizeInBytes = buffers.indexCount * sizeof(uint32_t);
    ibv.Format = DXGI_FORMAT_R32_UINT;

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vbv);
    commandList->IASetIndexBuffer(&ibv);
    commandList->DrawIndexedInstanced(buffers.indexCount, 1, 0, 0, 0);
}

MATERIALX_NAMESPACE_END
