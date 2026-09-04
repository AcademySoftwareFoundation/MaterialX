//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLCOMPOUNDNODE_H
#define MATERIALX_WGSLCOMPOUNDNODE_H

#include <MaterialXGenWgsl/Export.h>

#include <MaterialXGenShader/Nodes/CompoundNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Compound node that emits a WGSL function ("fn name(param: type, ...)").
/// For closure/surface-shader compound nodes a `vd: VertexData` parameter is threaded so the
/// surface node body can access vertex data (normalWorld, positionWorld, ...).
class MX_GENWGSL_API WgslCompoundNode : public CompoundNode
{
  public:
    static ShaderNodeImplPtr create();
    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
