//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWTIMENODE_H
#define MATERIALX_HWTIMENODE_H

#include <MaterialXGenShader/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Time node implementation for hardware languages
class MX_GENSHADER_API HwTimeNode : public HwImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
