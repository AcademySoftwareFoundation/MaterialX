//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_TANGENTNODESLANG_H
#define MATERIALX_TANGENTNODESLANG_H

#include <MaterialXGenShader/Nodes/HwTangentNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Tangent node implementation for hardware languages
/// Differs from HwTangentNode by:
/// - `floatX` instead of `vecX`
/// - `mul(v, m)` instead of `m * v`
class MX_GENSHADER_API TangentNodeSlang : public HwTangentNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
