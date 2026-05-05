//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLMATERIAL_H
#define MATERIALX_HLSLMATERIAL_H

/// @file
/// Bind-time wrapper around a compiled HlslProgram.

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslContext.h>
#include <MaterialXRenderHlsl/HlslProgram.h>

#include <cstddef>
#include <unordered_map>
#include <vector>

struct ID3D11Buffer;
struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11SamplerState;
struct ID3D11ShaderResourceView;
struct ID3D11VertexShader;
struct D3D11_INPUT_ELEMENT_DESC;

MATERIALX_NAMESPACE_BEGIN

class HlslMaterial;
using HlslMaterialPtr = shared_ptr<class HlslMaterial>;

/// @class HlslMaterial
/// Bundles a compiled HlslProgram with its GPU-side shader objects, the
/// cbuffer storage that backs each reflected b# slot, and helpers to
/// bind everything onto the immediate context for a draw.
///
/// Construction takes a built program and a device. The cbuffers are
/// allocated (DEFAULT usage, zero-initialised) at sizes inferred from
/// pixel-stage reflection. Callers update individual cbuffers byte-wise
/// via setCbufferDataByName() / setCbufferDataBySlot(); shader-resource
/// views (textures) and samplers are bound via setTexture() / setSampler().
///
/// The class deliberately stays at the "wrap a single program" level -
/// it does not know about MaterialX uniform names or value conversion;
/// that machinery belongs in a future ShaderRenderer subclass.
class MX_RENDERHLSL_API HlslMaterial
{
  public:
    HlslMaterial(HlslContextPtr context, HlslProgramPtr program);
    ~HlslMaterial();

    HlslMaterial(const HlslMaterial&) = delete;
    HlslMaterial& operator=(const HlslMaterial&) = delete;

    static HlslMaterialPtr create(HlslContextPtr context, HlslProgramPtr program)
    {
        return std::make_shared<HlslMaterial>(context, program);
    }

    /// Borrowed accessors. The material retains ownership.
    ID3D11VertexShader* getVertexShader() const { return _vs; }
    ID3D11PixelShader*  getPixelShader()  const { return _ps; }

    /// Build a vertex input layout from explicit element descriptors and
    /// the wrapped program's vertex bytecode (D3D matches by signature).
    /// Returns true on success; the layout is owned by this material.
    bool createInputLayout(const D3D11_INPUT_ELEMENT_DESC* elements, unsigned int count);
    ID3D11InputLayout* getInputLayout() const { return _inputLayout; }

    /// Selects which shader stage's cbuffer pool an update or query
    /// targets. The HLSL generator emits a separate cbuffer per stage
    /// (e.g. vertexCB for the world / view-projection matrices,
    /// pixelCB for material uniforms), so updates need to be routed
    /// to the right stage explicitly.
    enum class Stage { Vertex, Pixel };

    /// Update the bytes backing a cbuffer by reflected name (matches the
    /// HLSL `cbuffer NAME { ... }` identifier). Returns true if the cbuffer
    /// exists and the size fits. Larger payloads are truncated.
    bool setCbufferDataByName(Stage stage, const std::string& name, const void* data, std::size_t size);

    /// Update by reflected slot index (b#). Same semantics as above.
    bool setCbufferDataBySlot(Stage stage, unsigned int slot, const void* data, std::size_t size);

    /// Backwards-compatible single-stage overloads default to the pixel
    /// stage (matches the historical behavior).
    bool setCbufferDataByName(const std::string& name, const void* data, std::size_t size)
    { return setCbufferDataByName(Stage::Pixel, name, data, size); }
    bool setCbufferDataBySlot(unsigned int slot, const void* data, std::size_t size)
    { return setCbufferDataBySlot(Stage::Pixel, slot, data, size); }

    /// Reflect a stage's cbuffer member by name and return its byte
    /// offset within the cbuffer, or SIZE_MAX if not found. Useful for
    /// tests and host bindings that want to update a single uniform
    /// without copying the entire cbuffer payload.
    std::size_t lookupVariableOffset(Stage stage, const std::string& cbufferName,
                                     const std::string& memberName) const;

    /// Patch `count` bytes into a cbuffer at the given byte offset
    /// without disturbing the rest of its contents. The class keeps a
    /// CPU-side mirror per cbuffer; this helper updates the mirror in
    /// place and pushes the whole cbuffer once. Returns false if the
    /// stage / slot doesn't exist or the write would overflow.
    bool setCbufferRange(Stage stage, unsigned int slot,
                         std::size_t offset, const void* data, std::size_t count);

    /// Convenience overload: lookup the cbuffer slot by name then
    /// forward to setCbufferRange(stage, slot, offset, data, count).
    bool setCbufferRange(Stage stage, const std::string& cbufferName,
                         std::size_t offset, const void* data, std::size_t count);

    /// Patch `count` bytes for the named uniform on the given stage,
    /// searching every cbuffer in that stage to find which one owns
    /// it. Lets callers bind values without knowing which cbuffer the
    /// generator emitted them in - the layout differs between the
    /// default single-cbuffer-per-stage path and the binding-context
    /// path that splits uniforms across PrivateUniforms / PublicUniforms
    /// / LightData. Returns true when one cbuffer accepted the write.
    bool patchVariable(Stage stage, const std::string& memberName,
                       const void* data, std::size_t count);

    /// Bind a shader-resource view to a t# slot. Borrowed pointer; caller
    /// keeps the SRV alive for as long as it is bound.
    void setTexture(unsigned int slot, ID3D11ShaderResourceView* srv);

    /// Bind a sampler state to an s# slot. Borrowed pointer.
    void setSampler(unsigned int slot, ID3D11SamplerState* sampler);

    /// Set both shader stages, cbuffer bindings, sampled textures, and
    /// samplers on the immediate context. Does NOT set the input layout
    /// or primitive topology - callers do that separately so they can
    /// drive their own geometry.
    void bind();

    /// Helper exposing the material's reflected pixel-stage bindings,
    /// cached at construction time.
    const std::vector<HlslResourceBinding>& getPixelBindings() const { return _pixelBindings; }

  private:
    HlslContextPtr   _context;
    HlslProgramPtr   _program;
    ID3D11VertexShader* _vs = nullptr;
    ID3D11PixelShader*  _ps = nullptr;
    ID3D11InputLayout*  _inputLayout = nullptr;

    // Cbuffer storage per stage indexed by slot. Each vector is resized
    // to (max stage-local slot index + 1).
    std::vector<ID3D11Buffer*> _vsCbuffers;
    std::vector<ID3D11Buffer*> _psCbuffers;
    std::unordered_map<std::string, unsigned int> _vsCbufferNameToSlot;
    std::unordered_map<std::string, unsigned int> _psCbufferNameToSlot;

    // CPU mirror of each cbuffer's contents, for read-modify-write
    // updates via setCbufferRange. Sized to match the GPU buffer at
    // construction time and zero-initialised.
    std::vector<std::vector<uint8_t>> _vsCbufferMirrors;
    std::vector<std::vector<uint8_t>> _psCbufferMirrors;

    // Sampled textures and samplers indexed by slot, vector resized as
    // setTexture / setSampler discover slot indices.
    std::vector<ID3D11ShaderResourceView*> _textures;
    std::vector<ID3D11SamplerState*>       _samplers;

    std::vector<HlslResourceBinding> _pixelBindings;
};

MATERIALX_NAMESPACE_END

#endif
