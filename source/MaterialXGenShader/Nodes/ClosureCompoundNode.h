//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CLOSURECOMPOUNDNODE_H
#define MATERIALX_CLOSURECOMPOUNDNODE_H

#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

/// Extending the CompoundNode with requirements for closures.
class MX_GENSHADER_API ClosureCompoundNode : public CompoundNode
{
public:
    static ShaderNodeImplPtr create();

    void addClassification(ShaderNode& node) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

protected:
    void emitFunctionDefinition(ClosureContext* cct, GenContext& context, ShaderStage& stage) const;
};

MATERIALX_NAMESPACE_END

#endif
