//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWBITANGENTNODE_H
#define MATERIALX_HWBITANGENTNODE_H

#include <MaterialXGenHw/HwImplementation.h>

MATERIALX_NAMESPACE_BEGIN

/// Bitangent node implementation for hardware languages
class MX_GENHW_API HwBitangentNode : public HwImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
