//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CLOSURECOMPOUNDNODEMDL_H
#define MATERIALX_CLOSURECOMPOUNDNODEMDL_H

#include <MaterialXGenMdl/Export.h>
#include <MaterialXGenMdl/Nodes/CompoundNodeMdl.h>

namespace MaterialX
{

/// Compound node implementation
class MX_GENMDL_API ClosureCompoundNodeMdl : public CompoundNodeMdl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
