//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_BITANGENTNODESLANG_H
#define MATERIALX_BITANGENTNODESLANG_H

#include <MaterialXGenShader/Nodes/HwBitangentNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Bitangent node implementation for Slang.
/// Differs from HwBitangentNode by:
/// - `floatX` instead of `vecX`
/// - `mul(v, m)` instead of `m * v`
class MX_GENSHADER_API BitangentNodeSlang : public HwBitangentNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
