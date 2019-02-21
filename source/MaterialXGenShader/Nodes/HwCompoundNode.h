#ifndef MATERIALX_HWCOMPOUNDNODE_H
#define MATERIALX_HWCOMPOUNDNODE_H

#include <MaterialXGenShader/Nodes/CompoundNode.h>

namespace MaterialX
{

class HwShaderGenerator;
class HwGenContext;
class HwClosureContext;

/// Extending the CompoundNode with requirements for HW.
class HwCompoundNode : public CompoundNode
{
public:
    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const override;

    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const override;

protected:
    void emitFunctionDefinition(ShaderStage& stage, const HwShaderGenerator& shadergen, GenContext& context, const HwClosureContext* ccx) const;
};

} // namespace MaterialX

#endif
