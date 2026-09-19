//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLMATERIALNODE_H
#define MATERIALX_WGSLMATERIALNODE_H

#include <MaterialXGenWgsl/Export.h>

#include <MaterialXGenShader/Nodes/MaterialNode.h>

MATERIALX_NAMESPACE_BEGIN

/// @class WgslMaterialNode
/// WGSL material node implementation.
///
/// Identical to MaterialNode except that the material output variable is declared
/// in WGSL order ("var <name>: <type> = <value>") instead of the C/GLSL order
/// ("<type> <name> = <value>") hardcoded in the shared MaterialNode.
class MX_GENWGSL_API WgslMaterialNode : public MaterialNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
