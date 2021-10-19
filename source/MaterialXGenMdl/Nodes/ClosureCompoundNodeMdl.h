//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CLOSURECOMPOUNDNODEMDL_H
#define MATERIALX_CLOSURECOMPOUNDNODEMDL_H

#include <MaterialXGenMdl/Nodes/CompoundNodeMdl.h>

namespace MaterialX
{

/// Compound node implementation
class ClosureCompoundNodeMdl : public CompoundNodeMdl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

protected:
    string _returnStruct;
};

} // namespace MaterialX

#endif
