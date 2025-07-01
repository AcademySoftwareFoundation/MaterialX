//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SURFACENODESLANG_H
#define MATERIALX_SURFACENODESLANG_H

#include <MaterialXGenSlang/Export.h>
#include <MaterialXGenSlang/SlangShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Surface node implementation for Slang
/// Differs from SurfaceNodeGlsl by:
/// - `floatX` instead of `vecX`
/// - `mul(v, m)` instead of `m * v`
/// - `lerp` instead of `mix`
/// - downcasting to SlangShaderGenerator instead of the GlslShaderGenerator.
class MX_GENSLANG_API SurfaceNodeSlang : public HwImplementation
{
  public:
    SurfaceNodeSlang();

    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    virtual void emitLightLoop(const ShaderNode& node, GenContext& context, ShaderStage& stage, const string& outColor) const;
};

MATERIALX_NAMESPACE_END

#endif
