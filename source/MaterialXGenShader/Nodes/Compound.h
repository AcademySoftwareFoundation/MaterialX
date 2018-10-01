#ifndef MATERIALX_COMPOUND_H
#define MATERIALX_COMPOUND_H

#include <MaterialXGenShader/SgImplementation.h>
#include <MaterialXGenShader/SgNode.h>
#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

class Compound : public SgImplementation
{
public:
    static SgImplementationPtr create();

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen) override;

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionDefinition(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;

    SgNodeGraph* getNodeGraph() const override { return _rootGraph.get(); }

protected:
    SgNodeGraphPtr _rootGraph;
    string _functionName;
};

} // namespace MaterialX

#endif
