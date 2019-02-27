#ifndef MATERIALX_HWCOMPOUNDNODE_H
#define MATERIALX_HWCOMPOUNDNODE_H

#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

/// Extending the CompoundNode with requirements for HW.
class HwCompoundNode : public CompoundNode
{
public:
    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

    void emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

protected:
    void emitFunctionDefinition(ShaderStage& stage, GenContext& context, const HwShaderGenerator& shadergen, HwClosureContextPtr ccx) const;
};

} // namespace MaterialX

#endif
