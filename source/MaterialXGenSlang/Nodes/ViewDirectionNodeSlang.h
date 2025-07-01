//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VIEWDIRECTIONNODESLANG_H
#define MATERIALX_VIEWDIRECTIONNODESLANG_H

#include <MaterialXGenShader/Nodes/HwViewDirectionNode.h>

MATERIALX_NAMESPACE_BEGIN

/// ViewDirection node implementation for hardware languages
/// Differs from HwViewDirectionNode by:
/// - `floatX` instead of `vecX`
/// - `mul(v, m)` instead of `m * v`
class MX_GENSHADER_API ViewDirectionNodeSlang : public HwViewDirectionNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
