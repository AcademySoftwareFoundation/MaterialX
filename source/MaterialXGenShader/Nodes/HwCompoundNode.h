//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_HWCOMPOUNDNODE_H
#define MATERIALX_HWCOMPOUNDNODE_H

#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

/// Extending the CompoundNode with requirements for HW.
class MX_GENSHADER_API HwCompoundNode : public CompoundNode
{
public:
    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

protected:
    void emitFunctionDefinition(HwClosureContextPtr ccx, GenContext& context, ShaderStage& stage) const;
};

} // namespace MaterialX

#endif
