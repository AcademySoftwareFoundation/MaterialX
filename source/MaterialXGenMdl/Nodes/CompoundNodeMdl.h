//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_COMPOUNDNODEMDL_H
#define MATERIALX_COMPOUNDNODEMDL_H

#include <MaterialXGenMdl/Export.h>

#include <MaterialXGenShader/Nodes/CompoundNode.h>

MATERIALX_NAMESPACE_BEGIN

/// Compound node implementation
class MX_GENMDL_API CompoundNodeMdl : public CompoundNode
{
  public:
    static ShaderNodeImplPtr create();

    void initialize(const InterfaceElement& element, GenContext& context) override;
    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  protected:
    void emitFunctionSignature(const ShaderNode& node, GenContext& context, ShaderStage& stage) const;

    string _returnStruct;
};

MATERIALX_NAMESPACE_END

#endif
