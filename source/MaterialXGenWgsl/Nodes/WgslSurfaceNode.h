//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLSURFACENODE_H
#define MATERIALX_WGSLSURFACENODE_H

#include <MaterialXGenWgsl/Export.h>

#include <MaterialXGenHw/Nodes/HwSurfaceNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Surface node for WGSL.
class MX_GENWGSL_API WgslSurfaceNode : public HwSurfaceNode
{
  public:
    static ShaderNodeImplPtr create();

  protected:
    void emitShadowOcclusionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage, const string& vertexPrefix) const override;

    void emitAmbientOcclusionCall(GenContext& context, ShaderStage& stage, const string& ambOccUv) const override;
};

MATERIALX_NAMESPACE_END

#endif
