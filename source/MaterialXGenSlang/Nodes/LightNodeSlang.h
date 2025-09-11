//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_LIGHTNODESLANG_H
#define MATERIALX_LIGHTNODESLANG_H

#include <MaterialXGenSlang/SlangShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Light node implementation for Slang
/// Differs from LightNodeGlsl by:
/// - `floatX` instead of `vecX`
/// - downcasting to SlangShaderGenerator instead of the GlslShaderGenerator.
class MX_GENSLANG_API LightNodeSlang : public HwImplementation
{
  public:
    LightNodeSlang();

    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
