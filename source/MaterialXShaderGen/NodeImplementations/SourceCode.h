#ifndef MATERIALX_SOURCECODENODE_H
#define MATERIALX_SOURCECODENODE_H

#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

/// Node implementation using data driven static source code.
/// This is the defaul implementation used for all nodes that 
/// does not have a custom C++ implementation class.
class SourceCode : public NodeImplementation
{
public:
    static NodeImplementationPtr creator();

    void initialize(const Implementation& implementation) override;

    void emitFunction(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int numArgs = 0, ...) override;

protected:
    bool _inlined;
    string _functionName;
    string _functionSource;
    vector<string> _extraInputs;
};

} // namespace MaterialX

#endif
