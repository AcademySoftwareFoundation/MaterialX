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

    void registerInputs(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionDefinition(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    SgNodeGraph* getNodeGraph() const override { return _rootGraph.get(); }

protected:
    SgNodeGraphPtr _rootGraph;
    string _functionName;
};

} // namespace MaterialX

#endif
