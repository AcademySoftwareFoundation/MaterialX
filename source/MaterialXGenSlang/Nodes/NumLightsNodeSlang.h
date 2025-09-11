//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_NUMLIGHTSNODESLANG_H
#define MATERIALX_NUMLIGHTSNODESLANG_H

#include <MaterialXGenSlang/SlangShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Utility node for getting number of active lights for Slang.
/// Does not differ from NumLightsNodeGlsl, but we do not include cross generators.
class MX_GENSLANG_API NumLightsNodeSlang : public HwImplementation
{
  public:
    NumLightsNodeSlang();

    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
