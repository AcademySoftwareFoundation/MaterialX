//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLD3D12MATERIAL_H
#define MATERIALX_HLSLD3D12MATERIAL_H

/// @file
/// Bind-time wrapper around a compiled HlslProgram for D3D12.

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslD3D12Context.h>
#include <MaterialXRenderHlsl/HlslProgram.h>
#include <MaterialXRenderHlsl/HlslRenderUtil.h>
#include <MaterialXRenderHlsl/HlslRootSignature.h>

struct ID3D12GraphicsCommandList;

MATERIALX_NAMESPACE_BEGIN

class HlslD3D12Material;
using HlslD3D12MaterialPtr = shared_ptr<class HlslD3D12Material>;

/// @class HlslD3D12Material
/// Bundles a compiled HlslProgram with the D3D12 objects needed to draw
/// with it: a root signature derived from reflection, pipeline state
/// objects, CPU copies of every constant buffer, and the texture and
/// sampler descriptors bound to the pixel stage.
///
/// The uniform API mirrors HlslMaterial. Constant buffer writes only update
/// CPU copies; bind() copies them into the context's per-frame upload ring,
/// so a material can be drawn several times in one frame with different
/// values.
///
/// The vertex input layout is derived from the vertex shader's input
/// signature: every input is packed as 32-bit components, in declaration
/// order, into a single interleaved stream (see buildHlslVertexLayout).
class MX_RENDERHLSL_API HlslD3D12Material
{
  public:
    /// Selects which stage's constant buffers an update or query targets.
    enum class Stage
    {
        Vertex,
        Pixel
    };

    HlslD3D12Material(HlslD3D12ContextPtr context, HlslProgramPtr program);
    ~HlslD3D12Material();

    HlslD3D12Material(const HlslD3D12Material&) = delete;
    HlslD3D12Material& operator=(const HlslD3D12Material&) = delete;

    static HlslD3D12MaterialPtr create(HlslD3D12ContextPtr context, HlslProgramPtr program)
    {
        return std::make_shared<HlslD3D12Material>(context, program);
    }

    /// Reflection of the vertex and pixel stages.
    const HlslStageReflection& getVertexReflection() const;
    const HlslStageReflection& getPixelReflection() const;

    /// Reflected pixel-stage bindings.
    const std::vector<HlslResourceBinding>& getPixelBindings() const;

    /// Root signature layout used by this material.
    const HlslRootSignatureDesc& getRootSignatureDesc() const;

    /// Interleaved vertex layout and stride expected by the vertex stage.
    const std::vector<HlslVertexElement>& getVertexLayout() const;
    unsigned int getVertexStride() const;

    /// Replace the contents of a constant buffer, by reflected name or by
    /// b# register in space 0. Shorter payloads are zero-padded and longer
    /// ones truncated. Returns false if the buffer does not exist.
    bool setCbufferDataByName(Stage stage, const std::string& name, const void* data, std::size_t size);
    bool setCbufferDataBySlot(Stage stage, unsigned int slot, const void* data, std::size_t size);

    /// Return the byte offset of a member of the named constant buffer, or
    /// SIZE_MAX if not found.
    std::size_t lookupVariableOffset(Stage stage, const std::string& cbufferName,
                                     const std::string& memberName) const;

    /// Write `count` bytes at `offset` inside a constant buffer. Returns
    /// false if the buffer does not exist or the write would overflow.
    bool setCbufferRange(Stage stage, unsigned int slot, std::size_t offset, const void* data, std::size_t count);
    bool setCbufferRange(Stage stage, const std::string& cbufferName, std::size_t offset,
                         const void* data, std::size_t count);

    /// Write the named uniform, whichever constant buffer of the stage owns it.
    bool patchVariable(Stage stage, const std::string& memberName, const void* data, std::size_t count);

    /// Write `<arrayName>[index].<memberName>` on the given stage.
    bool patchArrayMember(Stage stage, const std::string& arrayName, std::size_t index,
                          const std::string& memberName, const void* data, std::size_t count);

    /// Bind a staging descriptor (D3D12_CPU_DESCRIPTOR_HANDLE::ptr) to a t#
    /// or s# register of the pixel stage in space 0. Unbound registers read
    /// a null texture view or a default sampler.
    void setTexture(unsigned int slot, uint64_t srvDescriptor);
    void setSampler(unsigned int slot, uint64_t samplerDescriptor);

    /// Set the root signature, pipeline state, constant buffers and
    /// descriptor tables on `commandList`, which must be the frame command
    /// list of the context. `renderTargetFormat` and `depthFormat` are
    /// DXGI_FORMAT values of the bound framebuffer.
    void bind(ID3D12GraphicsCommandList* commandList, uint32_t renderTargetFormat, uint32_t depthFormat,
              const HlslD3D12RenderState& state = HlslD3D12RenderState());

  private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

MATERIALX_NAMESPACE_END

#endif
