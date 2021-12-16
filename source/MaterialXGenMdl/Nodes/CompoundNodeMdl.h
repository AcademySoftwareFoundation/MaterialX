//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
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
    string _returnStruct;
};

MATERIALX_NAMESPACE_END

#endif
