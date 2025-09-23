//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWNUMLIGHTSNODE_H
#define MATERIALX_HWNUMLIGHTSNODE_H

#include <MaterialXGenHw/HwImplementation.h>

MATERIALX_NAMESPACE_BEGIN

/// Utility node for getting number of active lights for hardware languages.
class MX_GENHW_API HwNumLightsNode : public HwImplementation
{
  public:
    HwNumLightsNode();

    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
