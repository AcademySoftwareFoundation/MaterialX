#ifndef MATERIALX_HWSOURCECODENODE_H
#define MATERIALX_HWSOURCECODENODE_H

#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

/// Extending the SourceCodeNode with requirements for HW.
class HwSourceCodeNode : public SourceCodeNode
{
public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen, GenContext& context) const override;
};

} // namespace MaterialX

#endif
