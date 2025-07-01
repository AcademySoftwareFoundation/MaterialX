//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_LIGHTSAMPLERNODESLANG_H
#define MATERIALX_LIGHTSAMPLERNODESLANG_H

#include <MaterialXGenSlang/SlangShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Utility node for sampling lights for Slang.
/// Differs from LightSamplerNodeGlsl by:
/// - `floatX` instead of `vecX`
class MX_GENSLANG_API LightSamplerNodeSlang : public HwImplementation
{
  public:
    LightSamplerNodeSlang();

    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
