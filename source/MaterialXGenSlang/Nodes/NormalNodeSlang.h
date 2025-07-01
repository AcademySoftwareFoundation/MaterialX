//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_NORMALNODESLANG_H
#define MATERIALX_NORMALNODESLANG_H

#include <MaterialXGenShader/Nodes/HwNormalNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Normal node implementation for hardware languages
/// Differs from HwNormalNode by:
/// - `floatX` instead of `vecX`
/// - `mul(v, m)` instead of `m * v`
class MX_GENSHADER_API NormalNodeSlang : public HwNormalNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
