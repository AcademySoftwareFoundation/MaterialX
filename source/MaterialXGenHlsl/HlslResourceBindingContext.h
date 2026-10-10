//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLRESOURCEBINDING_H
#define MATERIALX_HLSLRESOURCEBINDING_H

/// @file
/// HLSL resource binding context

#include <MaterialXGenHlsl/Export.h>

#include <MaterialXGenHw/HwResourceBindingContext.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a HlslResourceBindingContext
using HlslResourceBindingContextPtr = shared_ptr<class HlslResourceBindingContext>;

/// @class HlslResourceBindingContext
/// Resource binding context for HLSL targets that emits explicit register()
/// annotations. HLSL has three relevant register namespaces for graphics
/// shaders:
///
///   * b#  constant buffers
///   * t#  shader resource views (textures)
///   * s#  samplers
///
/// Each namespace has its own counter. The combined SamplerTexture2D handle
/// used by the generated MaterialX shaders pulls one slot from each of t#
/// and s#, since DXC sees the struct fields as separate root parameters.
///
/// When this context is attached to an HlslShaderGenerator via
/// GenContext::pushUserData(HW::USER_DATA_BINDING_CONTEXT), the generator
/// emits explicit register() bindings on cbuffers and texture/sampler
/// declarations. Without it the generator omits the register() annotations
/// and lets DXC auto-assign slots.
class MX_GENHLSL_API HlslResourceBindingContext : public HwResourceBindingContext
{
  public:
    HlslResourceBindingContext(size_t cbufferRegister = 0,
                               size_t textureRegister = 0,
                               size_t samplerRegister = 0);

    static HlslResourceBindingContextPtr create(size_t cbufferRegister = 0,
                                                size_t textureRegister = 0,
                                                size_t samplerRegister = 0)
    {
        return std::make_shared<HlslResourceBindingContext>(
            cbufferRegister, textureRegister, samplerRegister);
    }

    void initialize() override;
    void emitDirectives(GenContext& context, ShaderStage& stage) override;
    void emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage) override;
    void emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
                                        ShaderStage& stage, const std::string& structInstanceName,
                                        const std::string& arraySuffix) override;

    /// Allocate the next constant buffer register and return its index (b#).
    size_t nextCbufferRegister() { return _cbufferRegister++; }

    /// Allocate the next texture (SRV) register and return its index (t#).
    size_t nextTextureRegister() { return _textureRegister++; }

    /// Allocate the next sampler register and return its index (s#).
    size_t nextSamplerRegister() { return _samplerRegister++; }

    /// Optional shader-visibility register space. HLSL register annotations
    /// can be qualified with `, space<n>` for D3D12 root signature layouts;
    /// when non-empty this context emits the qualifier.
    void setRegisterSpace(const std::string& space) { _registerSpace = space; }
    const std::string& getRegisterSpace() const { return _registerSpace; }

  protected:
    size_t _initCbufferRegister;
    size_t _initTextureRegister;
    size_t _initSamplerRegister;

    size_t _cbufferRegister = 0;
    size_t _textureRegister = 0;
    size_t _samplerRegister = 0;

    std::string _registerSpace;
};

MATERIALX_NAMESPACE_END

#endif
