//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLSOURCECODENODE_H
#define MATERIALX_WGSLSOURCECODENODE_H

#include <MaterialXGenWgsl/Export.h>

#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

MATERIALX_NAMESPACE_BEGIN

/// @class WgslSourceCodeNode
/// WGSL source-code node implementation.
///
/// Identical to SourceCodeNode except that the local constant temporaries emitted
/// for unconnected inputs of an inlined node use WGSL declaration order
/// ("const <name>: <type> = <value>") instead of the C/GLSL order
/// ("const <type> <name> = <value>") hardcoded in the shared SourceCodeNode.
class MX_GENWGSL_API WgslSourceCodeNode : public SourceCodeNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END

#endif
