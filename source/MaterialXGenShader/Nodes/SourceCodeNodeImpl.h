#ifndef MATERIALX_SOURCECODE_H
#define MATERIALX_SOURCECODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// Implementation using data driven static source code.
/// This is the defaul implementation used for all nodes that 
/// does not have a custom ShaderNodeImpl class.
class SourceCodeNodeImpl : public ShaderNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen) override;

    void emitFunctionDefinition(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

protected:
    bool _inlined;
    string _functionName;
    string _functionSource;
};

} // namespace MaterialX

#endif
