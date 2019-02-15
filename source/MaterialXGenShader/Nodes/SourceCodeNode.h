#ifndef MATERIALX_SOURCECODENODE_H
#define MATERIALX_SOURCECODENODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// Implementation using data driven static source code.
/// This is the defaul implementation used for all nodes that 
/// does not have a custom ShaderNodeImpl class.
class SourceCodeNode : public ShaderNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen, const GenOptions& options) override;

    void emitFunctionDefinition(ShaderStage& stage, const ShaderNode& node, ShaderGenerator& shadergen) const override;

    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen) const override;

protected:
    bool _inlined;
    string _functionName;
    string _functionSource;
};

} // namespace MaterialX

#endif
