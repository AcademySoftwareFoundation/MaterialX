#ifndef MATERIALX_COMPOUND_H
#define MATERIALX_COMPOUND_H

#include <MaterialXShaderGen/SgImplementation.h>
#include <MaterialXShaderGen/SgNode.h>

namespace MaterialX
{

class Compound : public SgImplementation
{
public:
    static SgImplementationPtr creator();

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen) override;

    void emitFunction(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int numArgs = 0, ...) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int numArgs = 0, ...) override;

protected:
    SgNodeGraphPtr _sgNodeGraph;
    string _functionName;
};

} // namespace MaterialX

#endif
