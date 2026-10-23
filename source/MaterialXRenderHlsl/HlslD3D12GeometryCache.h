//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLD3D12GEOMETRYCACHE_H
#define MATERIALX_HLSLD3D12GEOMETRYCACHE_H

/// @file
/// D3D12 vertex and index buffers for MaterialX meshes.

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslD3D12Context.h>
#include <MaterialXRenderHlsl/HlslRenderUtil.h>

#include <MaterialXRender/Mesh.h>

struct ID3D12GraphicsCommandList;

MATERIALX_NAMESPACE_BEGIN

class HlslD3D12GeometryCache;
using HlslD3D12GeometryCachePtr = shared_ptr<class HlslD3D12GeometryCache>;

/// GPU buffers needed to draw one mesh partition.
struct HlslD3D12DrawBuffers
{
    uint64_t vertexBufferAddress = 0; ///< GPU virtual address of the vertex buffer.
    unsigned int vertexBufferSize = 0;
    unsigned int vertexStride = 0;
    uint64_t indexBufferAddress = 0;  ///< GPU virtual address of the 32-bit index buffer.
    unsigned int indexCount = 0;
};

/// @class HlslD3D12GeometryCache
/// Uploads mesh streams and partition indices to D3D12 buffers on first
/// use and keeps them for later draws. Vertex buffers are keyed by mesh and
/// vertex layout, since materials with different input signatures need
/// differently interleaved data; index buffers are keyed by partition.
class MX_RENDERHLSL_API HlslD3D12GeometryCache
{
  public:
    explicit HlslD3D12GeometryCache(HlslD3D12ContextPtr context);
    ~HlslD3D12GeometryCache();

    HlslD3D12GeometryCache(const HlslD3D12GeometryCache&) = delete;
    HlslD3D12GeometryCache& operator=(const HlslD3D12GeometryCache&) = delete;

    static HlslD3D12GeometryCachePtr create(HlslD3D12ContextPtr context)
    {
        return std::make_shared<HlslD3D12GeometryCache>(context);
    }

    /// Return the buffers for drawing `partition` of `mesh` with the given
    /// vertex layout, uploading them on first use. Returns false if the mesh
    /// or partition holds no data. Uploads run on an immediate command
    /// list, so this may be called while a frame is being recorded.
    bool getDrawBuffers(MeshPtr mesh, MeshPartitionPtr partition,
                        const std::vector<HlslVertexElement>& layout, unsigned int stride,
                        HlslD3D12DrawBuffers& buffers);

    /// Release the buffers of `mesh`, or of every mesh when null.
    void release(MeshPtr mesh = nullptr);

    /// Record a triangle-list draw of `buffers` on `commandList`.
    static void draw(ID3D12GraphicsCommandList* commandList, const HlslD3D12DrawBuffers& buffers);

  private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

MATERIALX_NAMESPACE_END

#endif
