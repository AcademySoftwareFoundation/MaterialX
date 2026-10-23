//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLROOTSIGNATURE_H
#define MATERIALX_HLSLROOTSIGNATURE_H

/// @file
/// Root signature layout derived from HLSL shader reflection.

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslShaderReflection.h>

MATERIALX_NAMESPACE_BEGIN

/// Shader stages a root parameter is visible to.
enum class HlslShaderVisibility
{
    All,
    Vertex,
    Pixel
};

/// Kind of a root parameter.
enum class HlslRootParameterType
{
    ConstantBuffer, ///< Root constant buffer view bound by GPU address (b#).
    TextureTable,   ///< Descriptor table of shader resource views (t#).
    SamplerTable    ///< Descriptor table of samplers (s#).
};

/// One parameter of a root signature.
struct HlslRootParameter
{
    HlslRootParameterType type = HlslRootParameterType::ConstantBuffer;
    HlslShaderVisibility visibility = HlslShaderVisibility::All;
    unsigned int registerBase = 0;  ///< First register covered.
    unsigned int registerCount = 1; ///< Number of registers covered (1 for a constant buffer).
    unsigned int space = 0;         ///< Register space.
    std::string name;               ///< Constant buffer name, empty for tables.
};

/// @class HlslRootSignatureDesc
/// API-neutral description of a D3D12 root signature that covers every
/// resource a MaterialX-generated vertex and pixel stage bind. It is built
/// from HlslStageReflection alone, so it can be computed and inspected
/// without a D3D12 device; HlslD3D12Material serializes it into an
/// ID3D12RootSignature.
///
/// The layout is:
///   - one root constant buffer view per reflected cbuffer, visible to the
///     stage that declares it;
///   - per stage and register space, one texture table and one sampler
///     table spanning register 0 to the highest slot in use. Unused slots
///     inside a table are bound to null descriptors.
class MX_RENDERHLSL_API HlslRootSignatureDesc
{
  public:
    HlslRootSignatureDesc() = default;

    /// Build the layout from the reflection of both stages.
    static HlslRootSignatureDesc create(const HlslStageReflection& vertex,
                                        const HlslStageReflection& pixel);

    /// Return every root parameter, in root signature order.
    const std::vector<HlslRootParameter>& getParameters() const { return _parameters; }

    /// Return the index of the constant buffer parameter bound to the given
    /// register, space and stage, or -1 if absent.
    int findConstantBuffer(HlslShaderVisibility visibility, unsigned int reg, unsigned int space = 0) const;

    /// Return the index of the table of the given type for the given stage
    /// and register space, or -1 if absent.
    int findTable(HlslRootParameterType type, HlslShaderVisibility visibility, unsigned int space = 0) const;

    /// Return the number of 32-bit root signature slots the layout uses
    /// (two per constant buffer view, one per table). D3D12 allows at most 64.
    unsigned int getRootCost() const;

  private:
    std::vector<HlslRootParameter> _parameters;
};

MATERIALX_NAMESPACE_END

#endif
