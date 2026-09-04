//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLSURFACENODE_H
#define MATERIALX_WGSLSURFACENODE_H

#include <MaterialXGenWgsl/Export.h>

#include <MaterialXGenHw/Nodes/HwSurfaceNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Surface node that emits native WGSL (var name: type = value, for (var i: i32 = ...; ...)).
class MX_GENWGSL_API WgslSurfaceNode : public HwSurfaceNode
{
  public:
    static ShaderNodeImplPtr create();
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitLightLoop(const ShaderNode& node, GenContext& context, ShaderStage& stage, const string& outColor) const override;
};

MATERIALX_NAMESPACE_END

#endif
