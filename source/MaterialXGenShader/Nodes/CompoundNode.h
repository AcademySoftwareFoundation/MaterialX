#ifndef MATERIALX_COMPOUNDNODE_H
#define MATERIALX_COMPOUNDNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

class CompoundNode : public ShaderNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    void initialize(ElementPtr implementation, const ShaderGenerator& shadergen, GenContext& context) override;

    void createVariables(Shader& shader, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const override;

    void emitFunctionDefinition(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const override;

    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const override;

    ShaderGraph* getGraph() const override { return _rootGraph.get(); }

protected:
    ShaderGraphPtr _rootGraph;
    string _functionName;
};

} // namespace MaterialX

#endif
