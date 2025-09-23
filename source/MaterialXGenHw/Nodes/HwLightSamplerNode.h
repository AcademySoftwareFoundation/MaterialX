//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWLIGHTSAMPLERNODE_H
#define MATERIALX_HWLIGHTSAMPLERNODE_H

#include <MaterialXGenHw/HwImplementation.h>

MATERIALX_NAMESPACE_BEGIN

/// Utility node for sampling lights for hardware languages.
class MX_GENHW_API HwLightSamplerNode : public HwImplementation
{
  public:
    HwLightSamplerNode();

    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
