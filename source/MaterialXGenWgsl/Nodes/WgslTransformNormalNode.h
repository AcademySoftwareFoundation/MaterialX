//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLTRANSFORMNORMALNODE_H
#define MATERIALX_WGSLTRANSFORMNORMALNODE_H

#include <MaterialXGenWgsl/Export.h>

#include <MaterialXGenHw/Nodes/HwTransformNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Normal transform node for WGSL.
class MX_GENWGSL_API WgslTransformNormalNode : public HwTransformNormalNode
{
  public:
    static ShaderNodeImplPtr create();

  protected:
    void emitNormalizeCall(const ShaderOutput* output, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
