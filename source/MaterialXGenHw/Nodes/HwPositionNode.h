//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWPOSITIONNODE_H
#define MATERIALX_HWPOSITIONNODE_H

#include <MaterialXGenHw/HwImplementation.h>

MATERIALX_NAMESPACE_BEGIN

/// Position node implementation for hardware languages
class MX_GENHW_API HwPositionNode : public HwImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
