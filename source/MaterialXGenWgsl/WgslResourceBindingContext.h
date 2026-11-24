//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLRESOURCEBINDINGCONTEXT_H
#define MATERIALX_WGSLRESOURCEBINDINGCONTEXT_H

/// @file
/// WGSL resource binding context for standalone shaders.

#include <MaterialXGenWgsl/Export.h>

#include <MaterialXGenHw/HwResourceBindingContext.h>

MATERIALX_NAMESPACE_BEGIN

using WgslResourceBindingContextPtr = shared_ptr<class WgslResourceBindingContext>;

/// @class WgslResourceBindingContext
/// Emits standard WGSL resource bindings for a complete standalone shader.
///
/// All resources are placed in a single bind group with a monotonically increasing
/// binding index:
///   - Value uniforms are emitted individually as `@group(g) @binding(n) var<uniform> name: type;`
///     so node code can reference them by their plain variable names (no struct prefix).
///   - File textures are split into a `texture_2d<f32>` + `sampler` pair, each with its own binding.
///   - Light data is emitted as a uniform array of a `LightData` struct.
class MX_GENWGSL_API WgslResourceBindingContext : public HwResourceBindingContext
{
  public:
    explicit WgslResourceBindingContext(size_t group);

    static WgslResourceBindingContextPtr create(size_t group = 0)
    {
        return std::make_shared<WgslResourceBindingContext>(group);
    }

    void initialize() override;
    void emitDirectives(GenContext& context, ShaderStage& stage) override;
    void emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage) override;
    void emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
                                        ShaderStage& stage, const string& structInstanceName,
                                        const string& arraySuffix) override;

  protected:
    size_t _group;
    size_t _binding;
};

MATERIALX_NAMESPACE_END

#endif
