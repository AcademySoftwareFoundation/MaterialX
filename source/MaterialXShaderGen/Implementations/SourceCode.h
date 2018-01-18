#ifndef MATERIALX_SOURCECODE_H
#define MATERIALX_SOURCECODE_H

#include <MaterialXShaderGen/SgImplementation.h>

namespace MaterialX
{

/// Implementation using data driven static source code.
/// This is the defaul implementation used for all nodes that 
/// does not have a custom SgImplementation class.
class SourceCode : public SgImplementation
{
public:
    static SgImplementationPtr creator();

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen) override;

    void emitFunctionDefinition(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

protected:
    bool _inlined;
    string _functionName;
    string _functionSource;
};

} // namespace MaterialX

#endif
