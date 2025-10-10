//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWLIGHTNODE_H
#define MATERIALX_HWLIGHTNODE_H

#include <MaterialXGenHw/HwImplementation.h>

MATERIALX_NAMESPACE_BEGIN

/// Light node implementation for hardware languages.
class MX_GENHW_API HwLightNode : public HwImplementation
{
  public:
    HwLightNode();

    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
