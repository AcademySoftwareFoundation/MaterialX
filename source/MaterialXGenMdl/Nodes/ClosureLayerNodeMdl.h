//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_CLOSURELAYERNODEMDL_H
#define MATERIALX_CLOSURELAYERNODEMDL_H

#include <MaterialXGenMdl/Export.h>
#include <MaterialXGenMdl/Nodes/SourceCodeNodeMdl.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

/// Closure layer node implementation for MDL.
class MX_GENMDL_API ClosureLayerNodeMdl : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    /// String constants
    static const string TOP;
    static const string BASE;
};

/// Layerable BSDF node.
class MX_GENMDL_API LayerableNodeMdl : public SourceCodeNodeMdl
{
  public:
    static ShaderNodeImplPtr create();

    void addInputs(ShaderNode& node, GenContext&) const override;
};

MATERIALX_NAMESPACE_END

#endif
